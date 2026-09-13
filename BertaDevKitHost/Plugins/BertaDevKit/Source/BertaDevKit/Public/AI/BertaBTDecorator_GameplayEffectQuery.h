#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "GameplayEffect.h"

#include "BertaBTDecorator_GameplayEffectQuery.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;
struct FActiveGameplayEffect;
struct FActiveGameplayEffectHandle;
struct FGameplayEffectSpec;

/** Reactively tests whether at least one active Gameplay Effect matches a query. */
UCLASS()
class BERTADEVKIT_API UBertaBTDecorator_GameplayEffectQuery : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBertaBTDecorator_GameplayEffectQuery(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Effect")
	FGameplayEffectQuery GameplayEffectQuery;

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	void HandleGameplayEffectAdded(
		UAbilitySystemComponent* Target,
		const FGameplayEffectSpec& AppliedSpec,
		FActiveGameplayEffectHandle ActiveHandle);
	void HandleGameplayEffectRemoved(const FActiveGameplayEffect& RemovedEffect);
	void RequestConditionReevaluation();
	void UnregisterGameplayEffectEvents();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	FDelegateHandle GameplayEffectAddedDelegateHandle;
	FDelegateHandle GameplayEffectRemovedDelegateHandle;
};
