#include "Components/BertaGSCAbilityQueueInputBridgeComponent.h"

#include "BertaGASCompanionExt.h"
#include "Abilities/GameplayAbility.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/GSCCoreComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UBertaGSCAbilityQueueInputBridgeComponent::UBertaGSCAbilityQueueInputBridgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBertaGSCAbilityQueueInputBridgeComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bStartAutomatically && !StartBridge())
	{
		EmitDiagnostic(TEXT("Bridge inactive: owner requires both UGSCCoreComponent and UGSCAbilityQueueComponent."));
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
	if (BoundCoreComponent && AbilityQueueComponent)
	{
		return true;
	}

	AActor* Owner = GetOwner();
	UGSCCoreComponent* CoreComponent = Owner ? Owner->FindComponentByClass<UGSCCoreComponent>() : nullptr;
	UGSCAbilityQueueComponent* QueueComponent = Owner ? Owner->FindComponentByClass<UGSCAbilityQueueComponent>() : nullptr;
	if (!CoreComponent || !QueueComponent)
	{
		return false;
	}

	CoreComponent->OnAbilityFailed.AddUniqueDynamic(this, &ThisClass::HandleAbilityFailed);
	CoreComponent->OnAbilityEnded.AddUniqueDynamic(this, &ThisClass::HandleAbilityEnded);
	BoundCoreComponent = CoreComponent;
	AbilityQueueComponent = QueueComponent;
	EmitDiagnostic(TEXT("Bridge active: observing public GSC ability failure and end events."));
	return true;
}

void UBertaGSCAbilityQueueInputBridgeComponent::StopBridge()
{
	if (BoundCoreComponent)
	{
		BoundCoreComponent->OnAbilityFailed.RemoveDynamic(this, &ThisClass::HandleAbilityFailed);
		BoundCoreComponent->OnAbilityEnded.RemoveDynamic(this, &ThisClass::HandleAbilityEnded);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReconciliationTimerHandle);
	}
	ReconciliationTimerHandle.Invalidate();
	PendingFailures.Reset();
	PendingEndedAbility.Reset();
	PendingEndedAbilityClass = nullptr;
	BoundCoreComponent = nullptr;
	AbilityQueueComponent = nullptr;
}

bool UBertaGSCAbilityQueueInputBridgeComponent::IsBridgeActive() const
{
	return BoundCoreComponent != nullptr && AbilityQueueComponent != nullptr;
}

void UBertaGSCAbilityQueueInputBridgeComponent::HandleAbilityFailed(
	const UGameplayAbility* Ability,
	const FGameplayTagContainer& ReasonTags)
{
	if (!Ability)
	{
		EmitDiagnostic(TEXT("Observed ability failure without an ability; ignored."));
		return;
	}

	const UGameplayAbility* AlreadyQueued = AbilityQueueComponent ? AbilityQueueComponent->GetCurrentQueuedAbility() : nullptr;
	UClass* AbilityClass = Ability->GetClass();
	if (FPendingFailure* Existing = PendingFailures.FindByPredicate(
		[AbilityClass](const FPendingFailure& Pending)
		{
			return Pending.AbilityClass.Get() == AbilityClass;
		}))
	{
		Existing->ReasonTags.AppendTags(ReasonTags);
		Existing->bObservedInPublicQueue |= AlreadyQueued && AlreadyQueued->GetClass() == AbilityClass;
		EmitDiagnostic(FString::Printf(TEXT("Coalesced duplicate failure signal for %s."), *AbilityClass->GetPathName()));
		ScheduleReconciliation();
		return;
	}

	FPendingFailure& Pending = PendingFailures.AddDefaulted_GetRef();
	Pending.Ability = const_cast<UGameplayAbility*>(Ability);
	Pending.AbilityClass = AbilityClass;
	Pending.ReasonTags = ReasonTags;
	Pending.bObservedInPublicQueue = AlreadyQueued && AlreadyQueued->GetClass() == AbilityClass;
	EmitDiagnostic(FString::Printf(TEXT("Observed ability failure: %s."), *AbilityClass->GetPathName()));
	ScheduleReconciliation();
}

