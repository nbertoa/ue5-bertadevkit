#pragma once

#include "AI/BertaGASBehaviorTreeTypes.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"

#include "BertaBTTask_WaitTargetGameplayTagQuery.generated.h"

class UAbilitySystemComponent;
class UBehaviorTree;
class UBehaviorTreeComponent;
class UBlackboardComponent;

/** Waits for a tag-query state on the current Blackboard-selected Actor. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_WaitTargetGameplayTagQuery : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_WaitTargetGameplayTagQuery(const FObjectInitializer& ObjectInitializer);

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

	UPROPERTY(EditAnywhere, Category = "Gameplay Tags")
	FGameplayTagQuery GameplayTagQuery;

	UPROPERTY(EditAnywhere, Category = "Gameplay Tags")
	EBertaGameplayTagQueryWaitCondition WaitCondition = EBertaGameplayTagQueryWaitCondition::Matches;

private:
	struct FRegisteredGameplayTagEvent
	{
		FGameplayTag Tag;
		FDelegateHandle Handle;
	};

	UAbilitySystemComponent* ResolveTargetAbilitySystemComponent(const UBlackboardComponent& Blackboard) const;
	bool IsWaitConditionSatisfied(bool bQueryMatches) const;
	bool IsCurrentTargetSatisfied(const UBlackboardComponent& Blackboard) const;
	void BindTargetAbilitySystemComponent(UBlackboardComponent& Blackboard);
	void UnbindTargetGameplayTagEvents();
	void UnregisterObservers();
	void CompleteIfSatisfied();
	void HandleGameplayTagChanged(FGameplayTag CallbackTag, int32 NewCount);
	EBlackboardNotificationResult HandleTargetActorChanged(
		const UBlackboardComponent& Blackboard,
		FBlackboard::FKey ChangedKeyId);

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	TWeakObjectPtr<UBlackboardComponent> ObservedBlackboardComponent;
	TArray<FRegisteredGameplayTagEvent> RegisteredGameplayTagEvents;
	FDelegateHandle BlackboardObserverHandle;
	bool bIsWaiting = false;
};
