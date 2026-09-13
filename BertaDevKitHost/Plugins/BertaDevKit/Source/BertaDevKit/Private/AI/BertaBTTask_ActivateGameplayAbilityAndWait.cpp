#include "AI/BertaBTTask_ActivateGameplayAbilityAndWait.h"

#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpec.h"

UBertaBTTask_ActivateGameplayAbilityAndWait::UBertaBTTask_ActivateGameplayAbilityAndWait(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Activate Gameplay Ability And Wait");
	bCreateNodeInstance = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBertaBTTask_ActivateGameplayAbilityAndWait::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterDelegates();

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
	if (!AbilitySpec)
	{
		return EBTNodeResult::Failed;
	}

	AbilitySpecHandle = AbilitySpec->Handle;
	ObservedAbilitySystemComponent = AbilitySystemComponent;
	ObservedBehaviorTreeComponent = &OwnerComp;
	bIsWaiting = true;
	bIsExecutingTask = true;
	PendingExecutionResult = EBTNodeResult::InProgress;

	AbilityActivatedDelegateHandle = AbilitySystemComponent->AbilityActivatedCallbacks.AddUObject(
		this,
		&ThisClass::HandleAbilityActivated);
	AbilityFailedDelegateHandle = AbilitySystemComponent->AbilityFailedCallbacks.AddUObject(
		this,
		&ThisClass::HandleAbilityFailed);

	const bool bActivationRequested = AbilitySystemComponent->TryActivateAbility(
		AbilitySpecHandle,
		bAllowRemoteActivation);
	bIsExecutingTask = false;

	if (!bActivationRequested)
	{
		UnregisterDelegates();
		return EBTNodeResult::Failed;
	}

	if (PendingExecutionResult != EBTNodeResult::InProgress)
	{
		const EBTNodeResult::Type Result = PendingExecutionResult;
		UnregisterDelegates();
		return Result;
	}

	// TryActivateAbility may report that a remote request was sent without establishing
	// a locally observable execution. A latent task cannot truthfully wait in that state.
	if (!ActivatedAbility.IsValid())
	{
		UnregisterDelegates();
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBertaBTTask_ActivateGameplayAbilityAndWait::AbortTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UGameplayAbility* AbilityToCancel = ActivatedAbility.Get();
	const FGameplayAbilitySpecHandle HandleToCancel = AbilitySpecHandle;
	UnregisterDelegates();

	// Instanced abilities identify this task's exact execution. Spec-level cancellation
	// would also cancel unrelated InstancedPerExecution activations.
	if (bCancelAbilityOnAbort && AbilityToCancel && AbilityToCancel->IsInstantiated() &&
		AbilityToCancel->CanBeCanceled() && AbilityToCancel->GetCurrentAbilitySpecHandle() == HandleToCancel)
	{
		AbilityToCancel->CancelAbility(
			HandleToCancel,
			AbilityToCancel->GetCurrentActorInfo(),
			AbilityToCancel->GetCurrentActivationInfo(),
			true);
	}

	return EBTNodeResult::Aborted;
}

void UBertaBTTask_ActivateGameplayAbilityAndWait::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	const EBTNodeResult::Type TaskResult)
{
	UnregisterDelegates();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBertaBTTask_ActivateGameplayAbilityAndWait::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterDelegates();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTTask_ActivateGameplayAbilityAndWait::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nAbility: %s\nCancel On Abort: %s"),
		*Super::GetStaticDescription(),
		*GetNameSafe(AbilityClass),
		bCancelAbilityOnAbort ? TEXT("true") : TEXT("false"));
}

UAbilitySystemComponent* UBertaBTTask_ActivateGameplayAbilityAndWait::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

void UBertaBTTask_ActivateGameplayAbilityAndWait::HandleAbilityActivated(UGameplayAbility* Ability)
{
	if (!bIsWaiting || !Ability || ActivatedAbility.IsValid() ||
		Ability->GetCurrentAbilitySpecHandle() != AbilitySpecHandle)
	{
		return;
	}

	ActivatedAbility = Ability;
	AbilityEndedDelegateHandle = Ability->OnGameplayAbilityEndedWithData.AddUObject(
		this,
		&ThisClass::HandleAbilityEnded);
}

void UBertaBTTask_ActivateGameplayAbilityAndWait::HandleAbilityFailed(
	const UGameplayAbility* Ability,
	const FGameplayTagContainer& FailureTags)
{
	if (bIsWaiting && Ability && Ability->GetClass() == AbilityClass)
	{
		CompleteTask(EBTNodeResult::Failed);
	}
}

void UBertaBTTask_ActivateGameplayAbilityAndWait::HandleAbilityEnded(const FAbilityEndedData& EndedData)
{
	if (!bIsWaiting || EndedData.AbilitySpecHandle != AbilitySpecHandle ||
		EndedData.AbilityThatEnded != ActivatedAbility.Get())
	{
		return;
	}

	CompleteTask(EndedData.bWasCancelled ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
}

void UBertaBTTask_ActivateGameplayAbilityAndWait::CompleteTask(const EBTNodeResult::Type Result)
{
	if (!bIsWaiting)
	{
		return;
	}

	if (bIsExecutingTask)
	{
		PendingExecutionResult = Result;
		bIsWaiting = false;
		return;
	}

	UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get();
	UnregisterDelegates();
	if (BehaviorTreeComponent)
	{
		FinishLatentTask(*BehaviorTreeComponent, Result);
	}
}

void UBertaBTTask_ActivateGameplayAbilityAndWait::UnregisterDelegates()
{
	bIsWaiting = false;
	bIsExecutingTask = false;

	if (UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get())
	{
		if (AbilityActivatedDelegateHandle.IsValid())
		{
			AbilitySystemComponent->AbilityActivatedCallbacks.Remove(AbilityActivatedDelegateHandle);
		}
		if (AbilityFailedDelegateHandle.IsValid())
		{
			AbilitySystemComponent->AbilityFailedCallbacks.Remove(AbilityFailedDelegateHandle);
		}
	}

	if (UGameplayAbility* Ability = ActivatedAbility.Get(); Ability && AbilityEndedDelegateHandle.IsValid())
	{
		Ability->OnGameplayAbilityEndedWithData.Remove(AbilityEndedDelegateHandle);
	}

	AbilityActivatedDelegateHandle.Reset();
	AbilityFailedDelegateHandle.Reset();
	AbilityEndedDelegateHandle.Reset();
	AbilitySpecHandle = FGameplayAbilitySpecHandle();
	ActivatedAbility.Reset();
	ObservedAbilitySystemComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
	PendingExecutionResult = EBTNodeResult::InProgress;
}
