#pragma once

#include "AI/BertaGASBehaviorTreeTypes.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"

#include "BertaBTTask_WaitTargetAttributeThreshold.generated.h"

class UAbilitySystemComponent;
class UBehaviorTree;
class UBehaviorTreeComponent;
class UBlackboardComponent;
struct FOnAttributeChangeData;

/** Waits until an attribute on the current Blackboard-selected Actor satisfies a numeric condition. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_WaitTargetAttributeThreshold : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_WaitTargetAttributeThreshold(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Gameplay Attribute")
	FBertaGameplayAttributeCondition Condition;

private:
	UAbilitySystemComponent* ResolveTargetAbilitySystemComponent(const UBlackboardComponent& Blackboard) const;
	bool IsCurrentTargetSatisfied(const UBlackboardComponent& Blackboard) const;
	void BindTargetAbilitySystemComponent(UBlackboardComponent& Blackboard);
	void UnbindTargetAttributeEvent();
	void UnregisterObservers();
	void CompleteIfSatisfied();
	void HandleAttributeChanged(const FOnAttributeChangeData& ChangeData);
	EBlackboardNotificationResult HandleTargetActorChanged(
		const UBlackboardComponent& Blackboard,
		FBlackboard::FKey ChangedKeyId);

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	TWeakObjectPtr<UBlackboardComponent> ObservedBlackboardComponent;
	FGameplayAttribute ObservedAttribute;
	FDelegateHandle AttributeChangedDelegateHandle;
	FDelegateHandle BlackboardObserverHandle;
	bool bIsWaiting = false;
};
