#pragma once

#include "AssetActions/BertaAssetNamingUtils.h"

struct FBertaAssetNamingBatchCandidate
{
	FAssetData AssetData;
	FBertaAssetNamingPlan Plan;
	FString SourceObjectPath;
	FString TargetPackagePath;
	FString TargetPackageName;
	FString TargetObjectPath;
};

enum class EBertaAssetNamingBatchConflictType : uint8
{
	DuplicateTarget,
	OccupiedTarget,
	InvalidTarget,
};

struct FBertaAssetNamingBatchConflict
{
	EBertaAssetNamingBatchConflictType Type;
	int32 CandidateIndex = INDEX_NONE;
	FString SourceObjectPath;
	FString TargetObjectPath;
	FText Reason;
};

struct FBertaAssetNamingBatchPreflightResult
{
	TArray<FBertaAssetNamingBatchConflict> Conflicts;

	bool IsSafe() const
	{
		return Conflicts.IsEmpty();
	}
};

namespace BertaAssetNamingBatch
{
	bool BuildCandidate(const FAssetData& AssetData, const FBertaAssetNamingPlan& Plan, FBertaAssetNamingBatchCandidate& OutCandidate, FText& OutFailureReason);
	FBertaAssetNamingBatchPreflightResult Preflight(const TArray<FBertaAssetNamingBatchCandidate>& Candidates, TFunctionRef<bool(const FBertaAssetNamingBatchCandidate&)> IsTargetOccupied);
	bool Execute(const TArray<FBertaAssetNamingBatchCandidate>& Candidates, const TArray<UObject*>& LoadedAssets);
}
