#include "AssetActions/BertaAssetCleaner.h"

#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetIdentifier.h"
#include "AssetViewUtils.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/PackagePath.h"
#include "UObject/ObjectRedirector.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "WorldPartition/DataLayer/ExternalDataLayerHelper.h"

#define LOCTEXT_NAMESPACE "BertaAssetCleaner"

namespace
{
	bool ContainsPackagePathSegment(const FString& PackageName, const TCHAR* Segment)
	{
		return PackageName.Contains(FString::Printf(TEXT("/%s/"), Segment), ESearchCase::IgnoreCase);
	}

	bool IsGeneratedWorldStoragePackage(const FAssetData& AssetData)
	{
		const FString PackageName = AssetData.PackageName.ToString();
		return ContainsPackagePathSegment(PackageName, FPackagePath::GetExternalActorsFolderName())
			|| ContainsPackagePathSegment(PackageName, FPackagePath::GetExternalObjectsFolderName())
			|| FExternalDataLayerHelper::IsExternalDataLayerPath(PackageName);
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

	struct FInspectionSummary
	{
		int32 ReferencedCount = 0;
		int32 UnusedCandidateCount = 0;
		int32 ProtectedCount = 0;
		int32 SkippedCount = 0;
	};

	void AddToSummary(const FBertaAssetCleanerClassificationResult& Result, FInspectionSummary& InOutSummary)
	{
		switch (Result.Classification)
		{
		case EBertaAssetCleanerClassification::Referenced:
			++InOutSummary.ReferencedCount;
			break;
		case EBertaAssetCleanerClassification::UnusedCandidate:
			++InOutSummary.UnusedCandidateCount;
			break;
		case EBertaAssetCleanerClassification::Protected:
			++InOutSummary.ProtectedCount;
			break;
		case EBertaAssetCleanerClassification::Skipped:
			++InOutSummary.SkippedCount;
			break;
		}
	}

	bool InspectAssets(const TArray<FAssetData>& Assets, const TCHAR* OperationName, TArray<FBertaAssetCleanerAssetResult>& OutResults, FInspectionSummary& OutSummary)
	{
		OutResults.Reset();
		OutSummary = FInspectionSummary();

		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		if (AssetRegistry.IsGathering())
		{
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] %s aborted because the Asset Registry is still gathering."), OperationName);
			ShowNotification(LOCTEXT("AssetRegistryGathering", "Asset Cleaner cannot run while the Asset Registry is gathering. Try again when scanning completes."), SNotificationItem::CS_None);
			return false;
		}

		UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
		OutResults.Reserve(Assets.Num());
		for (const FAssetData& AssetData : Assets)
		{
			TArray<FAssetIdentifier> Referencers;
			const bool bReferencerQuerySucceeded = AssetRegistry.GetReferencers(
				FAssetIdentifier(AssetData.PackageName),
				Referencers,
				UE::AssetRegistry::EDependencyCategory::All,
				UE::AssetRegistry::FDependencyQuery());

			FBertaAssetCleanerInspection Inspection;
			Inspection.bReferencerQuerySucceeded = bReferencerQuerySucceeded;
			Inspection.ReferencerCount = Referencers.Num();
			Inspection.bPrimaryAssetQueryAvailable = AssetManager && AssetManager->HasInitialScanCompleted();
			if (Inspection.bPrimaryAssetQueryAvailable)
			{
				Inspection.bIsRegisteredPrimaryAsset = AssetManager->GetPrimaryAssetIdForPath(AssetData.GetSoftObjectPath()).IsValid();
			}

			FBertaAssetCleanerAssetResult& Result = OutResults.AddDefaulted_GetRef();
			Result.AssetData = AssetData;
			Result.Classification = FBertaAssetCleaner::ClassifyAsset(AssetData, Inspection);
			AddToSummary(Result.Classification, OutSummary);
		}

		return true;
	}

