#pragma once

#include "AssetRegistry/AssetData.h"

enum class EBertaAssetCleanerClassification : uint8
{
	Referenced,
	UnusedCandidate,
	Protected,
	Skipped,
};

struct FBertaAssetCleanerInspection
{
	bool bReferencerQuerySucceeded = false;
	int32 ReferencerCount = 0;
	bool bPrimaryAssetQueryAvailable = false;
	bool bIsRegisteredPrimaryAsset = false;
};

struct FBertaAssetCleanerClassificationResult
{
	EBertaAssetCleanerClassification Classification = EBertaAssetCleanerClassification::Skipped;
	FString Reason;
};

struct FBertaAssetCleanerAssetResult
{
	FAssetData AssetData;
	FBertaAssetCleanerClassificationResult Classification;
};

class FBertaAssetCleaner
{
public:
	static void AuditUnusedAssets(const TArray<FAssetData>& Assets);
	static void CleanUnusedAssets(const TArray<FAssetData>& Assets);
	static FBertaAssetCleanerClassificationResult ClassifyAsset(const FAssetData& AssetData, const FBertaAssetCleanerInspection& Inspection);
	static void CollectUnusedCandidateAssets(const TArray<FBertaAssetCleanerAssetResult>& InspectionResults, TArray<FAssetData>& OutCandidates);
	static void CollectLoadedCandidateObjects(const TArray<FAssetData>& Candidates, TConstArrayView<UObject*> LoadedObjects, TArray<UObject*>& OutLoadedCandidates);
};
