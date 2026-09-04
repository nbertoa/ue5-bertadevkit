#pragma once

#include "AssetRegistry/AssetData.h"
#include "Containers/Set.h"

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

struct FBertaAssetCleanerPackageRecord
{
	FName PackageName;
	TArray<FAssetData> Assets;
	bool bProtected = false;
	bool bSkipped = false;
	FString Reason;
	TSet<FName> ReferencerPackages;
	TSet<FName> DependencyPackages;
	bool bHasExternalReferencer = false;
	bool bDependencyQuerySucceeded = true;
};

struct FBertaAssetCleanerGraphAnalysis
{
	bool bComplete = true;
	TSet<FName> LivePackages;
	TSet<FName> OrphanPackages;
	TArray<TArray<FName>> OrphanGroups;
};

class FBertaAssetCleaner
{
public:
	static void AuditUnusedAssets(const TArray<FAssetData>& Assets);
	static void CleanUnusedAssets(const TArray<FAssetData>& Assets);
	static FBertaAssetCleanerClassificationResult ClassifyAsset(const FAssetData& AssetData, const FBertaAssetCleanerInspection& Inspection);
	static FBertaAssetCleanerGraphAnalysis AnalyzePackageGraph(const TArray<FBertaAssetCleanerPackageRecord>& PackageRecords);
	static void CollectLoadedCandidateObjects(const TArray<FAssetData>& Candidates, TConstArrayView<UObject*> LoadedObjects, TArray<UObject*>& OutLoadedCandidates);
};
