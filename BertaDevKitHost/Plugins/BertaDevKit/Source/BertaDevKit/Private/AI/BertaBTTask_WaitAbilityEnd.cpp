#include "AI/BertaBTTask_WaitAbilityEnd.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpec.h"

UBertaBTTask_WaitAbilityEnd::UBertaBTTask_WaitAbilityEnd(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Wait Ability End");
	bCreateNodeInstance = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBertaBTTask_WaitAbilityEnd::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterAbilityEvent();

	if (!AbilityClass)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!AbilitySystemComponent)
	{
		return EBTNodeResult::Failed;
	}

	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass);
	if (!AbilitySpec || !AbilitySpec->IsActive())
	{
		return EBTNodeResult::Succeeded;
	}

	AbilitySpecHandle = AbilitySpec->Handle;
	ObservedAbilitySystemComponent = AbilitySystemComponent;
	ObservedBehaviorTreeComponent = &OwnerComp;
	bIsWaiting = true;
	AbilityEndedDelegateHandle = AbilitySystemComponent->OnAbilityEnded.AddUObject(
		this,
		&ThisClass::HandleAbilityEnded);

	if (!AbilityEndedDelegateHandle.IsValid())
	{
		UnregisterAbilityEvent();
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBertaBTTask_WaitAbilityEnd::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnregisterAbilityEvent();
	return EBTNodeResult::Aborted;
}

void UBertaBTTask_WaitAbilityEnd::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	const EBTNodeResult::Type TaskResult)
{
	UnregisterAbilityEvent();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBertaBTTask_WaitAbilityEnd::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterAbilityEvent();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTTask_WaitAbilityEnd::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\nAbility: %s"), *Super::GetStaticDescription(), *GetNameSafe(AbilityClass));
}

UAbilitySystemComponent* UBertaBTTask_WaitAbilityEnd::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

void UBertaBTTask_WaitAbilityEnd::HandleAbilityEnded(const FAbilityEndedData& EndedData)
{
	if (!bIsWaiting || EndedData.AbilitySpecHandle != AbilitySpecHandle)
	{
		return;
	}

	UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get();
	UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
	if (!BehaviorTreeComponent || !AbilitySystemComponent)
	{
		UnregisterAbilityEvent();
		return;
	}

	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle);
	if (AbilitySpec && AbilitySpec->IsActive())
	{
		return;
	}

	UnregisterAbilityEvent();
	FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
}

void UBertaBTTask_WaitAbilityEnd::UnregisterAbilityEvent()
{
	bIsWaiting = false;
	if (UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
		AbilitySystemComponent && AbilityEndedDelegateHandle.IsValid())
	{
		AbilitySystemComponent->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
	}

	AbilityEndedDelegateHandle.Reset();
	AbilitySpecHandle = FGameplayAbilitySpecHandle();
	ObservedAbilitySystemComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
}