	void LogInspectionDetails(const TArray<FBertaAssetCleanerAssetResult>& InspectionResults)
	{
		for (const FBertaAssetCleanerAssetResult& Result : InspectionResults)
		{
			switch (Result.Classification.Classification)
			{
			case EBertaAssetCleanerClassification::UnusedCandidate:
				UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] UNUSED CANDIDATE: %s (%s)"), *Result.AssetData.GetObjectPathString(), *Result.AssetData.AssetClassPath.ToString());
				break;
			case EBertaAssetCleanerClassification::Protected:
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] PROTECTED: %s (%s) - %s"), *Result.AssetData.GetObjectPathString(), *Result.AssetData.AssetClassPath.ToString(), *Result.Classification.Reason);
				break;
			case EBertaAssetCleanerClassification::Skipped:
				UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - %s"), *Result.AssetData.GetObjectPathString(), *Result.Classification.Reason);
				break;
			case EBertaAssetCleanerClassification::Referenced:
				break;
			}
		}
	}
}

FBertaAssetCleanerClassificationResult FBertaAssetCleaner::ClassifyAsset(const FAssetData& AssetData, const FBertaAssetCleanerInspection& Inspection)
{
	if (!Inspection.bReferencerQuerySucceeded)
	{
		return { EBertaAssetCleanerClassification::Skipped, TEXT("Asset Registry referencer query failed") };
	}

	if (Inspection.ReferencerCount > 0)
	{
		return { EBertaAssetCleanerClassification::Referenced, FString() };
	}

	if (AssetData.AssetClassPath == UWorld::StaticClass()->GetClassPathName())
	{
		return { EBertaAssetCleanerClassification::Protected, TEXT("World/map asset") };
	}

	if (AssetData.AssetClassPath == UObjectRedirector::StaticClass()->GetClassPathName())
	{
		return { EBertaAssetCleanerClassification::Protected, TEXT("Object redirector") };
	}

	if (IsGeneratedWorldStoragePackage(AssetData))
	{
		return { EBertaAssetCleanerClassification::Protected, TEXT("World Partition or external generated storage") };
	}

	if (!Inspection.bPrimaryAssetQueryAvailable)
	{
		return { EBertaAssetCleanerClassification::Skipped, TEXT("Asset Manager is unavailable; primary asset registration could not be checked") };
	}

	if (Inspection.bIsRegisteredPrimaryAsset)
	{
		return { EBertaAssetCleanerClassification::Protected, TEXT("Registered Primary Asset") };
	}

	return { EBertaAssetCleanerClassification::UnusedCandidate, FString() };
}

void FBertaAssetCleaner::AuditUnusedAssets(const TArray<FAssetData>& Assets)
{
	TArray<FBertaAssetCleanerAssetResult> InspectionResults;
	FInspectionSummary Summary;
	if (!InspectAssets(Assets, TEXT("Audit"), InspectionResults, Summary))
	{
		return;
	}

	LogInspectionDetails(InspectionResults);
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] Audit complete: %d unused candidate(s), %d protected, %d referenced, %d skipped."), Summary.UnusedCandidateCount, Summary.ProtectedCount, Summary.ReferencedCount, Summary.SkippedCount);
	if (Summary.UnusedCandidateCount == 0 && Summary.SkippedCount == 0)
	{
		ShowNotification(LOCTEXT("NoUnusedCandidates", "Asset Cleaner: No unused candidates found. No assets were modified."), SNotificationItem::CS_Success);
	}
	else
	{
		ShowNotification(FText::Format(LOCTEXT("AuditSummary", "Asset Cleaner: {0} unused candidate(s), {1} protected, {2} skipped. See Output Log. No assets were modified."), FText::AsNumber(Summary.UnusedCandidateCount), FText::AsNumber(Summary.ProtectedCount), FText::AsNumber(Summary.SkippedCount)), Summary.UnusedCandidateCount > 0 ? SNotificationItem::CS_Success : SNotificationItem::CS_None);
	}
}

