#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "BertaGASAbilityTypes.generated.h"

/** Read-only cooldown state for one granted Gameplay Ability. */
USTRUCT(BlueprintType)
struct BERTADEVKIT_API FBertaAbilityCooldownInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Ability")
	bool bIsOnCooldown = false;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Ability")
	float TimeRemainingSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Ability")
	float DurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Ability")
	float RemainingNormalized = 0.0f;
};

/** Read-only result of evaluating whether one granted Gameplay Ability can pay its cost. */
USTRUCT(BlueprintType)
struct BERTADEVKIT_API FBertaAbilityCostCheckResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Ability")
	bool bCanPayCost = false;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Ability")
	FGameplayTagContainer FailureTags;
};

/** Read-only result of evaluating all current activation rules for one granted Gameplay Ability. */
USTRUCT(BlueprintType)
struct BERTADEVKIT_API FBertaAbilityActivationCheckResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Ability")
	bool bCanActivate = false;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Ability")
	FGameplayTagContainer FailureTags;
};
