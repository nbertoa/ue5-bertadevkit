#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"

#include "BertaGSCAbilityQueueInputBridgeComponent.generated.h"

class UGameplayAbility;
class UGSCAbilityInputBindingComponent;
class UGSCAbilityQueueComponent;
class UGSCCoreComponent;
class UAbilitySystemComponent;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBertaGSCQueueBridgeDiagnostic, const FString&, Message);

/**
 * Bridges explicitly reported Enhanced Input activation failures to the public GSC Ability Queue state.
 * Ability end forwarding is deferred one frame so native GSC handling wins and duplicate activation is avoided.
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

	/**
	 * Reports a failed activation from the handler of SourceInputAction.
	 * Call this only when that exact input handler's activation attempt has returned false.
	 */
	UFUNCTION(BlueprintCallable, Category = "BertaGASCompanionExt|Ability Queue", meta = (ReturnDisplayName = "Accepted"))
	bool ReportInputDrivenFailure(
		UInputAction* SourceInputAction,
		TSubclassOf<UGameplayAbility> AbilityClass,
		const FGameplayTagContainer& ReasonTags);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;

private:
	TWeakObjectPtr<UAbilitySystemComponent> BoundAbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<UGSCCoreComponent> CoreComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UGSCAbilityInputBindingComponent> AbilityInputBindingComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UGSCAbilityQueueComponent> AbilityQueueComponent = nullptr;

	TWeakObjectPtr<UGameplayAbility> PendingEndedAbility;
	TSubclassOf<UGameplayAbility> PendingEndedAbilityClass;
	TSubclassOf<UGameplayAbility> ExplicitlySubmittedAbilityClass;
	FTimerHandle ReconciliationTimerHandle;
	FDelegateHandle AbilityEndedDelegateHandle;
	bool bBridgeStarted = false;

	bool RefreshAbilitySystemBinding();
	void UnbindAbilitySystemComponent();
	void ScheduleReconciliation();
	void ReconcilePublicQueueState();
	void EmitDiagnostic(const FString& Message);
	bool IsAbilityAllowed(TSubclassOf<UGameplayAbility> AbilityClass) const;

	UFUNCTION()
	void HandleAbilityActorInfoInitialized();

	void HandleAbilityEnded(UGameplayAbility* Ability);
};
