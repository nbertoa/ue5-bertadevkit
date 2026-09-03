#include "ContentBrowser/BertaContentBrowserMenu.h"

#include "AssetActions/BertaAssetAuditor.h"
#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserMenuContexts.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "BertaContentBrowserMenu"

namespace
{
	const FName BertaContentBrowserOwnerName(TEXT("BertaDevKitContentBrowser"));

	bool IsProjectPath(const FString& Path)
	{
		return Path == TEXT("/Game") || Path.StartsWith(TEXT("/Game/"));
	}

	bool IsAssetInFolder(const FAssetData& Asset, const FString& Folder)
	{
		const FString AssetPath = Asset.PackagePath.ToString();
		return AssetPath == Folder || AssetPath.StartsWith(Folder + TEXT("/"));
	}

	void AddAssetNamingEntries(FToolMenuSection& Section, const TArray<FAssetData>& Assets, const FText& TooltipScope)
	{
		FToolMenuEntry AuditEntry = FToolMenuEntry::InitMenuEntry(TEXT("BertaAuditAssetNaming"), LOCTEXT("AuditAssetNaming", "Audit Asset Naming"), FText::Format(LOCTEXT("AuditAssetNamingTooltip", "Audit Asset Naming for {0}."), TooltipScope), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([Assets]() { UBertaAssetAuditor::AuditAssetNaming(Assets); })));
		AuditEntry.Owner = FToolMenuOwner(BertaContentBrowserOwnerName);
		Section.AddEntry(AuditEntry);

		FToolMenuEntry FixEntry = FToolMenuEntry::InitMenuEntry(TEXT("BertaFixAssetNaming"), LOCTEXT("FixAssetNaming", "Fix Asset Naming"), FText::Format(LOCTEXT("FixAssetNamingTooltip", "Fix Asset Naming for {0}."), TooltipScope), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([Assets]() { UBertaAssetAuditor::FixAssetNaming(Assets); })));
		FixEntry.Owner = FToolMenuOwner(BertaContentBrowserOwnerName);
		Section.AddEntry(FixEntry);
	}
}

void FBertaContentBrowserMenu::Register()
{
	if (!UToolMenus::IsToolMenuUIEnabled())
	{
		UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[FBertaContentBrowserMenu::Register] UToolMenus is not available yet."));
		return;
	}

	UToolMenu* AssetMenu = UToolMenus::Get()->ExtendMenu(TEXT("ContentBrowser.AssetContextMenu"));
	UToolMenu* FolderMenu = UToolMenus::Get()->ExtendMenu(TEXT("ContentBrowser.FolderContextMenu"));
	if (!AssetMenu || !FolderMenu)
	{
		UE_LOG(LogBertaDevKitEditor, Error, TEXT("[FBertaContentBrowserMenu::Register] Failed to extend Content Browser context menus."));
		return;
	}

	FToolMenuSection& AssetSection = AssetMenu->FindOrAddSection(TEXT("BertaDevKitAssetNaming"), LOCTEXT("BertaDevKit", "BertaDevKit"));
	FToolMenuEntry& AssetDynamicEntry = AssetSection.AddDynamicEntry(TEXT("BertaAssetNamingActions"), FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& Section)
	{
		if (const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(Section))
		{
			TArray<FAssetData> Assets;
			FilterProjectAssets(Context->SelectedAssets, Assets);
			if (!Assets.IsEmpty())
			{
				AddAssetNamingEntries(Section, Assets, LOCTEXT("SelectedAssets", "the selected project asset(s)"));
			}
		}
	}));
	AssetDynamicEntry.Owner = FToolMenuOwner(BertaContentBrowserOwnerName);

	FToolMenuSection& FolderSection = FolderMenu->FindOrAddSection(TEXT("BertaDevKitAssetNaming"), LOCTEXT("BertaDevKit", "BertaDevKit"));
	FToolMenuEntry& FolderDynamicEntry = FolderSection.AddDynamicEntry(TEXT("BertaAssetNamingActions"), FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& Section)
	{
		if (const UContentBrowserFolderContext* Context = Section.FindContext<UContentBrowserFolderContext>())
		{
			const bool bHasProjectFolder = Context->GetSelectedPackagePaths().ContainsByPredicate([](const FString& Folder) { return IsProjectPath(Folder); });
			if (!bHasProjectFolder)
			{
				return;
			}

			TArray<FAssetData> Assets;
			GatherAssetsInProjectFolders(Context->GetSelectedPackagePaths(), Assets);
			AddAssetNamingEntries(Section, Assets, LOCTEXT("SelectedFolders", "the selected project folder(s) recursively"));
		}
	}));
	FolderDynamicEntry.Owner = FToolMenuOwner(BertaContentBrowserOwnerName);
}

void FBertaContentBrowserMenu::Unregister()
{
	UToolMenus::UnregisterOwner(BertaContentBrowserOwnerName);
}

void FBertaContentBrowserMenu::FilterProjectAssets(const TArray<FAssetData>& InAssets, TArray<FAssetData>& OutAssets)
{
	OutAssets.Reset();
	for (const FAssetData& Asset : InAssets)
	{
		if (IsProjectPath(Asset.PackagePath.ToString()))
		{
			OutAssets.Add(Asset);
		}
	}
}

void FBertaContentBrowserMenu::FilterAssetsInProjectFolders(const TArray<FString>& InFolders, const TArray<FAssetData>& InAssets, TArray<FAssetData>& OutAssets)
{
	TArray<FString> ProjectFolders;
	for (const FString& Folder : InFolders)
	{
		if (IsProjectPath(Folder))
		{
			ProjectFolders.AddUnique(Folder);
		}
	}

	OutAssets.Reset();
	TSet<FName> SeenAssets;
	for (const FAssetData& Asset : InAssets)
	{
		if (!IsProjectPath(Asset.PackagePath.ToString()) || !ProjectFolders.ContainsByPredicate([&Asset](const FString& Folder) { return IsAssetInFolder(Asset, Folder); }))
		{
			continue;
		}

		const FName ObjectPath(*Asset.GetObjectPathString());
		if (!SeenAssets.Contains(ObjectPath))
		{
			SeenAssets.Add(ObjectPath);
			OutAssets.Add(Asset);
		}
	}
}

void FBertaContentBrowserMenu::GatherAssetsInProjectFolders(const TArray<FString>& InFolders, TArray<FAssetData>& OutAssets)
{
	TArray<FString> ProjectFolders;
	for (const FString& Folder : InFolders)
	{
		if (IsProjectPath(Folder))
		{
			ProjectFolders.AddUnique(Folder);
		}
	}

	OutAssets.Reset();
	if (ProjectFolders.IsEmpty())
	{
		return;
	}

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	for (const FString& Folder : ProjectFolders)
	{
		Filter.PackagePaths.Add(*Folder);
	}

	TArray<FAssetData> Assets;
	FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get().GetAssets(Filter, Assets);
	FilterAssetsInProjectFolders(ProjectFolders, Assets, OutAssets);
}

#undef LOCTEXT_NAMESPACE
