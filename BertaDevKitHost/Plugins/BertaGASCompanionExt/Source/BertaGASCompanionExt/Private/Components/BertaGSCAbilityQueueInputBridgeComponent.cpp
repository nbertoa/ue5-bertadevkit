#include "Components/BertaGSCAbilityQueueInputBridgeComponent.h"

#include "Components/BertaGSCAbilityQueueInputBridgeInternal.h"

#include "BertaGASCompanionExt.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/GSCAbilityInputBindingComponent.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/GSCCoreComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "InputAction.h"
#include "TimerManager.h"

UBertaGSCAbilityQueueInputBridgeComponent::UBertaGSCAbilityQueueInputBridgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBertaGSCAbilityQueueInputBridgeComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bStartAutomatically)
	{
		StartBridge();
	}
}

void UBertaGSCAbilityQueueInputBridgeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopBridge();
	Super::EndPlay(EndPlayReason);
}

void UBertaGSCAbilityQueueInputBridgeComponent::BeginDestroy()
{
	StopBridge();
	Super::BeginDestroy();
}

bool UBertaGSCAbilityQueueInputBridgeComponent::StartBridge()
{
	if (bBridgeStarted)
	{
		if (!RefreshAbilitySystemBinding())
		{
			EmitDiagnostic(TEXT("Bridge waiting: GAS Companion has not initialized an ASC for this owner yet."));
		}
		return true;
	}

	AActor* Owner = GetOwner();
	UGSCCoreComponent* NewCoreComponent = Owner ? Owner->FindComponentByClass<UGSCCoreComponent>() : nullptr;
	UGSCAbilityInputBindingComponent* NewInputBindingComponent = Owner
		? Owner->FindComponentByClass<UGSCAbilityInputBindingComponent>()
		: nullptr;
	UGSCAbilityQueueComponent* NewQueueComponent = Owner ? Owner->FindComponentByClass<UGSCAbilityQueueComponent>() : nullptr;
	if (!NewCoreComponent || !NewInputBindingComponent || !NewQueueComponent)
	{
		EmitDiagnostic(TEXT("Bridge could not start: owner requires UGSCAbilityInputBindingComponent, UGSCAbilityQueueComponent, and UGSCCoreComponent."));
		return false;
	}

	CoreComponent = NewCoreComponent;
	AbilityInputBindingComponent = NewInputBindingComponent;
	AbilityQueueComponent = NewQueueComponent;
	CoreComponent->OnInitAbilityActorInfo.AddUniqueDynamic(this, &ThisClass::HandleAbilityActorInfoInitialized);
	bBridgeStarted = true;
	if (!RefreshAbilitySystemBinding())
	{
		EmitDiagnostic(TEXT("Bridge waiting: GAS Companion has not initialized an ASC for this owner yet."));
	}
	return true;
}

void UBertaGSCAbilityQueueInputBridgeComponent::StopBridge()
{
	if (CoreComponent)
	{
		CoreComponent->OnInitAbilityActorInfo.RemoveDynamic(this, &ThisClass::HandleAbilityActorInfoInitialized);
	}
	UnbindAbilitySystemComponent();
	CoreComponent = nullptr;
	AbilityInputBindingComponent = nullptr;
	AbilityQueueComponent = nullptr;
	bBridgeStarted = false;
}

bool UBertaGSCAbilityQueueInputBridgeComponent::IsBridgeActive() const
{
	return bBridgeStarted
		&& BoundAbilitySystemComponent.IsValid()
		&& CoreComponent != nullptr
		&& AbilityInputBindingComponent != nullptr
		&& AbilityQueueComponent != nullptr;
}

