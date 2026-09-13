#pragma once

#include "AI/BertaGASBehaviorTreeTypes.h"
#include "BehaviorTree/BTTaskNode.h"

#include "BertaBTTask_WaitAttributeThreshold.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;
struct FOnAttributeChangeData;

/** Waits until one controlled-Pawn GAS attribute satisfies a numeric condition. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_WaitAttributeThreshold : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_WaitAttributeThreshold(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Attribute")
	FBertaGameplayAttributeCondition Condition;

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	bool IsConditionSatisfied(const UAbilitySystemComponent& AbilitySystemComponent) const;
	void HandleAttributeChanged(const FOnAttributeChangeData& ChangeData);
	void UnregisterAttributeEvent();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	FGameplayAttribute ObservedAttribute;
	FDelegateHandle AttributeChangedDelegateHandle;
	bool bIsWaiting = false;
};
