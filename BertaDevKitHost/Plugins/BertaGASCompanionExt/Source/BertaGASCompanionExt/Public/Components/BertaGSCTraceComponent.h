#pragma once

#include "AttributeSet.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"

#include "BertaGSCTraceComponent.generated.h"

class UGameplayAbility;
class UGSCCoreComponent;

UENUM(BlueprintType)
enum class EBertaGSCTraceEventType : uint8
{
	AbilityActivated,
	AbilityEnded,
	AbilityFailed,
	AbilityCommitted,
	CooldownStarted,
	CooldownEnded,
	GameplayEffectAdded,
	GameplayEffectRemoved,
	GameplayEffectStackChanged,
	GameplayEffectTimeChanged,
	GameplayTagChanged,
	AttributeChanged
};

/** One immutable diagnostic observation captured from GAS Companion public delegates. */
USTRUCT(BlueprintType)
struct BERTAGASCOMPANIONEXT_API FBertaGSCTraceEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	float RelativeTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	float WorldTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	EBertaGSCTraceEventType Type = EBertaGSCTraceEventType::AbilityActivated;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	FString ActorPath;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	FString AbilityClassPath;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	FString GameplayEffectClassPath;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	FGameplayTag GameplayTag;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	bool bGameplayTagPresent = false;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	FGameplayTagContainer GameplayTags;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	FString AttributeName;

	/** True only when OldValue came from an earlier observed value or an explicit old-value delegate parameter. */
	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	bool bOldValueKnown = false;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	float OldValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	float NewValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	float DeltaValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	float TimeRemainingSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	float DurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	float GameplayEffectStartTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Trace")
	FString Message;
};

/** Opt-in event recorder for the GAS Companion delegate surface. This component never ticks. */
UCLASS(ClassGroup = (BertaGASCompanionExt), meta = (BlueprintSpawnableComponent))
class BERTAGASCOMPANIONEXT_API UBertaGSCTraceComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UBertaGSCTraceComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaGASCompanionExt|Trace", meta = (ClampMin = "1", ClampMax = "10000"))
	int32 MaximumEventCount = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaGASCompanionExt|Trace")
	bool bLogEvents = false;

	UFUNCTION(BlueprintCallable, Category = "BertaGASCompanionExt|Trace", meta = (ReturnDisplayName = "Success"))
	bool StartTracing();

	UFUNCTION(BlueprintCallable, Category = "BertaGASCompanionExt|Trace")
	void StopTracing();

	UFUNCTION(BlueprintCallable, Category = "BertaGASCompanionExt|Trace")
	void ClearTrace();

	UFUNCTION(BlueprintPure, Category = "BertaGASCompanionExt|Trace")
	TArray<FBertaGSCTraceEvent> GetTraceEvents() const;

	UFUNCTION(BlueprintPure, Category = "BertaGASCompanionExt|Trace")
	FString DumpTraceToText() const;

	UFUNCTION(BlueprintPure, Category = "BertaGASCompanionExt|Trace")
	bool IsTracing() const;

	static FString FormatTraceEvent(const FBertaGSCTraceEvent& Event);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UGSCCoreComponent> BoundCoreComponent = nullptr;

	UPROPERTY(Transient)
	TArray<FBertaGSCTraceEvent> Events;

	TMap<FActiveGameplayEffectHandle, FString> GameplayEffectPaths;
	TMap<FGameplayAttribute, float> LastObservedAttributeValues;
	float TraceStartWorldTimeSeconds = 0.0f;

	FBertaGSCTraceEvent MakeEvent(EBertaGSCTraceEventType Type) const;
	void AddEvent(FBertaGSCTraceEvent&& Event);
	FString ResolveGameplayEffectPath(FActiveGameplayEffectHandle ActiveHandle) const;

	UFUNCTION()
	void HandleAbilityActivated(const UGameplayAbility* Ability);

	UFUNCTION()
	void HandleAbilityEnded(const UGameplayAbility* Ability);

	UFUNCTION()
	void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& ReasonTags);

	UFUNCTION()
	void HandleAbilityCommitted(UGameplayAbility* Ability);

	UFUNCTION()
	void HandleCooldownStarted(UGameplayAbility* Ability, const FGameplayTagContainer CooldownTags, float TimeRemaining, float Duration);

	UFUNCTION()
	void HandleCooldownEnded(UGameplayAbility* Ability, FGameplayTag CooldownTag, float Duration);

	UFUNCTION()
	void HandleGameplayEffectAdded(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle);

	UFUNCTION()
	void HandleGameplayEffectRemoved(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle);

	UFUNCTION()
	void HandleGameplayEffectStackChanged(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle, int32 NewStackCount, int32 OldStackCount);

	UFUNCTION()
	void HandleGameplayEffectTimeChanged(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle, float NewStartTime, float NewDuration);

	UFUNCTION()
	void HandleGameplayTagChanged(FGameplayTag ChangedTag, int32 NewTagCount);

	UFUNCTION()
	void HandleAttributeChanged(FGameplayAttribute Attribute, float DeltaValue, const FGameplayTagContainer EventTags);
};