bool UBertaGSCAbilityQueueInputBridgeComponent::ReportInputDrivenFailure(
	UInputAction* SourceInputAction,
	const TSubclassOf<UGameplayAbility> AbilityClass,
	const FGameplayTagContainer& ReasonTags)
{
	using namespace BertaGSCAbilityQueueInputBridgePrivate;

	FInputFailureContext Context;
	Context.bBridgeActive = IsBridgeActive();
	Context.bRequestValid = SourceInputAction && AbilityClass;
	if (Context.bBridgeActive && Context.bRequestValid)
	{
		Context.bQueueEnabledAndOpen = AbilityQueueComponent->bAbilityQueueEnabled
			&& AbilityQueueComponent->IsAbilityQueueOpened();
		Context.bRuntimeBindingMatches = AbilityInputBindingComponent->GetBoundInputActionForAbilityClass(AbilityClass)
			== SourceInputAction;
		Context.bAbilityAllowed = IsAbilityAllowed(AbilityClass);
	}

	switch (EvaluateInputFailure(Context))
	{
	case EInputFailureDecision::BridgeInactive:
		EmitDiagnostic(TEXT("Input failure rejected: bridge is waiting for a valid ASC or is stopped."));
		return false;
	case EInputFailureDecision::InvalidRequest:
		EmitDiagnostic(TEXT("Input failure rejected: SourceInputAction and AbilityClass must both be valid."));
		return false;
	case EInputFailureDecision::QueueUnavailable:
		EmitDiagnostic(TEXT("Input failure rejected: ability queue is disabled or closed."));
		return false;
	case EInputFailureDecision::BindingMismatch:
		EmitDiagnostic(FString::Printf(
			TEXT("Input failure rejected: %s is not the runtime GSC binding for %s."),
			*SourceInputAction->GetPathName(),
			*AbilityClass->GetPathName()));
		return false;
	case EInputFailureDecision::AbilityNotAllowed:
		EmitDiagnostic(FString::Printf(TEXT("Input failure rejected: %s is not allowed by the open queue."), *AbilityClass->GetPathName()));
		return false;
	case EInputFailureDecision::Accept:
		break;
	default:
		checkNoEntry();
		return false;
	}

	const UGameplayAbility* AlreadyQueued = AbilityQueueComponent->GetCurrentQueuedAbility();
	if (AlreadyQueued && AlreadyQueued->GetClass() == AbilityClass.Get())
	{
		EmitDiagnostic(FString::Printf(TEXT("Input failure already queued by native GSC handling: %s."), *AbilityClass->GetPathName()));
		return true;
	}

	const UGameplayAbility* Ability = AbilityClass->GetDefaultObject<UGameplayAbility>();
	AbilityQueueComponent->OnAbilityFailed(Ability, ReasonTags);
	const UGameplayAbility* QueuedAbility = AbilityQueueComponent->GetCurrentQueuedAbility();
	const bool bAccepted = QueuedAbility && QueuedAbility->GetClass() == AbilityClass.Get();
	if (bAccepted)
	{
		ExplicitlySubmittedAbilityClass = AbilityClass;
		EmitDiagnostic(FString::Printf(TEXT("Forwarded explicit input failure for %s."), *AbilityClass->GetPathName()));
	}
	else
	{
		EmitDiagnostic(FString::Printf(TEXT("Input failure was not accepted by the public GSC queue for %s."), *AbilityClass->GetPathName()));
	}
	return bAccepted;
}

void UBertaGSCAbilityQueueInputBridgeComponent::HandleAbilityEnded(UGameplayAbility* Ability)
{
	const UGameplayAbility* QueuedAbility = IsBridgeActive() ? AbilityQueueComponent->GetCurrentQueuedAbility() : nullptr;
	if (!ExplicitlySubmittedAbilityClass
		|| !QueuedAbility
		|| QueuedAbility->GetClass() != ExplicitlySubmittedAbilityClass.Get())
	{
		return;
	}

	PendingEndedAbility = Ability;
	PendingEndedAbilityClass = Ability ? Ability->GetClass() : nullptr;
	ScheduleReconciliation();
}

bool UBertaGSCAbilityQueueInputBridgeComponent::RefreshAbilitySystemBinding()
{
	AActor* Owner = GetOwner();
	UAbilitySystemComponent* CurrentAbilitySystemComponent = Owner
		? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner)
		: nullptr;
	if (BoundAbilitySystemComponent.Get() == CurrentAbilitySystemComponent && CurrentAbilitySystemComponent)
	{
		return true;
	}

	UnbindAbilitySystemComponent();
	if (!CurrentAbilitySystemComponent)
	{
		return false;
	}

	AbilityEndedDelegateHandle = CurrentAbilitySystemComponent->AbilityEndedCallbacks.AddUObject(this, &ThisClass::HandleAbilityEnded);
	BoundAbilitySystemComponent = CurrentAbilitySystemComponent;
	EmitDiagnostic(FString::Printf(TEXT("Bridge active: bound to ASC %s."), *CurrentAbilitySystemComponent->GetPathName()));
	return true;
}

