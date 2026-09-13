#pragma once

#include "Abilities/GameplayAbilityTypes.h"
#include "BehaviorTree/BTDecorator.h"
#include "GameplayAbilitySpecHandle.h"

#include "BertaBTDecorator_AbilityActive.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;
class UGameplayAbility;

/** Reactively tests whether an exact granted ability class has any active execution. */
UCLASS()
class BERTADEVKIT_API UBertaBTDecorator_AbilityActive : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBertaBTDecorator_AbilityActive(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	void HandleAbilityActivated(UGameplayAbility* Ability);
	void HandleAbilityEnded(const FAbilityEndedData& EndedData);
	void RequestConditionReevaluation();
	void UnregisterAbilityEvents();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	FGameplayAbilitySpecHandle AbilitySpecHandle;
	FDelegateHandle AbilityActivatedDelegateHandle;
	FDelegateHandle AbilityEndedDelegateHandle;
	bool bIsHandlingObservedActivation = false;
};
