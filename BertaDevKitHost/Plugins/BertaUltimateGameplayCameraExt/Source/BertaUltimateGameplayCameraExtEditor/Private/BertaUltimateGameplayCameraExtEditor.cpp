#include "Audit/BertaUGCAudit.h"
#include "Diff/BertaUGCPresetDiff.h"

#include "Camera/Data/UGC_CameraData.h"
#include "AssetRegistry/AssetData.h"
#include "ContentBrowserMenuContexts.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "ToolMenuEntry.h"
#include "ToolMenuSection.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaUGCExtEditor, Log, All);

#define LOCTEXT_NAMESPACE "BertaUltimateGameplayCameraExtEditor"

class FBertaUltimateGameplayCameraExtEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBertaUltimateGameplayCameraExtEditorModule::RegisterMenus));
	}

	virtual void ShutdownModule() override
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(MenuOwner);
	}

private:
	void RegisterMenus()
	{
		if (!UToolMenus::IsToolMenuUIEnabled())
		{
			UE_LOG(LogBertaUGCExtEditor, Warning, TEXT("UGC Editor tools menu is unavailable after engine initialization."));
			return;
		}
		UToolMenu* Tools = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
		if (!Tools)
		{
			UE_LOG(LogBertaUGCExtEditor, Warning, TEXT("UGC Editor tools could not extend the Tools menu."));
			return;
		}
		FToolMenuSection& Section = Tools->FindOrAddSection(TEXT("BertaUltimateGameplayCameraExt"));
		FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
			TEXT("BertaAuditUltimateGameplayCamera"), LOCTEXT("AuditUGC", "Audit Ultimate Gameplay Camera"),
			LOCTEXT("AuditUGCTooltip", "Read-only audit of UGC setup, presets and explicit Blueprint camera writes. See Output Log."),
			FSlateIcon(), FUIAction(FExecuteAction::CreateStatic(&BertaUGCAudit::Run)));
		Entry.Owner = FToolMenuOwner(MenuOwner);
		Section.AddEntry(Entry);

		UToolMenu* AssetMenu = UToolMenus::Get()->ExtendMenu(TEXT("ContentBrowser.AssetContextMenu"));
		if (!AssetMenu) return;
		FToolMenuSection& AssetSection = AssetMenu->FindOrAddSection(TEXT("BertaUltimateGameplayCameraExt"));
		FToolMenuEntry& Dynamic = AssetSection.AddDynamicEntry(TEXT("BertaUGCPresetComparison"),
			FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
			{
				const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection);
				if (!Context || Context->SelectedAssets.Num() != 2) return;
				const UUGC_CameraDataAssetBase* Left = Cast<UUGC_CameraDataAssetBase>(Context->SelectedAssets[0].GetAsset());
				const UUGC_CameraDataAssetBase* Right = Cast<UUGC_CameraDataAssetBase>(Context->SelectedAssets[1].GetAsset());
				if (!Left || !Right || Left == Right) return;
				const FAssetData LeftData = Context->SelectedAssets[0];
				const FAssetData RightData = Context->SelectedAssets[1];
				FToolMenuEntry Compare = FToolMenuEntry::InitMenuEntry(
					TEXT("BertaCompareUGCPresets"), LOCTEXT("CompareUGCPresets", "Compare UGC Camera Presets"),
					LOCTEXT("CompareUGCPresetsTooltip", "Read-only reflected diff of exactly two UGC camera data assets. See Output Log."),
					FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([LeftData, RightData]()
					{
						const UUGC_CameraDataAssetBase* CurrentLeft = Cast<UUGC_CameraDataAssetBase>(LeftData.GetAsset());
						const UUGC_CameraDataAssetBase* CurrentRight = Cast<UUGC_CameraDataAssetBase>(RightData.GetAsset());
						if (CurrentLeft && CurrentRight && CurrentLeft != CurrentRight)
							BertaUGCPresetDiff::Run(*CurrentLeft, *CurrentRight);
					})));
				Compare.Owner = FToolMenuOwner(TEXT("BertaUltimateGameplayCameraExtEditor"));
				InSection.AddEntry(Compare);
			}));
		Dynamic.Owner = FToolMenuOwner(MenuOwner);
	}

	const FName MenuOwner = TEXT("BertaUltimateGameplayCameraExtEditor");
};

IMPLEMENT_MODULE(FBertaUltimateGameplayCameraExtEditorModule, BertaUltimateGameplayCameraExtEditor)

#undef LOCTEXT_NAMESPACE
