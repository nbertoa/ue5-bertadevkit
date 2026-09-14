#pragma once

#include "AI/BertaGASAbilityTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaGASAbilityUtils.generated.h"

class AActor;
class UGameplayAbility;

/** Side-effect-free inspection of Gameplay Abilities granted to an Actor's ASC. */
UCLASS()
class BERTADEVKIT_API UBertaGASAbilityUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDevKit|GAS|Abilities",
		meta = (ReturnDisplayName = "Success"))
	static bool GetAbilityCooldownInfo(
		AActor* Actor,
		TSubclassOf<UGameplayAbility> AbilityClass,
		FBertaAbilityCooldownInfo& OutInfo);

	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDevKit|GAS|Abilities",
		meta = (ReturnDisplayName = "Success"))
	static bool CheckAbilityCost(
		AActor* Actor,
		TSubclassOf<UGameplayAbility> AbilityClass,
		FBertaAbilityCostCheckResult& OutResult);

	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDevKit|GAS|Abilities",
		meta = (ReturnDisplayName = "Success"))
	static bool CheckAbilityActivation(
		AActor* Actor,
		TSubclassOf<UGameplayAbility> AbilityClass,
		FBertaAbilityActivationCheckResult& OutResult);
};
