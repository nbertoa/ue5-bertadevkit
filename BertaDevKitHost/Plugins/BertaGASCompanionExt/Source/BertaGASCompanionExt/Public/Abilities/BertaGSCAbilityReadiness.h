#pragma once

#include "Abilities/BertaGSCAbilityActivationReport.h"
#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaGSCAbilityReadiness.generated.h"

class AActor;
class UGameplayAbility;

UENUM(BlueprintType)
enum class EBertaGSCAbilityReadinessState : uint8
{
	Ready,
	Active,
	BlockedByCost,
	BlockedByCooldown,
	BlockedByActivation,
	NotFullyInspectable
};

/** Read-only state of one exact granted ability spec. Multiple blockers may be present. */
USTRUCT(BlueprintType)
struct BERTAGASCOMPANIONEXT_API FBertaGSCAbilityReadinessEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	FBertaGSCAbilityActivationReport Inspection;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	TArray<EBertaGSCAbilityReadinessState> States;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	bool bReady = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	bool bHasProblem = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness|Queue")
	bool bQueueEligibilityKnown = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness|Queue")
	bool bAllowedByCurrentQueue = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness|Queue")
	bool bIsCurrentlyQueued = false;

	/** False only when GSC exposes a queued class/instance that cannot be correlated to one of duplicate same-class specs. */
	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness|Queue")
	bool bCurrentQueuedSpecKnown = false;
};

/** Deterministically ordered readiness snapshot for every ability granted to one local ASC. */
USTRUCT(BlueprintType)
struct BERTAGASCOMPANIONEXT_API FBertaGSCAbilityReadinessReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	FString ActorPath;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	FString AbilitySystemComponentPath;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	bool bAbilitySystemComponentFound = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	int32 TotalGrantedAbilities = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	int32 ReadyCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	int32 ActiveCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	int32 ProblemCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	TArray<FBertaGSCAbilityReadinessEntry> Entries;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	FString Summary;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Readiness")
	FString ProblemsSummary;
};

/** Side-effect-free actor-wide GAS Companion readiness diagnostics. */
UCLASS()
class BERTAGASCOMPANIONEXT_API UBertaGSCAbilityReadinessLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BertaGASCompanionExt|Diagnostics", meta = (ReturnDisplayName = "Success"))
	static bool BuildAbilityReadinessReport(AActor* Actor, FBertaGSCAbilityReadinessReport& OutReport);

	UFUNCTION(BlueprintPure, Category = "BertaGASCompanionExt|Diagnostics")
	static FString FormatAbilityReadinessReport(const FBertaGSCAbilityReadinessReport& Report);

	UFUNCTION(BlueprintPure, Category = "BertaGASCompanionExt|Diagnostics")
	static FString FormatAbilityReadinessProblems(const FBertaGSCAbilityReadinessReport& Report);
};
