#include "Validation/BertaGSCAbilitySetValidator.h"

#include "Validation/BertaGSCContractAudit.h"
#include "Abilities/GSCAbilitySet.h"
#include "AttributeSet.h"
#include "Engine/DataTable.h"
#include "Misc/DataValidation.h"

namespace BertaGSCAbilitySetValidation
{
	bool IsGameplayEffectLevelValid(const float Level)
	{
		return FMath::IsFinite(Level);
	}

	bool IsInitializationDataResolvable(const TSoftObjectPtr<UDataTable>& InitializationData)
	{
		return InitializationData.IsNull() || InitializationData.LoadSynchronous() != nullptr;
	}

	bool IsInitializationDataRowStructureValid(const TSoftObjectPtr<UDataTable>& InitializationData)
	{
		if (InitializationData.IsNull())
		{
			return true;
		}
		const UDataTable* Table = InitializationData.LoadSynchronous();
		return Table && Table->GetRowStruct() == FAttributeMetaData::StaticStruct();
	}
}

bool UBertaGSCAbilitySetValidator::CanValidateAsset_Implementation(
	const FAssetData& InAssetData,
	UObject* InAsset,
	FDataValidationContext& InContext) const
{
	return InAsset && InAsset->IsA<UGSCAbilitySet>();
}

EDataValidationResult UBertaGSCAbilitySetValidator::ValidateLoadedAsset_Implementation(
	const FAssetData& InAssetData,
	UObject* InAsset,
	FDataValidationContext& InContext)
{
	FBertaGSCContractAuditReport Report;
	UBertaGSCContractAuditLibrary::AuditAsset(InAsset, Report);
	for (const FBertaGSCContractFinding& Finding : Report.Findings)
	{
		const FText FindingText = FText::FromString(FString::Printf(
			TEXT("%s: %s Evidence: %s"), *Finding.PropertyPath, *Finding.Message, *Finding.Evidence));
		if (Finding.Severity == EBertaGSCContractFindingSeverity::Error)
		{
			AssetFails(InAsset, FindingText);
		}
		else if (Finding.Severity == EBertaGSCContractFindingSeverity::Warning)
		{
			AssetWarning(InAsset, FindingText);
		}
	}

	if (Report.ErrorCount > 0)
	{
		return EDataValidationResult::Invalid;
	}
	AssetPasses(InAsset);
	return EDataValidationResult::Valid;
}
