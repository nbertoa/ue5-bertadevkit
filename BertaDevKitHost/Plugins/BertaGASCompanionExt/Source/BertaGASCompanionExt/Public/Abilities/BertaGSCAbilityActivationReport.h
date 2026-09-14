#pragma once

#include "AI/BertaGASAbilityTypes.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaGSCAbilityActivationReport.generated.h"

class AActor;
class UGameplayAbility;
class UInputAction;

/** Read-only explanation of the current activation state for one exact granted ability class. */
USTRUCT(BlueprintType)
struct BERTAGASCOMPANIONEXT_API FBertaGSCAbilityActivationReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	FString ActorPath;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	FString AbilityClassPath;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	bool bAbilitySystemComponentFound = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	bool bExactAbilityGranted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	int32 GrantedLevel = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	bool bAbilityActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	FBertaAbilityCostCheckResult Cost;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	bool bCostCheckAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	FBertaAbilityCooldownInfo Cooldown;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	bool bCooldownCheckAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	FBertaAbilityActivationCheckResult Activation;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	bool bActivationCheckAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	TObjectPtr<UInputAction> BoundInputAction = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation|Queue")
	bool bAbilityQueueComponentFound = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation|Queue")
	bool bAbilityQueueEnabled = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation|Queue")
	bool bAbilityQueueOpened = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation|Queue")
	bool bAbilityQueueAllowsAll = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation|Queue")
	FString QueuedAbilityClassPath;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation|Queue")
	TArray<TSubclassOf<UGameplayAbility>> QueueAllowedAbilityClasses;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Activation")
	FString Summary;
};

/** Side-effect-free GAS Companion activation diagnostics. */
UCLASS()
class BERTAGASCOMPANIONEXT_API UBertaGSCAbilityActivationLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BertaGASCompanionExt|Abilities", meta = (DisplayName = "Build GSC Ability Activation Report", ReturnDisplayName = "Success"))
	static bool BuildAbilityActivationReport(
		AActor* Actor,
		TSubclassOf<UGameplayAbility> ExactAbilityClass,
		FBertaGSCAbilityActivationReport& OutReport);

	UFUNCTION(BlueprintPure, Category = "BertaGASCompanionExt|Abilities")
	static FString FormatAbilityActivationReport(const FBertaGSCAbilityActivationReport& Report);
};
