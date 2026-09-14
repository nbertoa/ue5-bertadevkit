#pragma once

#include "CoreMinimal.h"

class AActor;
class UAbilitySystemComponent;
class UGameplayAbility;
struct FBertaGSCAbilityActivationReport;
struct FGameplayAbilitySpec;
struct FGameplayTagContainer;

namespace BertaGSCAbilityActivation
{
	bool BuildForSpec(
		AActor* Actor,
		UAbilitySystemComponent& AbilitySystemComponent,
		const FGameplayAbilitySpec& AbilitySpec,
		FBertaGSCAbilityActivationReport& OutReport);

	FString BoolText(bool bValue);
	FString SortedTags(const FGameplayTagContainer& Tags);
	const UGameplayAbility* GetCurrentQueuedAbility(
		AActor* Actor,
		UAbilitySystemComponent& AbilitySystemComponent);
}
