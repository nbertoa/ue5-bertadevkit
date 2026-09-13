#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "GameplayEffect.h"

#include "BertaBTTask_WaitGameplayEffectApplied.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;
struct FActiveGameplayEffectHandle;
struct FGameplayEffectSpec;

/** Waits for the next matching Gameplay Effect applied to the controlled Pawn. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_WaitGameplayEffectApplied : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_WaitGameplayEffectApplied(const FObjectInitializer& ObjectInitializer);

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
	bool IsQuerySupported() const;
	void HandleGameplayEffectApplied(
		UAbilitySystemComponent* Target,
		const FGameplayEffectSpec& AppliedSpec,
		FActiveGameplayEffectHandle ActiveHandle);
	void UnregisterGameplayEffectEvent();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	FDelegateHandle GameplayEffectAppliedDelegateHandle;
	bool bIsWaiting = false;
};
