#pragma once

#include "Abilities/GameplayAbilityTypes.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpecHandle.h"

#include "BertaBTTask_ActivateGameplayAbilityAndWait.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;
class UGameplayAbility;

/** Activates one granted ability and waits for that exact execution to end. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_ActivateGameplayAbilityAndWait : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_ActivateGameplayAbilityAndWait(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	bool bAllowRemoteActivation = true;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	bool bCancelAbilityOnAbort = true;

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	void HandleAbilityActivated(UGameplayAbility* Ability);
	void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureTags);
	void HandleAbilityEnded(const FAbilityEndedData& EndedData);
	void CompleteTask(EBTNodeResult::Type Result);
	void UnregisterDelegates();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	TWeakObjectPtr<UGameplayAbility> ActivatedAbility;
	FGameplayAbilitySpecHandle AbilitySpecHandle;
	FDelegateHandle AbilityActivatedDelegateHandle;
	FDelegateHandle AbilityFailedDelegateHandle;
	FDelegateHandle AbilityEndedDelegateHandle;
	EBTNodeResult::Type PendingExecutionResult = EBTNodeResult::InProgress;
	bool bIsWaiting = false;
	bool bIsExecutingTask = false;
};
