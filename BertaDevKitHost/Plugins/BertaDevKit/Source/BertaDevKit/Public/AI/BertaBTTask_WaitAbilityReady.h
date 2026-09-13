#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpecHandle.h"

#include "BertaBTTask_WaitAbilityReady.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;

/** Polls at a bounded interval until an exact granted ability can activate. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_WaitAbilityReady : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_WaitAbilityReady(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float CheckIntervalSeconds = 0.1f;

private:
	static constexpr float MinimumCheckIntervalSeconds = 0.01f;

	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	bool CanActivateObservedAbility() const;
	float GetValidatedCheckInterval() const;
	void ClearWaitState();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	FGameplayAbilitySpecHandle AbilitySpecHandle;
	bool bIsWaiting = false;
};
