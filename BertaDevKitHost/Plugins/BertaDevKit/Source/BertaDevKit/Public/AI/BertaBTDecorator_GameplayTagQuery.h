#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "GameplayTagContainer.h"

#include "BertaBTDecorator_GameplayTagQuery.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;

/**
 * Tests a Gameplay Tag Query against the controlled Pawn's Ability System Component.
 * While relevant, changes to every tag referenced by the query notify the Behavior Tree
 * so its configured Observer Aborts policy can react without Blackboard mirroring.
 */
UCLASS()
class BERTADEVKIT_API UBertaBTDecorator_GameplayTagQuery : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBertaBTDecorator_GameplayTagQuery(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Tags")
	FGameplayTagQuery GameplayTagQuery;

private:
	struct FRegisteredGameplayTagEvent
	{
		FGameplayTag Tag;
		FDelegateHandle Handle;
	};

	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	void HandleGameplayTagChanged(FGameplayTag CallbackTag, int32 NewCount);
	void UnregisterGameplayTagEvents();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	TArray<FRegisteredGameplayTagEvent> RegisteredGameplayTagEvents;
};
