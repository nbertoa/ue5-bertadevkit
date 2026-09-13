#include "AI/BertaBTTask_WaitGameplayEffectApplied.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ActiveGameplayEffectHandle.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectTypes.h"

UBertaBTTask_WaitGameplayEffectApplied::UBertaBTTask_WaitGameplayEffectApplied(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Wait Gameplay Effect Applied");
	bCreateNodeInstance = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBertaBTTask_WaitGameplayEffectApplied::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterGameplayEffectEvent();
	if (!IsQuerySupported())
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
	GameplayEffectAppliedDelegateHandle = AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.AddUObject(
		this,
		&ThisClass::HandleGameplayEffectApplied);
	if (!GameplayEffectAppliedDelegateHandle.IsValid())
	{
		UnregisterGameplayEffectEvent();
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBertaBTTask_WaitGameplayEffectApplied::AbortTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterGameplayEffectEvent();
	return EBTNodeResult::Aborted;
}

void UBertaBTTask_WaitGameplayEffectApplied::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	const EBTNodeResult::Type TaskResult)
{
	UnregisterGameplayEffectEvent();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBertaBTTask_WaitGameplayEffectApplied::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterGameplayEffectEvent();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTTask_WaitGameplayEffectApplied::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nGameplay Effect Query: %s"),
		*Super::GetStaticDescription(),
		GameplayEffectQuery.IsEmpty() ? TEXT("None") : TEXT("Configured"));
}

UAbilitySystemComponent* UBertaBTTask_WaitGameplayEffectApplied::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

bool UBertaBTTask_WaitGameplayEffectApplied::IsQuerySupported() const
{
	return !GameplayEffectQuery.IsEmpty() && !GameplayEffectQuery.CustomMatchDelegate.IsBound() &&
		!GameplayEffectQuery.CustomMatchDelegate_BP.IsBound();
}

void UBertaBTTask_WaitGameplayEffectApplied::HandleGameplayEffectApplied(
	UAbilitySystemComponent* Target,
	const FGameplayEffectSpec& AppliedSpec,
	const FActiveGameplayEffectHandle ActiveHandle)
{
	if (!bIsWaiting || !GameplayEffectQuery.Matches(AppliedSpec))
	{
		return;
	}

	UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get();
	UnregisterGameplayEffectEvent();
	if (BehaviorTreeComponent)
	{
		FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
	}
}

void UBertaBTTask_WaitGameplayEffectApplied::UnregisterGameplayEffectEvent()
{
	bIsWaiting = false;
	if (UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
		AbilitySystemComponent && GameplayEffectAppliedDelegateHandle.IsValid())
	{
		AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.Remove(GameplayEffectAppliedDelegateHandle);
	}

	GameplayEffectAppliedDelegateHandle.Reset();
	ObservedAbilitySystemComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
}
