#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "GameplayEffect.h"

#include "BertaBTTask_WaitGameplayEffectRemoved.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;
struct FActiveGameplayEffect;

/** Waits until no active Gameplay Effect matching a query remains on the controlled Pawn. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_WaitGameplayEffectRemoved : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_WaitGameplayEffectRemoved(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Effect")
	FGameplayEffectQuery GameplayEffectQuery;

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	void HandleGameplayEffectRemoved(const FActiveGameplayEffect& RemovedEffect);
	void UnregisterGameplayEffectEvent();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	FDelegateHandle GameplayEffectRemovedDelegateHandle;
	bool bIsWaiting = false;
};
