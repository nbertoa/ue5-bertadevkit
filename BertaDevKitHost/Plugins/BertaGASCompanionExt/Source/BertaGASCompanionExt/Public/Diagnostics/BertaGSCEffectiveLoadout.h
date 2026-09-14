#pragma once

#include "Abilities/BertaGSCAbilityReadiness.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaGSCEffectiveLoadout.generated.h"

class AActor;
class UAttributeSet;
class UGameplayEffect;

/** Strength of the runtime evidence recorded for an item's origin. */
UENUM(BlueprintType)
enum class EBertaGSCProvenanceConfidence : uint8
{
	Proven,
	Inferred,
	Unknown
};

USTRUCT(BlueprintType)
struct BERTAGASCOMPANIONEXT_API FBertaGSCLoadoutAbility
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FBertaGSCAbilityReadinessEntry Readiness;

	/** Runtime FGameplayAbilitySpec::SourceObject evidence; this is not automatically an Ability Set grant claim. */
	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString SourceObjectPath;

	/** Active Gameplay Effect linked by the ASC as the grant source for this exact spec, when available. */
	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString GrantingGameplayEffectClassPath;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	EBertaGSCProvenanceConfidence ProvenanceConfidence = EBertaGSCProvenanceConfidence::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString ProvenanceEvidence;
};

USTRUCT(BlueprintType)
struct BERTAGASCOMPANIONEXT_API FBertaGSCLoadoutGameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString EffectClassPath;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString ActiveHandle;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	int32 StackCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	float DurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	float TimeRemainingSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	bool bInfiniteDuration = false;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString InstigatorPath;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString EffectCauserPath;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString SourceObjectPath;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	EBertaGSCProvenanceConfidence ProvenanceConfidence = EBertaGSCProvenanceConfidence::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString ProvenanceEvidence;
};

USTRUCT(BlueprintType)
struct BERTAGASCOMPANIONEXT_API FBertaGSCLoadoutAttributeSet
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	TSubclassOf<UAttributeSet> AttributeSetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString AttributeSetClassPath;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString ObjectPath;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	EBertaGSCProvenanceConfidence ProvenanceConfidence = EBertaGSCProvenanceConfidence::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString ProvenanceEvidence;
};

USTRUCT(BlueprintType)
struct BERTAGASCOMPANIONEXT_API FBertaGSCLoadoutGameplayTag
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FGameplayTag Tag;

	/** Effective local ASC count. GAS does not publicly break this count down by loose/effect source. */
	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	int32 Count = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	EBertaGSCProvenanceConfidence ProvenanceConfidence = EBertaGSCProvenanceConfidence::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString ProvenanceEvidence;
};

/** Read-only snapshot of the effective state known by the Actor's local ASC. */
USTRUCT(BlueprintType)
struct BERTAGASCOMPANIONEXT_API FBertaGSCEffectiveLoadoutSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString ActorPath;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString AbilitySystemComponentPath;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	bool bAbilitySystemComponentFound = false;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	TArray<FBertaGSCLoadoutAbility> Abilities;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	TArray<FBertaGSCLoadoutGameplayEffect> ActiveGameplayEffects;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	TArray<FBertaGSCLoadoutAttributeSet> AttributeSets;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	TArray<FBertaGSCLoadoutGameplayTag> OwnedGameplayTags;

	UPROPERTY(BlueprintReadOnly, Category = "Effective Loadout")
	FString Summary;
};

UCLASS()
class BERTAGASCOMPANIONEXT_API UBertaGSCEffectiveLoadoutLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BertaGASCompanionExt|Loadout", meta = (ReturnDisplayName = "Success"))
	static bool BuildEffectiveLoadoutSnapshot(AActor* Actor, FBertaGSCEffectiveLoadoutSnapshot& OutSnapshot);

	UFUNCTION(BlueprintPure, Category = "BertaGASCompanionExt|Loadout")
	static FString FormatEffectiveLoadoutSnapshot(const FBertaGSCEffectiveLoadoutSnapshot& Snapshot);

	UFUNCTION(BlueprintPure, Category = "BertaGASCompanionExt|Loadout")
	static FString FormatProvenanceConfidence(EBertaGSCProvenanceConfidence Confidence);
};
