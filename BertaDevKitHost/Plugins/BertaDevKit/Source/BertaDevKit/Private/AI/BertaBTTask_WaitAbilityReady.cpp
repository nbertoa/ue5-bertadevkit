#include "AI/BertaBTTask_WaitAbilityReady.h"

#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpec.h"

UBertaBTTask_WaitAbilityReady::UBertaBTTask_WaitAbilityReady(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Wait Ability Ready");
	bCreateNodeInstance = true;
	bTickIntervals = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBertaBTTask_WaitAbilityReady::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	ClearWaitState();
	if (!AbilityClass)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent
		? AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass)
		: nullptr;
	if (!AbilitySpec)
	{
		return EBTNodeResult::Failed;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
	AbilitySpecHandle = AbilitySpec->Handle;
	if (CanActivateObservedAbility())
	{
		ClearWaitState();
		return EBTNodeResult::Succeeded;
	}

	bIsWaiting = true;
	SetNextTickTime(NodeMemory, GetValidatedCheckInterval());
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBertaBTTask_WaitAbilityReady::AbortTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	ClearWaitState();
	return EBTNodeResult::Aborted;
}

void UBertaBTTask_WaitAbilityReady::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	if (!bIsWaiting)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
	if (!AbilitySystemComponent || !AbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle))
	{
		ClearWaitState();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (CanActivateObservedAbility())
	{
		ClearWaitState();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	SetNextTickTime(NodeMemory, GetValidatedCheckInterval());
}

void UBertaBTTask_WaitAbilityReady::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	const EBTNodeResult::Type TaskResult)
{
	ClearWaitState();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBertaBTTask_WaitAbilityReady::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	ClearWaitState();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTTask_WaitAbilityReady::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nAbility: %s\nCheck Interval: %.2fs"),
		*Super::GetStaticDescription(),
		*GetNameSafe(AbilityClass),
		GetValidatedCheckInterval());
}

UAbilitySystemComponent* UBertaBTTask_WaitAbilityReady::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

bool UBertaBTTask_WaitAbilityReady::CanActivateObservedAbility() const
{
	const UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent
		? AbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle)
		: nullptr;
	const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent
		? AbilitySystemComponent->AbilityActorInfo.Get()
		: nullptr;
	if (!AbilitySpec || !ActorInfo)
	{
		return false;
	}

	const UGameplayAbility* Ability = AbilitySpec->GetPrimaryInstance();
	if (!Ability)
	{
		Ability = AbilitySpec->Ability.Get();
	}

	return Ability && Ability->CanActivateAbility(AbilitySpecHandle, ActorInfo);
}

float UBertaBTTask_WaitAbilityReady::GetValidatedCheckInterval() const
{
	return FMath::Max(CheckIntervalSeconds, MinimumCheckIntervalSeconds);
}

void UBertaBTTask_WaitAbilityReady::ClearWaitState()
{
	bIsWaiting = false;
	AbilitySpecHandle = FGameplayAbilitySpecHandle();
	ObservedAbilitySystemComponent.Reset();
}
