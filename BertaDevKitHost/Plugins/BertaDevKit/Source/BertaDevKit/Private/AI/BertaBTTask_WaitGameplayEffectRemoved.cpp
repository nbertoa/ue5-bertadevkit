#include "AI/BertaBTTask_WaitGameplayEffectRemoved.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectTypes.h"

UBertaBTTask_WaitGameplayEffectRemoved::UBertaBTTask_WaitGameplayEffectRemoved(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Wait Gameplay Effect Removed");
	bCreateNodeInstance = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBertaBTTask_WaitGameplayEffectRemoved::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterGameplayEffectEvent();
	if (GameplayEffectQuery.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!AbilitySystemComponent)
	{
		return EBTNodeResult::Failed;
	}

	if (AbilitySystemComponent->GetActiveEffects(GameplayEffectQuery).IsEmpty())
	{
		return EBTNodeResult::Succeeded;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
	ObservedBehaviorTreeComponent = &OwnerComp;
	bIsWaiting = true;
	GameplayEffectRemovedDelegateHandle = AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(
		this,
		&ThisClass::HandleGameplayEffectRemoved);
	if (!GameplayEffectRemovedDelegateHandle.IsValid())
	{
		UnregisterGameplayEffectEvent();
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBertaBTTask_WaitGameplayEffectRemoved::AbortTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterGameplayEffectEvent();
	return EBTNodeResult::Aborted;
}

void UBertaBTTask_WaitGameplayEffectRemoved::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	const EBTNodeResult::Type TaskResult)
{
	UnregisterGameplayEffectEvent();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBertaBTTask_WaitGameplayEffectRemoved::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterGameplayEffectEvent();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTTask_WaitGameplayEffectRemoved::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nGameplay Effect Query: %s"),
		*Super::GetStaticDescription(),
		GameplayEffectQuery.IsEmpty() ? TEXT("None") : TEXT("Configured"));
}

UAbilitySystemComponent* UBertaBTTask_WaitGameplayEffectRemoved::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

void UBertaBTTask_WaitGameplayEffectRemoved::HandleGameplayEffectRemoved(
	const FActiveGameplayEffect& RemovedEffect)
{
	if (!bIsWaiting)
	{
		return;
	}

	UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get();
	UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
	if (!BehaviorTreeComponent || !AbilitySystemComponent)
	{
		UnregisterGameplayEffectEvent();
		return;
	}

	if (!AbilitySystemComponent->GetActiveEffects(GameplayEffectQuery).IsEmpty())
	{
		return;
	}

	UnregisterGameplayEffectEvent();
	FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
}

void UBertaBTTask_WaitGameplayEffectRemoved::UnregisterGameplayEffectEvent()
{
	bIsWaiting = false;
	if (UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
		AbilitySystemComponent && GameplayEffectRemovedDelegateHandle.IsValid())
	{
		AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().Remove(GameplayEffectRemovedDelegateHandle);
	}

	GameplayEffectRemovedDelegateHandle.Reset();
	ObservedAbilitySystemComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
}