void FBertaAssetCleaner::CleanUnusedAssets(const TArray<FAssetData>& Assets)
{
	TArray<FBertaAssetCleanerAssetResult> InspectionResults;
	FInspectionSummary Summary;
	if (!InspectAssets(Assets, TEXT("Clean"), InspectionResults, Summary))
	{
		return;
	}

	LogInspectionDetails(InspectionResults);
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] Clean preflight: %d current unused candidate(s), %d protected, %d referenced, %d skipped."), Summary.UnusedCandidateCount, Summary.ProtectedCount, Summary.ReferencedCount, Summary.SkippedCount);

	TArray<FAssetData> Candidates;
	CollectUnusedCandidateAssets(InspectionResults, Candidates);
	if (Candidates.IsEmpty())
	{
		ShowNotification(LOCTEXT("NoUnusedCandidatesToClean", "Asset Cleaner: No unused candidates to clean."), SNotificationItem::CS_Success);
		return;
	}

	TArray<UObject*> LoadedObjects;
	const AssetViewUtils::FLoadAssetsSettings LoadSettings{
		.bFollowRedirectors = false,
		.bAllowCancel = true,
	};
	const AssetViewUtils::ELoadAssetsResult LoadResult = AssetViewUtils::LoadAssetsIfNeeded(Candidates, LoadedObjects, LoadSettings);
	if (LoadResult == AssetViewUtils::ELoadAssetsResult::Cancelled)
	{
		UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] Clean canceled while loading %d current unused candidate(s)."), Candidates.Num());
		return;
	}

	TArray<UObject*> LoadedCandidates;
	CollectLoadedCandidateObjects(Candidates, LoadedObjects, LoadedCandidates);
	TSet<FName> LoadedCandidatePaths;
	for (const UObject* LoadedCandidate : LoadedCandidates)
	{
		LoadedCandidatePaths.Add(*LoadedCandidate->GetPathName());
	}

	for (const FAssetData& Candidate : Candidates)
	{
		if (!LoadedCandidatePaths.Contains(*Candidate.GetObjectPathString()))
		{
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - failed to load current unused candidate for native deletion."), *Candidate.GetObjectPathString());
		}
	}

	if (LoadedCandidates.IsEmpty())
	{
		ShowNotification(LOCTEXT("NoLoadedCandidatesToClean", "Asset Cleaner: No unused candidates could be loaded for deletion."), SNotificationItem::CS_None);
		return;
	}

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	if (AssetRegistry.IsGathering())
	{
		UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] Clean aborted before native deletion because the Asset Registry is gathering."));
		ShowNotification(LOCTEXT("AssetRegistryGatheringBeforeDeletion", "Asset Cleaner cannot clean while the Asset Registry is gathering. Try again when scanning completes."), SNotificationItem::CS_Fail);
		return;
	}

	const int32 DeletedCount = AssetViewUtils::DeleteAssets(LoadedCandidates);
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] Clean complete: %d of %d loaded candidate asset(s) deleted by Unreal's native deletion workflow."), DeletedCount, LoadedCandidates.Num());
	ShowNotification(FText::Format(LOCTEXT("CleanComplete", "Asset Cleaner: Unreal deleted {0} of {1} candidate asset(s). See Output Log."), FText::AsNumber(DeletedCount), FText::AsNumber(LoadedCandidates.Num())), DeletedCount > 0 ? SNotificationItem::CS_Success : SNotificationItem::CS_None);
}

void FBertaAssetCleaner::CollectUnusedCandidateAssets(const TArray<FBertaAssetCleanerAssetResult>& InspectionResults, TArray<FAssetData>& OutCandidates)
{
	OutCandidates.Reset();
	for (const FBertaAssetCleanerAssetResult& Result : InspectionResults)
	{
		if (Result.Classification.Classification == EBertaAssetCleanerClassification::UnusedCandidate)
		{
			OutCandidates.Add(Result.AssetData);
		}
	}
}

void FBertaAssetCleaner::CollectLoadedCandidateObjects(const TArray<FAssetData>& Candidates, TConstArrayView<UObject*> LoadedObjects, TArray<UObject*>& OutLoadedCandidates)
{
	OutLoadedCandidates.Reset();
	TSet<FName> CandidatePaths;
	for (const FAssetData& Candidate : Candidates)
	{
		CandidatePaths.Add(*Candidate.GetObjectPathString());
	}

	TSet<UObject*> SeenCandidates;
	for (UObject* LoadedObject : LoadedObjects)
	{
		if (LoadedObject && CandidatePaths.Contains(*LoadedObject->GetPathName()) && !SeenCandidates.Contains(LoadedObject))
		{
			SeenCandidates.Add(LoadedObject);
			OutLoadedCandidates.Add(LoadedObject);
		}
	}
}

#undef LOCTEXT_NAMESPACE