void UBertaGSCAbilityQueueInputBridgeComponent::UnbindAbilitySystemComponent()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReconciliationTimerHandle);
	}
	ReconciliationTimerHandle.Invalidate();
	PendingEndedAbility.Reset();
	PendingEndedAbilityClass = nullptr;
	ExplicitlySubmittedAbilityClass = nullptr;

	if (UAbilitySystemComponent* AbilitySystemComponent = BoundAbilitySystemComponent.Get())
	{
		AbilitySystemComponent->AbilityEndedCallbacks.Remove(AbilityEndedDelegateHandle);
	}
	AbilityEndedDelegateHandle.Reset();
	BoundAbilitySystemComponent.Reset();
}

void UBertaGSCAbilityQueueInputBridgeComponent::HandleAbilityActorInfoInitialized()
{
	if (bBridgeStarted && !RefreshAbilitySystemBinding())
	{
		EmitDiagnostic(TEXT("Bridge waiting: Ability Actor Info changed but no ASC is currently available."));
	}
}

void UBertaGSCAbilityQueueInputBridgeComponent::ScheduleReconciliation()
{
	if (ReconciliationTimerHandle.IsValid())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		ReconciliationTimerHandle = World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::ReconcilePublicQueueState);
	}
	else
	{
		PendingEndedAbility.Reset();
		PendingEndedAbilityClass = nullptr;
		ExplicitlySubmittedAbilityClass = nullptr;
	}
}

void UBertaGSCAbilityQueueInputBridgeComponent::ReconcilePublicQueueState()
{
	ReconciliationTimerHandle.Invalidate();
	if (!IsBridgeActive() || !AbilityQueueComponent->bAbilityQueueEnabled)
	{
		PendingEndedAbility.Reset();
		PendingEndedAbilityClass = nullptr;
		ExplicitlySubmittedAbilityClass = nullptr;
		EmitDiagnostic(TEXT("Ability end reconciliation skipped: bridge is inactive or queue is disabled."));
		return;
	}

	UGameplayAbility* EndedAbility = PendingEndedAbility.Get();
	if (!EndedAbility && PendingEndedAbilityClass)
	{
		EndedAbility = PendingEndedAbilityClass->GetDefaultObject<UGameplayAbility>();
	}
	const UGameplayAbility* QueuedAbility = AbilityQueueComponent->GetCurrentQueuedAbility();
	if (EndedAbility
		&& ExplicitlySubmittedAbilityClass
		&& QueuedAbility
		&& QueuedAbility->GetClass() == ExplicitlySubmittedAbilityClass.Get())
	{
		AbilityQueueComponent->OnAbilityEnded(EndedAbility);
		EmitDiagnostic(TEXT("Forwarded ability end because native GSC handling left an explicitly submitted input failure queued."));
	}
	PendingEndedAbility.Reset();
	PendingEndedAbilityClass = nullptr;
	ExplicitlySubmittedAbilityClass = nullptr;
}

void UBertaGSCAbilityQueueInputBridgeComponent::EmitDiagnostic(const FString& Message)
{
	if (bLogDiagnostics)
	{
		UE_LOG(LogBertaGASCompanionExt, Log, TEXT("[AbilityQueueInputBridge] %s"), *Message);
	}
	OnDiagnostic.Broadcast(Message);
}

bool UBertaGSCAbilityQueueInputBridgeComponent::IsAbilityAllowed(const TSubclassOf<UGameplayAbility> AbilityClass) const
{
	if (!AbilityQueueComponent)
	{
		return false;
	}
	if (AbilityQueueComponent->IsAllAbilitiesAllowedForAbilityQueue())
	{
		return true;
	}

	return AbilityQueueComponent->GetQueuedAllowedAbilities().ContainsByPredicate(
		[AbilityClass](const TSubclassOf<UGameplayAbility> AllowedClass)
		{
			return AllowedClass.Get() == AbilityClass.Get();
		});
}
