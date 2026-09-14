#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"

#include "BertaGSCAbilityQueueInputBridgeComponent.generated.h"

class UGameplayAbility;
class UGSCAbilityQueueComponent;
class UGSCCoreComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBertaGSCQueueBridgeDiagnostic, const FString&, Message);

/**
 * Reconciles input-driven public GSC ability failure/end events with the public Ability Queue state.
 * Forwarding is deferred one frame so native GSC handling wins and duplicate activation is avoided.
 */
UCLASS(ClassGroup = (BertaGASCompanionExt), meta = (BlueprintSpawnableComponent))
class BERTAGASCOMPANIONEXT_API UBertaGSCAbilityQueueInputBridgeComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UBertaGSCAbilityQueueInputBridgeComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaGASCompanionExt|Ability Queue")
	bool bStartAutomatically = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaGASCompanionExt|Ability Queue")
	bool bLogDiagnostics = false;

	UPROPERTY(BlueprintAssignable, Category = "BertaGASCompanionExt|Ability Queue")
	FBertaGSCQueueBridgeDiagnostic OnDiagnostic;

	UFUNCTION(BlueprintCallable, Category = "BertaGASCompanionExt|Ability Queue", meta = (ReturnDisplayName = "Success"))
	bool StartBridge();

	UFUNCTION(BlueprintCallable, Category = "BertaGASCompanionExt|Ability Queue")
	void StopBridge();

	UFUNCTION(BlueprintPure, Category = "BertaGASCompanionExt|Ability Queue")
	bool IsBridgeActive() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;

private:
	struct FPendingFailure
	{
		TWeakObjectPtr<UGameplayAbility> Ability;
		TSubclassOf<UGameplayAbility> AbilityClass;
		FGameplayTagContainer ReasonTags;
		bool bObservedInPublicQueue = false;
	};

	UPROPERTY(Transient)
	TObjectPtr<UGSCCoreComponent> BoundCoreComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UGSCAbilityQueueComponent> AbilityQueueComponent = nullptr;

	TArray<FPendingFailure> PendingFailures;
	TWeakObjectPtr<UGameplayAbility> PendingEndedAbility;
	TSubclassOf<UGameplayAbility> PendingEndedAbilityClass;
	FTimerHandle ReconciliationTimerHandle;

	void ScheduleReconciliation();
	void ReconcilePublicQueueState();
	void EmitDiagnostic(const FString& Message);
	bool IsAbilityAllowed(const UGameplayAbility& Ability) const;

	UFUNCTION()
	void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& ReasonTags);

	UFUNCTION()
	void HandleAbilityEnded(const UGameplayAbility* Ability);
};
