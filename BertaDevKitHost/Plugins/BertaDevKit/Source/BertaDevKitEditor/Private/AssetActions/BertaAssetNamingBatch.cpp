#include "AssetActions/BertaAssetNamingBatch.h"

#include "AssetToolsModule.h"
#include "Misc/PackageName.h"

namespace
{
	FBertaAssetNamingBatchConflict MakeConflict(EBertaAssetNamingBatchConflictType Type, int32 CandidateIndex, const FBertaAssetNamingBatchCandidate& Candidate, const FText& Reason)
	{
		FBertaAssetNamingBatchConflict Conflict;
		Conflict.Type = Type;
		Conflict.CandidateIndex = CandidateIndex;
		Conflict.SourceObjectPath = Candidate.SourceObjectPath;
		Conflict.TargetObjectPath = Candidate.TargetObjectPath;
		Conflict.Reason = Reason;
		return Conflict;
	}
}

bool BertaAssetNamingBatch::BuildCandidate(const FAssetData& AssetData, const FBertaAssetNamingPlan& Plan, FBertaAssetNamingBatchCandidate& OutCandidate, FText& OutFailureReason)
{
	OutCandidate = {};
	OutFailureReason = FText::GetEmpty();

	if (Plan.Status != EBertaAssetNamingStatus::NeedsRename)
	{
		OutFailureReason = NSLOCTEXT("BertaDevKit", "AssetNamingBatchNotRenameCandidate", "Asset does not require a rename.");
		return false;
	}

	OutCandidate.AssetData = AssetData;
	OutCandidate.Plan = Plan;
	OutCandidate.SourceObjectPath = AssetData.GetSoftObjectPath().ToString();
	if (OutCandidate.SourceObjectPath.IsEmpty())
	{
		OutFailureReason = NSLOCTEXT("BertaDevKit", "AssetNamingBatchMissingSourcePath", "Asset does not have a valid source object path.");
		return false;
	}

	OutCandidate.TargetPackagePath = FPackageName::GetLongPackagePath(AssetData.PackageName.ToString());
	OutCandidate.TargetPackageName = OutCandidate.TargetPackagePath + TEXT("/") + Plan.TargetName;
	OutCandidate.TargetObjectPath = FString::Printf(TEXT("%s.%s"), *OutCandidate.TargetPackageName, *Plan.TargetName);
	return true;
}

FBertaAssetNamingBatchPreflightResult BertaAssetNamingBatch::Preflight(const TArray<FBertaAssetNamingBatchCandidate>& Candidates, TFunctionRef<bool(const FBertaAssetNamingBatchCandidate&)> IsTargetOccupied)
{
	FBertaAssetNamingBatchPreflightResult Result;
	TMap<FString, TArray<int32>> TargetToCandidateIndices;

	for (int32 CandidateIndex = 0; CandidateIndex < Candidates.Num(); ++CandidateIndex)
	{
		const FBertaAssetNamingBatchCandidate& Candidate = Candidates[CandidateIndex];
		FText ValidationReason;
		if (!FPackageName::IsValidLongPackageName(Candidate.TargetPackageName, true, &ValidationReason))
		{
			Result.Conflicts.Add(MakeConflict(EBertaAssetNamingBatchConflictType::InvalidTarget, CandidateIndex, Candidate, ValidationReason));
			continue;
		}

		if (!FPackageName::IsValidObjectPath(Candidate.TargetObjectPath, &ValidationReason))
		{
			Result.Conflicts.Add(MakeConflict(EBertaAssetNamingBatchConflictType::InvalidTarget, CandidateIndex, Candidate, ValidationReason));
			continue;
		}

		TargetToCandidateIndices.FindOrAdd(Candidate.TargetObjectPath).Add(CandidateIndex);
	}

	for (const TPair<FString, TArray<int32>>& Pair : TargetToCandidateIndices)
	{
		if (Pair.Value.Num() < 2)
		{
			continue;
		}

		for (const int32 CandidateIndex : Pair.Value)
		{
			const FBertaAssetNamingBatchCandidate& Candidate = Candidates[CandidateIndex];
			Result.Conflicts.Add(MakeConflict(
				EBertaAssetNamingBatchConflictType::DuplicateTarget,
				CandidateIndex,
				Candidate,
				NSLOCTEXT("BertaDevKit", "AssetNamingBatchDuplicateTarget", "Another rename candidate has the same destination.")));
		}
	}

	for (int32 CandidateIndex = 0; CandidateIndex < Candidates.Num(); ++CandidateIndex)
	{
		const FBertaAssetNamingBatchCandidate& Candidate = Candidates[CandidateIndex];
		FText ValidationReason;
		if (!FPackageName::IsValidLongPackageName(Candidate.TargetPackageName, true, &ValidationReason)
			|| !FPackageName::IsValidObjectPath(Candidate.TargetObjectPath, &ValidationReason))
		{
			continue;
		}

		if (IsTargetOccupied(Candidate))
		{
			Result.Conflicts.Add(MakeConflict(
				EBertaAssetNamingBatchConflictType::OccupiedTarget,
				CandidateIndex,
				Candidate,
				NSLOCTEXT("BertaDevKit", "AssetNamingBatchOccupiedTarget", "The destination is already occupied.")));
		}
	}

	return Result;
}

bool BertaAssetNamingBatch::Execute(const TArray<FBertaAssetNamingBatchCandidate>& Candidates, const TArray<UObject*>& LoadedAssets)
{
	if (!ensureMsgf(Candidates.Num() == LoadedAssets.Num(), TEXT("Asset naming batch candidates and loaded assets must have matching counts.")))
	{
		return false;
	}

	for (int32 CandidateIndex = 0; CandidateIndex < Candidates.Num(); ++CandidateIndex)
	{
		UObject* Asset = LoadedAssets[CandidateIndex];
		const FBertaAssetNamingBatchCandidate& Candidate = Candidates[CandidateIndex];
		if (!ensureMsgf(IsValid(Asset), TEXT("Asset naming batch execution requires a valid loaded asset at index %d for source %s."), CandidateIndex, *Candidate.SourceObjectPath))
		{
			return false;
		}

		const FString CurrentObjectPath = Asset->GetPathName();
		if (!ensureMsgf(CurrentObjectPath == Candidate.SourceObjectPath, TEXT("Asset naming batch source mismatch at index %d: expected %s, loaded asset is %s."), CandidateIndex, *Candidate.SourceObjectPath, *CurrentObjectPath))
		{
			return false;
		}
	}

	TArray<FAssetRenameData> RenameData;
	RenameData.Reserve(Candidates.Num());
	for (int32 CandidateIndex = 0; CandidateIndex < Candidates.Num(); ++CandidateIndex)
	{
		UObject* Asset = LoadedAssets[CandidateIndex];
		const FBertaAssetNamingBatchCandidate& Candidate = Candidates[CandidateIndex];
		RenameData.Emplace(Asset, Candidate.TargetPackagePath, Candidate.Plan.TargetName);
	}

	return FAssetToolsModule::GetModule().Get().RenameAssets(RenameData);
}
