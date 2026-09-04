#include "AssetActions/BertaAssetCleaner.h"

#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetIdentifier.h"
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
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	if (AssetRegistry.IsGathering())
	{
		UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] Audit skipped because the Asset Registry is still gathering."));
		ShowNotification(LOCTEXT("AssetRegistryGathering", "Asset Cleaner cannot audit while the Asset Registry is gathering. Try again when scanning completes."), SNotificationItem::CS_None);
		return;
	}

	int32 ReferencedCount = 0;
	int32 UnusedCandidateCount = 0;
	int32 ProtectedCount = 0;
	int32 SkippedCount = 0;
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();

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

		const FBertaAssetCleanerClassificationResult Result = ClassifyAsset(AssetData, Inspection);
		switch (Result.Classification)
		{
		case EBertaAssetCleanerClassification::Referenced:
			++ReferencedCount;
			break;
		case EBertaAssetCleanerClassification::UnusedCandidate:
			++UnusedCandidateCount;
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] UNUSED CANDIDATE: %s (%s)"), *AssetData.GetObjectPathString(), *AssetData.AssetClassPath.ToString());
			break;
		case EBertaAssetCleanerClassification::Protected:
			++ProtectedCount;
			UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] PROTECTED: %s (%s) - %s"), *AssetData.GetObjectPathString(), *AssetData.AssetClassPath.ToString(), *Result.Reason);
			break;
		case EBertaAssetCleanerClassification::Skipped:
			++SkippedCount;
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - %s"), *AssetData.GetObjectPathString(), *Result.Reason);
			break;
		}
	}

	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] Audit complete: %d unused candidate(s), %d protected, %d referenced, %d skipped."), UnusedCandidateCount, ProtectedCount, ReferencedCount, SkippedCount);
	if (UnusedCandidateCount == 0 && SkippedCount == 0)
	{
		ShowNotification(LOCTEXT("NoUnusedCandidates", "Asset Cleaner: No unused candidates found. No assets were modified."), SNotificationItem::CS_Success);
	}
	else
	{
		ShowNotification(FText::Format(LOCTEXT("AuditSummary", "Asset Cleaner: {0} unused candidate(s), {1} protected, {2} skipped. See Output Log. No assets were modified."), FText::AsNumber(UnusedCandidateCount), FText::AsNumber(ProtectedCount), FText::AsNumber(SkippedCount)), UnusedCandidateCount > 0 ? SNotificationItem::CS_Success : SNotificationItem::CS_None);
	}
}

#undef LOCTEXT_NAMESPACE
