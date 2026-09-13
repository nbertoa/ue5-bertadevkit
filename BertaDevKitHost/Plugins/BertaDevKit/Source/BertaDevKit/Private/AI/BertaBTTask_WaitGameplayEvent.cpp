#include "AI/BertaBTTask_WaitGameplayEvent.h"

#include "AIController.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"

UBertaBTTask_WaitGameplayEvent::UBertaBTTask_WaitGameplayEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Wait Gameplay Event");
	bCreateNodeInstance = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBertaBTTask_WaitGameplayEvent::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterGameplayEvent();

	if (!EventTag.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!AbilitySystemComponent)
	{
		return EBTNodeResult::Failed;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
	ObservedBehaviorTreeComponent = &OwnerComp;
	bIsWaiting = true;

	if (bOnlyMatchExact)
	{
		GameplayEventDelegateHandle = AbilitySystemComponent->GenericGameplayEventCallbacks
			.FindOrAdd(EventTag)
			.AddUObject(this, &ThisClass::HandleExactGameplayEvent);
	}
	else
	{
		GameplayEventDelegateHandle = AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
			FGameplayTagContainer(EventTag),
			FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::HandleGameplayEvent));
	}

	if (!GameplayEventDelegateHandle.IsValid())
	{
		UnregisterGameplayEvent();
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBertaBTTask_WaitGameplayEvent::AbortTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterGameplayEvent();
	return EBTNodeResult::Aborted;
}

void UBertaBTTask_WaitGameplayEvent::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	const EBTNodeResult::Type TaskResult)
{
	UnregisterGameplayEvent();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBertaBTTask_WaitGameplayEvent::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterGameplayEvent();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTTask_WaitGameplayEvent::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nEvent: %s\nMatch: %s"),
		*Super::GetStaticDescription(),
		*EventTag.ToString(),
		bOnlyMatchExact ? TEXT("Exact") : TEXT("Hierarchical"));
}

UAbilitySystemComponent* UBertaBTTask_WaitGameplayEvent::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

void UBertaBTTask_WaitGameplayEvent::HandleExactGameplayEvent(const FGameplayEventData* Payload)
{
	HandleGameplayEvent(EventTag, Payload);
}

void UBertaBTTask_WaitGameplayEvent::HandleGameplayEvent(
	const FGameplayTag MatchingTag,
	const FGameplayEventData* Payload)
{
	if (!bIsWaiting)
	{
		return;
	}

	UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get();
	UnregisterGameplayEvent();
	if (BehaviorTreeComponent)
	{
		FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
	}
}

void UBertaBTTask_WaitGameplayEvent::UnregisterGameplayEvent()
{
	bIsWaiting = false;

	if (UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
		AbilitySystemComponent && GameplayEventDelegateHandle.IsValid() && EventTag.IsValid())
	{
		if (bOnlyMatchExact)
		{
			if (FGameplayEventMulticastDelegate* Delegate =
				AbilitySystemComponent->GenericGameplayEventCallbacks.Find(EventTag))
			{
				Delegate->Remove(GameplayEventDelegateHandle);
			}
		}
		else
		{
			AbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(
				FGameplayTagContainer(EventTag),
				GameplayEventDelegateHandle);
		}
	}

	GameplayEventDelegateHandle.Reset();
	ObservedAbilitySystemComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
}
