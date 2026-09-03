#include "AssetActions/BertaAssetAuditor.h"
#include "AssetActions/BertaAssetNamingBatch.h"
#include "AssetActions/BertaAssetNamingUtils.h"
#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorUtilityLibrary.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/MessageDialog.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
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
		FNotificationInfo Info(Message);
		Info.bFireAndForget = true;
		Info.ExpireDuration = 4.0f;

		if (const TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info))
		{
			Item->SetCompletionState(State);
		}
	}

	bool IsTargetOccupied(IAssetRegistry& AssetRegistry, const FBertaAssetNamingBatchCandidate& Candidate)
	{
		if (AssetRegistry.GetAssetByObjectPath(Candidate.TargetObjectPath).IsValid())
		{
			return true;
		}

		if (FindPackage(nullptr, *Candidate.TargetPackageName))
		{
			return true;
		}

		return FPackageName::DoesPackageExist(Candidate.TargetPackageName);
	}
}

void UBertaAssetAuditor::ResolveAssetScope(TArray<FAssetData>& OutAssets)
{
	OutAssets.Reset();
	const TArray<FAssetData> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssetData();
	if (!SelectedAssets.IsEmpty())
	{
		for (const FAssetData& Asset : SelectedAssets)
		{
			if (IsProjectAsset(Asset))
			{
				OutAssets.Add(Asset);
			}
		}

		return;
	}

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	FString Folder;
	if (!UEditorUtilityLibrary::GetCurrentContentBrowserPath(Folder)
		|| (Folder != TEXT("/Game") && !Folder.StartsWith(TEXT("/Game/"))))
	{
		Folder = TEXT("/Game");
	}

	Filter.PackagePaths.Add(*Folder);
	FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get().GetAssets(Filter, OutAssets);
}

void UBertaAssetAuditor::AuditAssetNaming()
{
	TArray<FAssetData> Assets;
	ResolveAssetScope(Assets);
	AuditAssetNaming(Assets);
}

void UBertaAssetAuditor::AuditAssetNaming(const TArray<FAssetData>& Assets)
{
	int32 NeedsRename = 0;
	int32 Unknown = 0;
	for (const FAssetData& Asset : Assets)
	{
		const FBertaAssetNamingPlan Plan = UBertaAssetNamingUtils::BuildRenamePlan(Asset);
		if (Plan.Status == EBertaAssetNamingStatus::UnknownClass)
		{
			++Unknown;
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetNaming] Unknown class: %s (%s)"), *Asset.AssetName.ToString(), *Asset.AssetClassPath.ToString());
		}
		else if (Plan.Status == EBertaAssetNamingStatus::NeedsRename)
		{
			++NeedsRename;
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetNaming] VIOLATION: %s -> %s"), *Asset.AssetName.ToString(), *Plan.TargetName);
		}
	}
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetNaming] Audit complete: %d need rename, %d unknown."), NeedsRename, Unknown);
	ShowNotification(FText::Format(NSLOCTEXT("BertaDevKit", "AssetAudit", "Asset Audit: {0} violation(s), {1} unknown. See Output Log."), FText::AsNumber(NeedsRename), FText::AsNumber(Unknown)), NeedsRename > 0 || Unknown > 0 ? SNotificationItem::CS_Fail : SNotificationItem::CS_Success);
}

void UBertaAssetAuditor::FixAssetNaming()
{
	TArray<FAssetData> Assets;
	ResolveAssetScope(Assets);
	FixAssetNaming(Assets);
}

