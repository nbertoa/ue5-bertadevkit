#pragma once

#include "AI/BertaGASBehaviorTreeTypes.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BehaviorTreeTypes.h"

#include "BertaBTDecorator_TargetAttributeThreshold.generated.h"

class UAbilitySystemComponent;
class UBehaviorTree;
class UBehaviorTreeComponent;
class UBlackboardComponent;
struct FOnAttributeChangeData;

/** Reactively compares an attribute on a Blackboard-selected Actor's ASC. */
UCLASS()
class BERTADEVKIT_API UBertaBTDecorator_TargetAttributeThreshold : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBertaBTDecorator_TargetAttributeThreshold(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Gameplay Attribute")
	FBertaGameplayAttributeCondition Condition;

private:
	UAbilitySystemComponent* ResolveTargetAbilitySystemComponent(const UBlackboardComponent& Blackboard) const;
	bool IsConditionSatisfied(const UAbilitySystemComponent& AbilitySystemComponent) const;
	void BindTargetAbilitySystemComponent(UBlackboardComponent& Blackboard);
	void UnbindTargetAttributeEvent();
	void UnregisterObservers();
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
};
