#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"

#include "BertaBTTask_WaitGameplayTagQuery.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;

UENUM(BlueprintType)
enum class EBertaGameplayTagQueryWaitCondition : uint8
{
	Matches,
	DoesNotMatch UMETA(DisplayName = "Does Not Match")
};

/**
 * Waits until a Gameplay Tag Query reaches the requested state on the controlled
 * Pawn's Ability System Component. Referenced tag changes drive evaluation without ticking.
 */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_WaitGameplayTagQuery : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_WaitGameplayTagQuery(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Tags")
	FGameplayTagQuery GameplayTagQuery;

	UPROPERTY(EditAnywhere, Category = "Gameplay Tags")
	EBertaGameplayTagQueryWaitCondition WaitCondition = EBertaGameplayTagQueryWaitCondition::Matches;

private:
	struct FRegisteredGameplayTagEvent
	{
		FGameplayTag Tag;
		FDelegateHandle Handle;
	};

	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	bool IsWaitConditionSatisfied(bool bQueryMatches) const;
	void HandleGameplayTagChanged(FGameplayTag CallbackTag, int32 NewCount);
	void UnregisterGameplayTagEvents();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> ObservedBehaviorTreeComponent;
	TArray<FRegisteredGameplayTagEvent> RegisteredGameplayTagEvents;
	bool bIsWaiting = false;
};
