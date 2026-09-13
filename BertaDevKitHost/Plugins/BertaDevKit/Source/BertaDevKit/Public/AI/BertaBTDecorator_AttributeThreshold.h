#pragma once

#include "AI/BertaGASBehaviorTreeTypes.h"
#include "BehaviorTree/BTDecorator.h"

#include "BertaBTDecorator_AttributeThreshold.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;
struct FOnAttributeChangeData;

/** Reactively compares one controlled-Pawn GAS attribute with a threshold. */
UCLASS()
class BERTADEVKIT_API UBertaBTDecorator_AttributeThreshold : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBertaBTDecorator_AttributeThreshold(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Attribute")
	FBertaGameplayAttributeCondition Condition;

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	void HandleAttributeChanged(const FOnAttributeChangeData& ChangeData);
	void UnregisterAttributeEvent();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	FGameplayAttribute ObservedAttribute;
	FDelegateHandle AttributeChangedDelegateHandle;
};