void UBertaGSCAbilityQueueInputBridgeComponent::HandleAbilityEnded(const UGameplayAbility* Ability)
{
	PendingEndedAbility = const_cast<UGameplayAbility*>(Ability);
	PendingEndedAbilityClass = Ability ? Ability->GetClass() : nullptr;
	const UGameplayAbility* QueuedAtEnd = AbilityQueueComponent ? AbilityQueueComponent->GetCurrentQueuedAbility() : nullptr;
	if (QueuedAtEnd)
	{
		for (FPendingFailure& Pending : PendingFailures)
		{
			Pending.bObservedInPublicQueue |= Pending.AbilityClass.Get() == QueuedAtEnd->GetClass();
		}
	}
	else if (!PendingFailures.IsEmpty())
	{
		PendingFailures.Reset();
		EmitDiagnostic(TEXT("Failure/end completed before reconciliation with no queued ability; skipped because public state cannot distinguish native consumption from an unhandled failure."));
	}
	EmitDiagnostic(FString::Printf(TEXT("Observed ability end: %s."), *GetPathNameSafe(Ability ? Ability->GetClass() : nullptr)));
	ScheduleReconciliation();
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
}

void UBertaGSCAbilityQueueInputBridgeComponent::ReconcilePublicQueueState()
{
	ReconciliationTimerHandle.Invalidate();
	if (!AbilityQueueComponent || !AbilityQueueComponent->bAbilityQueueEnabled || !AbilityQueueComponent->IsAbilityQueueOpened())
	{
		PendingFailures.Reset();
		PendingEndedAbility.Reset();
		PendingEndedAbilityClass = nullptr;
		EmitDiagnostic(TEXT("Reconciliation skipped: queue is missing, disabled, or closed."));
		return;
	}

	for (const FPendingFailure& Pending : PendingFailures)
	{
		if (Pending.bObservedInPublicQueue)
		{
			EmitDiagnostic(FString::Printf(TEXT("Native GSC handling exposed queued state for %s; no forwarding."), *GetPathNameSafe(Pending.AbilityClass)));
			continue;
		}

		UGameplayAbility* Ability = Pending.Ability.Get();
		if (!Ability && Pending.AbilityClass)
		{
			Ability = Pending.AbilityClass->GetDefaultObject<UGameplayAbility>();
		}
		if (!Ability || !IsAbilityAllowed(*Ability))
		{
			continue;
		}

		const UGameplayAbility* AlreadyQueued = AbilityQueueComponent->GetCurrentQueuedAbility();
		if (AlreadyQueued && AlreadyQueued->GetClass() == Ability->GetClass())
		{
			EmitDiagnostic(FString::Printf(TEXT("Native GSC handling already queued %s; no forwarding."), *Ability->GetClass()->GetPathName()));
			continue;
		}

		AbilityQueueComponent->OnAbilityFailed(Ability, Pending.ReasonTags);
		EmitDiagnostic(FString::Printf(TEXT("Forwarded unhandled failure for %s exactly once."), *Ability->GetClass()->GetPathName()));
	}
	PendingFailures.Reset();

	UGameplayAbility* EndedAbility = PendingEndedAbility.Get();
	if (!EndedAbility && PendingEndedAbilityClass)
	{
		EndedAbility = PendingEndedAbilityClass->GetDefaultObject<UGameplayAbility>();
	}
	if (EndedAbility && AbilityQueueComponent->GetCurrentQueuedAbility())
	{
		AbilityQueueComponent->OnAbilityEnded(EndedAbility);
		EmitDiagnostic(TEXT("Forwarded unhandled ability end for the remaining queued ability exactly once."));
	}
	PendingEndedAbility.Reset();
	PendingEndedAbilityClass = nullptr;
}

void UBertaGSCAbilityQueueInputBridgeComponent::EmitDiagnostic(const FString& Message)
{
	if (bLogDiagnostics)
	{
		UE_LOG(LogBertaGASCompanionExt, Log, TEXT("[AbilityQueueInputBridge] %s"), *Message);
	}
	OnDiagnostic.Broadcast(Message);
}

bool UBertaGSCAbilityQueueInputBridgeComponent::IsAbilityAllowed(const UGameplayAbility& Ability) const
{
	if (!AbilityQueueComponent)
	{
		return false;
	}
	if (AbilityQueueComponent->IsAllAbilitiesAllowedForAbilityQueue())
	{
		return true;
	}

	const UClass* AbilityClass = Ability.GetClass();
	return AbilityQueueComponent->GetQueuedAllowedAbilities().ContainsByPredicate(
		[AbilityClass](const TSubclassOf<UGameplayAbility> AllowedClass)
		{
			return AllowedClass.Get() == AbilityClass;
		});
}
