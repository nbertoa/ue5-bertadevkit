#pragma once

#include "Abilities/GameplayAbilityTypes.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpecHandle.h"

#include "BertaBTTask_WaitAbilityEnd.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;
class UGameplayAbility;

/** Waits until no active execution remains for one exact granted ability class. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_WaitAbilityEnd : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_WaitAbilityEnd(const FObjectInitializer& ObjectInitializer);

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

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	void HandleAbilityEnded(const FAbilityEndedData& EndedData);
	void UnregisterAbilityEvent();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	FGameplayAbilitySpecHandle AbilitySpecHandle;
	FDelegateHandle AbilityEndedDelegateHandle;
	bool bIsWaiting = false;
};
