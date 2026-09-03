#include "AssetActions/BertaAssetNamingValidator.h"

#include "AssetActions/BertaAssetNamingUtils.h"

#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "BertaAssetNamingValidator"

namespace
{
	bool IsGameAsset(const FAssetData& AssetData)
	{
		const FString PackageName = AssetData.PackageName.ToString();
		return PackageName == TEXT("/Game") || PackageName.StartsWith(TEXT("/Game/"));
	}

	bool IsSupportedNamingPlan(const FBertaAssetNamingPlan& Plan)
	{
		return Plan.Status == EBertaAssetNamingStatus::AlreadyCorrect || Plan.Status == EBertaAssetNamingStatus::NeedsRename;
	}
}

bool UBertaAssetNamingValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
	if (InContext.GetValidationUsecase() == EDataValidationUsecase::Save || !IsGameAsset(InAssetData))
	{
		return false;
	}

	return IsSupportedNamingPlan(UBertaAssetNamingUtils::BuildRenamePlan(InAssetData));
}

EDataValidationResult UBertaAssetNamingValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext)
{
	const FBertaAssetNamingPlan Plan = UBertaAssetNamingUtils::BuildRenamePlan(InAssetData);
	if (Plan.Status == EBertaAssetNamingStatus::AlreadyCorrect)
	{
		AssetPasses(InAsset);
		return EDataValidationResult::Valid;
	}

	if (Plan.Status == EBertaAssetNamingStatus::NeedsRename)
	{
		AssetFails(InAsset, FText::Format(
			LOCTEXT("AssetNamingViolation", "Asset naming violation: '{0}' should be '{1}' (expected prefix '{2}'). Use BertaDevKit Fix Asset Naming to repair it."),
			FText::FromString(InAssetData.GetObjectPathString()),
			FText::FromString(Plan.TargetName),
			FText::FromString(Plan.ExpectedPrefix)));
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::NotValidated;
}

#undef LOCTEXT_NAMESPACE
