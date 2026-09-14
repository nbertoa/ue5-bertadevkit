#pragma once

#include "Graph/ComboGraphNodeMontage.h"
#include "Graph/ComboGraphNodeSequence.h"
#include "GameplayTagContainer.h"

#include "BertaComboGraphTargetingNodes.generated.h"

class UTargetingPreset;

UINTERFACE(MinimalAPI)
class UBertaComboGraphTargetingProvider : public UInterface
{
	GENERATED_BODY()
};

class BERTACOMBOGRAPHEXT_API IBertaComboGraphTargetingProvider
{
	GENERATED_BODY()

public:
	virtual const UTargetingPreset* GetTargetingPresetForEffectEvent(FGameplayTag EventTag) const = 0;
};

/** Montage node with optional per-effect-event Targeting System presets. */
UCLASS(Blueprintable)
class BERTACOMBOGRAPHEXT_API UBertaComboGraphNodeMontage : public UComboGraphNodeMontage, public IBertaComboGraphTargetingProvider
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BertaComboGraphExt|Targeting")
	TMap<FGameplayTag, TObjectPtr<UTargetingPreset>> EffectTargetingPresets;

	virtual const UTargetingPreset* GetTargetingPresetForEffectEvent(FGameplayTag EventTag) const override;
};

/** Sequence node with optional per-effect-event Targeting System presets. */
UCLASS(Blueprintable)
class BERTACOMBOGRAPHEXT_API UBertaComboGraphNodeSequence : public UComboGraphNodeSequence, public IBertaComboGraphTargetingProvider
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BertaComboGraphExt|Targeting")
	TMap<FGameplayTag, TObjectPtr<UTargetingPreset>> EffectTargetingPresets;

	virtual const UTargetingPreset* GetTargetingPresetForEffectEvent(FGameplayTag EventTag) const override;
};
