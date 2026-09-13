#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"

#include "BertaBTDecorator_TargetGameplayTagQuery.generated.h"

class UAbilitySystemComponent;
class UBehaviorTree;
class UBehaviorTreeComponent;
class UBlackboardComponent;

/** Reactively evaluates a Gameplay Tag Query on an Actor selected from Blackboard. */
UCLASS()
class BERTADEVKIT_API UBertaBTDecorator_TargetGameplayTagQuery : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBertaBTDecorator_TargetGameplayTagQuery(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Gameplay Tags")
	FGameplayTagQuery GameplayTagQuery;

private:
	struct FRegisteredGameplayTagEvent
	{
		FGameplayTag Tag;
		FDelegateHandle Handle;
	};

	UAbilitySystemComponent* ResolveTargetAbilitySystemComponent(const UBlackboardComponent& Blackboard) const;
	void BindTargetAbilitySystemComponent(UBlackboardComponent& Blackboard);
	void UnbindTargetGameplayTagEvents();
	void UnregisterObservers();
	void HandleGameplayTagChanged(FGameplayTag CallbackTag, int32 NewCount);
	EBlackboardNotificationResult HandleTargetActorChanged(
		const UBlackboardComponent& Blackboard,
		FBlackboard::FKey ChangedKeyId);

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	TWeakObjectPtr<UBlackboardComponent> ObservedBlackboardComponent;
	TArray<FRegisteredGameplayTagEvent> RegisteredGameplayTagEvents;
	FDelegateHandle BlackboardObserverHandle;
};
