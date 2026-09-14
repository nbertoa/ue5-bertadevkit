#pragma once

#include "EditorValidatorBase.h"

#include "BertaGSCAbilitySetValidator.generated.h"

class UDataTable;

namespace BertaGSCAbilitySetValidation
{
	bool IsGameplayEffectLevelValid(float Level);
	bool IsInitializationDataResolvable(const TSoftObjectPtr<UDataTable>& InitializationData);
	bool IsInitializationDataRowStructureValid(const TSoftObjectPtr<UDataTable>& InitializationData);
}

/** Objective Data Validation checks for GAS Companion Ability Set assets. */
UCLASS()
class UBertaGSCAbilitySetValidator final : public UEditorValidatorBase
{
	GENERATED_BODY()

protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) override;
};
