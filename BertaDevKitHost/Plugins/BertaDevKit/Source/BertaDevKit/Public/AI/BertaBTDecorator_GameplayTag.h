#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "GameplayTagContainer.h"

#include "BertaBTDecorator_GameplayTag.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;

/**
 * Tests a single Gameplay Tag on the controlled Pawn's Ability System Component.
 * While relevant, tag presence changes notify the Behavior Tree so its configured
 * Observer Aborts policy can react without mirroring GAS state into a Blackboard.
 */
UCLASS()
class BERTADEVKIT_API UBertaBTDecorator_GameplayTag : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBertaBTDecorator_GameplayTag(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Tag")
	FGameplayTag GameplayTag;

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	void HandleGameplayTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void UnregisterGameplayTagEvent();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	FGameplayTag RegisteredGameplayTag;
	FDelegateHandle GameplayTagEventHandle;
};
