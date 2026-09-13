#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"

#include "BertaBTTask_WaitGameplayEvent.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;
struct FGameplayEventData;

/** Waits for the next matching Gameplay Event received by the controlled Pawn's ASC. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_WaitGameplayEvent : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_WaitGameplayEvent(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Event")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category = "Gameplay Event")
	bool bOnlyMatchExact = false;

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	void HandleExactGameplayEvent(const FGameplayEventData* Payload);
	void HandleGameplayEvent(FGameplayTag MatchingTag, const FGameplayEventData* Payload);
	void UnregisterGameplayEvent();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	FDelegateHandle GameplayEventDelegateHandle;
	bool bIsWaiting = false;
};
