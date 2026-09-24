#include "AssetActions/BertaAssetNamingBatch.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Log/BertaDevKitEditorLog.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

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

bool BertaAssetNamingBatch::IsProjectAsset(const FAssetData& AssetData)
{
	const FString Path = AssetData.PackagePath.ToString();
	return AssetData.IsValid()
		&& (Path == TEXT("/Game") || Path.StartsWith(TEXT("/Game/")))
		&& FPackageName::GetLongPackagePath(AssetData.PackageName.ToString()) == Path;
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
	if (!IsProjectAsset(AssetData))
	{
		OutFailureReason = NSLOCTEXT("BertaDevKit", "AssetNamingBatchOutsideProject", "Asset must be valid and located under /Game.");
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

FBertaAssetNamingBatchPreflightResult BertaAssetNamingBatch::PreflightInEditor(const TArray<FBertaAssetNamingBatchCandidate>& Candidates)
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	return Preflight(Candidates, [&AssetRegistry](const FBertaAssetNamingBatchCandidate& Candidate)
	{
		return AssetRegistry.GetAssetByObjectPath(Candidate.TargetObjectPath).IsValid()
			|| FindPackage(nullptr, *Candidate.TargetPackageName)
			|| FPackageName::DoesPackageExist(Candidate.TargetPackageName);
	});
}

bool BertaAssetNamingBatch::VerifyPostflight(const TArray<FBertaAssetNamingBatchCandidate>& Candidates, const TArray<FString>& CurrentObjectPaths, const bool bAssetToolsSucceeded)
{
	if (!bAssetToolsSucceeded || Candidates.Num() != CurrentObjectPaths.Num())
	{
		return false;
	}
	for (int32 Index = 0; Index < Candidates.Num(); ++Index)
	{
		if (CurrentObjectPaths[Index] != Candidates[Index].TargetObjectPath)
		{
			return false;
		}
	}
	return true;
}

bool BertaAssetNamingBatch::Execute(const TArray<FBertaAssetNamingBatchCandidate>& Candidates, const TArray<UObject*>& LoadedAssets)
{
	if (!ensureMsgf(Candidates.Num() == LoadedAssets.Num(), TEXT("Asset naming batch candidates and loaded assets must have matching counts.")))
	{
		return false;
	}
	const FBertaAssetNamingBatchPreflightResult PreflightResult = PreflightInEditor(Candidates);
	if (!PreflightResult.IsSafe())
	{
		for (const FBertaAssetNamingBatchConflict& Conflict : PreflightResult.Conflicts)
		{
			UE_LOG(LogBertaDevKitEditor, Error, TEXT("[AssetNaming] Rename preflight changed before execution: %s -> %s (%s)"), *Conflict.SourceObjectPath, *Conflict.TargetObjectPath, *Conflict.Reason.ToString());
		}
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
	TArray<TStrongObjectPtr<UObject>> AssetReferences;
	AssetReferences.Reserve(Candidates.Num());
	for (int32 CandidateIndex = 0; CandidateIndex < Candidates.Num(); ++CandidateIndex)
	{
		UObject* Asset = LoadedAssets[CandidateIndex];
		const FBertaAssetNamingBatchCandidate& Candidate = Candidates[CandidateIndex];
		AssetReferences.Emplace(Asset);
		RenameData.Emplace(Asset, Candidate.TargetPackagePath, Candidate.Plan.TargetName);
	}

	const bool bAssetToolsSucceeded = FAssetToolsModule::GetModule().Get().RenameAssets(RenameData);
	TArray<FString> CurrentObjectPaths;
	CurrentObjectPaths.Reserve(LoadedAssets.Num());
	for (const UObject* Asset : LoadedAssets)
	{
		CurrentObjectPaths.Add(Asset->GetPathName());
	}
	return VerifyPostflight(Candidates, CurrentObjectPaths, bAssetToolsSucceeded);
}
