#include "Targeting/BertaComboGraphTargetingNodes.h"

#include "TargetingSystem/TargetingPreset.h"

const UTargetingPreset* UBertaComboGraphNodeMontage::GetTargetingPresetForEffectEvent(const FGameplayTag EventTag) const
{
	const TObjectPtr<UTargetingPreset>* Preset = EffectTargetingPresets.Find(EventTag);
	return Preset ? Preset->Get() : nullptr;
}

const UTargetingPreset* UBertaComboGraphNodeSequence::GetTargetingPresetForEffectEvent(const FGameplayTag EventTag) const
{
	const TObjectPtr<UTargetingPreset>* Preset = EffectTargetingPresets.Find(EventTag);
	return Preset ? Preset->Get() : nullptr;
}
