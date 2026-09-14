#pragma once

#include "Abilities/Tasks/ComboGraphAbilityTask_StartGraph.h"

#include "BertaComboGraphAbilityTask_StartGraphTargeting.generated.h"

class UComboGraph;
class UInputAction;

/** Opt-in Combo Graph task that replaces only configured effect-container target resolution. */
UCLASS()
class BERTACOMBOGRAPHEXT_API UBertaComboGraphAbilityTask_StartGraphTargeting final : public UComboGraphAbilityTask_StartGraph
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BertaComboGraphExt|Targeting", meta = (DisplayName = "Start Combo Graph with Berta Targeting", AdvancedDisplay = "bBroadcastInternalEvents, InitialInput", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UBertaComboGraphAbilityTask_StartGraphTargeting* StartComboGraphWithTargeting(
		UGameplayAbility* OwningAbility,
		UComboGraph* ComboGraph,
		UInputAction* InputAction,
		bool bBroadcastInternalEvents = false);

protected:
	virtual FComboGraphGameplayEffectContainerSpec MakeEffectContainerSpecFromContainer(
		const FComboGraphGameplayEffectContainer& Container,
		FGameplayTag ContainerTag,
		const FGameplayEventData& EventData,
		int32 OverrideGameplayLevel = -1) override;
};
