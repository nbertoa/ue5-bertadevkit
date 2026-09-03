#include "AssetActions/BertaAssetAuditor.h"
#include "AssetActions/BertaAssetNamingUtils.h"
#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorUtilityLibrary.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace
{
	bool IsProjectAsset(const FAssetData& AssetData)
	{
		const FString Path = AssetData.PackagePath.ToString();
		return Path == TEXT("/Game") || Path.StartsWith(TEXT("/Game/"));
	}
	void ShowNotification(const FText& Message, SNotificationItem::ECompletionState State)
	{
		FNotificationInfo Info(Message); Info.bFireAndForget = true; Info.ExpireDuration = 4.0f;
		if (const TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info)) Item->SetCompletionState(State);
	}
}

void UBertaAssetAuditor::ResolveAssetScope(TArray<FAssetData>& OutAssets)
{
	OutAssets.Reset();
	for (const FAssetData& Asset : UEditorUtilityLibrary::GetSelectedAssetData()) if (IsProjectAsset(Asset)) OutAssets.Add(Asset);
	if (!OutAssets.IsEmpty()) return;
	FARFilter Filter; Filter.bRecursivePaths = true;
	FString Folder;
	if (!UEditorUtilityLibrary::GetCurrentContentBrowserPath(Folder) || (Folder != TEXT("/Game") && !Folder.StartsWith(TEXT("/Game/")))) Folder = TEXT("/Game");
	Filter.PackagePaths.Add(*Folder);
	FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get().GetAssets(Filter, OutAssets);
}

void UBertaAssetAuditor::AuditAssetNaming()
{
	TArray<FAssetData> Assets; ResolveAssetScope(Assets);
	int32 NeedsRename = 0, Unknown = 0;
	for (const FAssetData& Asset : Assets)
	{
		const FBertaAssetNamingPlan Plan = UBertaAssetNamingUtils::BuildRenamePlan(Asset);
		if (Plan.Status == EBertaAssetNamingStatus::UnknownClass) { ++Unknown; UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetNaming] Unknown class: %s (%s)"), *Asset.AssetName.ToString(), *Asset.AssetClassPath.ToString()); }
		else if (Plan.Status == EBertaAssetNamingStatus::NeedsRename) { ++NeedsRename; UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetNaming] VIOLATION: %s -> %s"), *Asset.AssetName.ToString(), *Plan.TargetName); }
	}
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetNaming] Audit complete: %d need rename, %d unknown."), NeedsRename, Unknown);
	ShowNotification(FText::Format(NSLOCTEXT("BertaDevKit", "AssetAudit", "Asset Audit: {0} violation(s), {1} unknown. See Output Log."), FText::AsNumber(NeedsRename), FText::AsNumber(Unknown)), NeedsRename > 0 || Unknown > 0 ? SNotificationItem::CS_Fail : SNotificationItem::CS_Success);
}

void UBertaAssetAuditor::FixAssetNaming()
{
	TArray<FAssetData> Assets; ResolveAssetScope(Assets);
	TArray<TPair<FAssetData, FBertaAssetNamingPlan>> Candidates;
	int32 Unknown = 0;
	for (const FAssetData& Asset : Assets) { FBertaAssetNamingPlan Plan = UBertaAssetNamingUtils::BuildRenamePlan(Asset); if (Plan.Status == EBertaAssetNamingStatus::NeedsRename) Candidates.Emplace(Asset, MoveTemp(Plan)); else if (Plan.Status == EBertaAssetNamingStatus::UnknownClass) ++Unknown; }
	if (Candidates.IsEmpty()) { ShowNotification(NSLOCTEXT("BertaDevKit", "NoAssetFix", "Asset Fix: No assets require renaming."), SNotificationItem::CS_Success); return; }
	if (FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(NSLOCTEXT("BertaDevKit", "ConfirmAssetFix", "Rename {0} project asset(s) to match BertaDevKit naming rules?"), FText::AsNumber(Candidates.Num()))) != EAppReturnType::Yes) return;
	int32 Renamed = 0, LoadFailed = 0, RenameFailed = 0;
	for (const TPair<FAssetData, FBertaAssetNamingPlan>& Candidate : Candidates)
	{
		UObject* Asset = Candidate.Key.GetAsset();
		if (!IsValid(Asset)) { ++LoadFailed; continue; }
		if (UBertaAssetNamingUtils::ExecuteRename(Asset, Candidate.Value) == EBertaRenameResult::Renamed) ++Renamed; else ++RenameFailed;
	}
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetNaming] Fix complete: %d renamed, %d unknown, %d load failed, %d rename failed."), Renamed, Unknown, LoadFailed, RenameFailed);
	ShowNotification(FText::Format(NSLOCTEXT("BertaDevKit", "AssetFix", "Asset Fix: {0} renamed, {1} unknown, {2} load failed, {3} rename failed."), FText::AsNumber(Renamed), FText::AsNumber(Unknown), FText::AsNumber(LoadFailed), FText::AsNumber(RenameFailed)), Unknown + LoadFailed + RenameFailed > 0 ? SNotificationItem::CS_Fail : SNotificationItem::CS_Success);
}