void UBertaAssetAuditor::FixAssetNaming(const TArray<FAssetData>& Assets)
{
	TArray<TPair<FAssetData, FBertaAssetNamingPlan>> Candidates;
	int32 Unknown = 0;
	for (const FAssetData& Asset : Assets)
	{
		FBertaAssetNamingPlan Plan = UBertaAssetNamingUtils::BuildRenamePlan(Asset);
		if (Plan.Status == EBertaAssetNamingStatus::NeedsRename)
		{
			Candidates.Emplace(Asset, MoveTemp(Plan));
		}
		else if (Plan.Status == EBertaAssetNamingStatus::UnknownClass)
		{
			++Unknown;
		}
	}

	if (Candidates.IsEmpty())
	{
		if (Unknown > 0)
		{
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetNaming] Fix skipped: %d asset(s) have unknown classes."), Unknown);
			ShowNotification(FText::Format(NSLOCTEXT("BertaDevKit", "UnknownAssetFix", "Asset Fix: No assets renamed; {0} unknown class(es). See Output Log."), FText::AsNumber(Unknown)), SNotificationItem::CS_Fail);
		}
		else
		{
			ShowNotification(NSLOCTEXT("BertaDevKit", "NoAssetFix", "Asset Fix: No assets require renaming."), SNotificationItem::CS_Success);
		}

		return;
	}

	if (Unknown > 0)
	{
		UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetNaming] Fix will skip %d asset(s) with unknown classes."), Unknown);
	}

	TArray<FBertaAssetNamingBatchCandidate> BatchCandidates;
	BatchCandidates.Reserve(Candidates.Num());
	int32 CandidateBuildFailed = 0;
	for (const TPair<FAssetData, FBertaAssetNamingPlan>& Candidate : Candidates)
	{
		FBertaAssetNamingBatchCandidate BatchCandidate;
		FText FailureReason;
		if (!BertaAssetNamingBatch::BuildCandidate(Candidate.Key, Candidate.Value, BatchCandidate, FailureReason))
		{
			++CandidateBuildFailed;
			UE_LOG(LogBertaDevKitEditor, Error, TEXT("[AssetNaming] Batch candidate failed: %s (%s)"), *Candidate.Key.GetSoftObjectPath().ToString(), *FailureReason.ToString());
			continue;
		}

		BatchCandidates.Add(MoveTemp(BatchCandidate));
	}

	if (CandidateBuildFailed > 0)
	{
		UE_LOG(LogBertaDevKitEditor, Error, TEXT("[AssetNaming] Fix aborted: %d candidate(s) could not produce valid rename destinations."), CandidateBuildFailed);
		ShowNotification(FText::Format(NSLOCTEXT("BertaDevKit", "AssetFixCandidateBuildFailed", "Asset Fix aborted: {0} invalid rename candidate(s). See Output Log."), FText::AsNumber(CandidateBuildFailed)), SNotificationItem::CS_Fail);
		return;
	}

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	const FBertaAssetNamingBatchPreflightResult PreflightResult = BertaAssetNamingBatch::Preflight(BatchCandidates, [&AssetRegistry](const FBertaAssetNamingBatchCandidate& Candidate)
	{
		return IsTargetOccupied(AssetRegistry, Candidate);
	});
	if (!PreflightResult.IsSafe())
	{
		for (const FBertaAssetNamingBatchConflict& Conflict : PreflightResult.Conflicts)
		{
			UE_LOG(LogBertaDevKitEditor, Error, TEXT("[AssetNaming] Batch conflict: %s -> %s (%s)"), *Conflict.SourceObjectPath, *Conflict.TargetObjectPath, *Conflict.Reason.ToString());
		}

		UE_LOG(LogBertaDevKitEditor, Error, TEXT("[AssetNaming] Fix aborted: %d conflict(s) across %d rename candidate(s)."), PreflightResult.Conflicts.Num(), BatchCandidates.Num());
		ShowNotification(FText::Format(NSLOCTEXT("BertaDevKit", "AssetFixPreflightFailed", "Asset Fix aborted: {0} conflict(s) across {1} rename candidate(s). See Output Log."), FText::AsNumber(PreflightResult.Conflicts.Num()), FText::AsNumber(BatchCandidates.Num())), SNotificationItem::CS_Fail);
		return;
	}

	const FText ConfirmationText = Unknown > 0
		? FText::Format(NSLOCTEXT("BertaDevKit", "ConfirmAssetFixWithUnknown", "Rename {0} project asset(s)? {1} unknown asset(s) will be skipped."), FText::AsNumber(BatchCandidates.Num()), FText::AsNumber(Unknown))
		: FText::Format(NSLOCTEXT("BertaDevKit", "ConfirmAssetFix", "Rename {0} project asset(s) to match BertaDevKit naming rules?"), FText::AsNumber(BatchCandidates.Num()));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ConfirmationText) != EAppReturnType::Yes)
	{
		return;
	}

	int32 LoadFailed = 0;
	TArray<TStrongObjectPtr<UObject>> LoadedAssetReferences;
	LoadedAssetReferences.Reserve(BatchCandidates.Num());
	TArray<UObject*> LoadedAssets;
	LoadedAssets.Reserve(BatchCandidates.Num());
	for (const FBertaAssetNamingBatchCandidate& Candidate : BatchCandidates)
	{
		UObject* Asset = Candidate.AssetData.GetAsset();
		if (!IsValid(Asset))
		{
			++LoadFailed;
			UE_LOG(LogBertaDevKitEditor, Error, TEXT("[AssetNaming] Batch load failed: %s"), *Candidate.SourceObjectPath);
			continue;
		}

		LoadedAssetReferences.Emplace(Asset);
		LoadedAssets.Add(Asset);
	}

	if (LoadFailed > 0)
	{
		UE_LOG(LogBertaDevKitEditor, Error, TEXT("[AssetNaming] Fix aborted: %d of %d candidate asset(s) failed to load; no rename was attempted."), LoadFailed, BatchCandidates.Num());
		ShowNotification(FText::Format(NSLOCTEXT("BertaDevKit", "AssetFixLoadFailed", "Asset Fix aborted: {0} asset(s) failed to load. No assets were renamed."), FText::AsNumber(LoadFailed)), SNotificationItem::CS_Fail);
		return;
	}

	const bool bRenameAssetsSucceeded = BertaAssetNamingBatch::Execute(BatchCandidates, LoadedAssets);
	int32 AtTarget = 0;
	int32 AtSource = 0;
	int32 Unexpected = 0;
	for (int32 CandidateIndex = 0; CandidateIndex < BatchCandidates.Num(); ++CandidateIndex)
	{
		const FString CurrentObjectPath = LoadedAssets[CandidateIndex]->GetPathName();
		const FBertaAssetNamingBatchCandidate& Candidate = BatchCandidates[CandidateIndex];
		if (CurrentObjectPath == Candidate.TargetObjectPath)
		{
			++AtTarget;
		}
		else if (CurrentObjectPath == Candidate.SourceObjectPath)
		{
			++AtSource;
		}
		else
		{
			++Unexpected;
			UE_LOG(LogBertaDevKitEditor, Error, TEXT("[AssetNaming] Batch postflight unexpected location: %s is at %s; expected %s or %s."), *Candidate.SourceObjectPath, *CurrentObjectPath, *Candidate.TargetObjectPath, *Candidate.SourceObjectPath);
		}
	}

	const bool bAllAtTarget = AtTarget == BatchCandidates.Num();
	if (bRenameAssetsSucceeded && bAllAtTarget)
	{
		UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetNaming] Fix complete: %d renamed, %d unknown, 0 load failed, 0 unexpected."), AtTarget, Unknown);
		ShowNotification(FText::Format(NSLOCTEXT("BertaDevKit", "AssetFix", "Asset Fix: {0} renamed, {1} unknown."), FText::AsNumber(AtTarget), FText::AsNumber(Unknown)), Unknown > 0 ? SNotificationItem::CS_Fail : SNotificationItem::CS_Success);
		return;
	}

	UE_LOG(LogBertaDevKitEditor, Error, TEXT("[AssetNaming] Batch rename incomplete: AssetTools returned %s; %d at target, %d at source, %d unexpected."), bRenameAssetsSucceeded ? TEXT("true") : TEXT("false"), AtTarget, AtSource, Unexpected);
	ShowNotification(FText::Format(NSLOCTEXT("BertaDevKit", "AssetFixIncomplete", "Asset Fix incomplete: {0} renamed, {1} unchanged, {2} unexpected. See Output Log."), FText::AsNumber(AtTarget), FText::AsNumber(AtSource), FText::AsNumber(Unexpected)), SNotificationItem::CS_Fail);
}
