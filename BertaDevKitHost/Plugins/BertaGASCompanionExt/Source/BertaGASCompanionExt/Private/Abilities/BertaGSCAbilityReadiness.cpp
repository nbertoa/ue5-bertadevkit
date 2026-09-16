#include "Abilities/BertaGSCAbilityReadiness.h"

#include "Abilities/BertaGSCAbilityActivationReportInternal.h"
#include "Abilities/BertaGSCAbilityReadinessInternal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpec.h"
#include "InputAction.h"

namespace
{
	FString StateName(const EBertaGSCAbilityReadinessState State)
	{
		switch (State)
		{
		case EBertaGSCAbilityReadinessState::Ready: return TEXT("Ready");
		case EBertaGSCAbilityReadinessState::Active: return TEXT("Active");
		case EBertaGSCAbilityReadinessState::BlockedByCost: return TEXT("BlockedByCost");
		case EBertaGSCAbilityReadinessState::BlockedByCooldown: return TEXT("BlockedByCooldown");
		case EBertaGSCAbilityReadinessState::BlockedByActivation: return TEXT("BlockedByActivation");
		case EBertaGSCAbilityReadinessState::NotFullyInspectable: return TEXT("NotFullyInspectable");
		default: return TEXT("Unknown");
		}
	}

	FString FormatEntry(const FBertaGSCAbilityReadinessEntry& Entry)
	{
		const FBertaGSCAbilityActivationReport& Inspection = Entry.Inspection;
		const FString QueueEligibility = Entry.bQueueEligibilityKnown
			? BertaGSCAbilityActivation::BoolText(Entry.bAllowedByCurrentQueue)
			: TEXT("Unknown");
		const FString QueuedState = Entry.bCurrentQueuedSpecKnown
			? BertaGSCAbilityActivation::BoolText(Entry.bIsCurrentlyQueued)
			: TEXT("Unknown");
		return FString::Printf(
			TEXT("%s | Handle=%s | Level=%d | States=[%s] | Input=%s | Cost=%s [%s] | Cooldown=%s Remaining=%.3fs Duration=%.3fs | Activation=%s [%s] | QueuePresent=%s Enabled=%s Open=%s Allowed=%s Queued=%s"),
			*Inspection.AbilityClassPath,
			*Entry.AbilitySpecHandle.ToString(),
			Inspection.GrantedLevel,
			*BertaGSCAbilityReadiness::StateText(Entry.States),
			*GetPathNameSafe(Inspection.BoundInputAction),
			Inspection.bCostCheckAvailable ? *BertaGSCAbilityActivation::BoolText(Inspection.Cost.bCanPayCost) : TEXT("Unknown"),
			*BertaGSCAbilityActivation::SortedTags(Inspection.Cost.FailureTags),
			Inspection.bCooldownCheckAvailable ? *BertaGSCAbilityActivation::BoolText(Inspection.Cooldown.bIsOnCooldown) : TEXT("Unknown"),
			Inspection.Cooldown.TimeRemainingSeconds,
			Inspection.Cooldown.DurationSeconds,
			Inspection.bActivationCheckAvailable ? *BertaGSCAbilityActivation::BoolText(Inspection.Activation.bCanActivate) : TEXT("Unknown"),
			*BertaGSCAbilityActivation::SortedTags(Inspection.Activation.FailureTags),
			*BertaGSCAbilityActivation::BoolText(Inspection.bAbilityQueueComponentFound),
			*BertaGSCAbilityActivation::BoolText(Inspection.bAbilityQueueEnabled),
			*BertaGSCAbilityActivation::BoolText(Inspection.bAbilityQueueOpened),
			*QueueEligibility,
			*QueuedState);
	}
}

namespace BertaGSCAbilityReadiness
{
	void Classify(FBertaGSCAbilityReadinessEntry& Entry)
	{
		Entry.States.Reset();
		const FBertaGSCAbilityActivationReport& Inspection = Entry.Inspection;
		const bool bFullyInspectable = Inspection.bCostCheckAvailable
			&& Inspection.bCooldownCheckAvailable
			&& Inspection.bActivationCheckAvailable;

		if (!bFullyInspectable)
		{
			Entry.States.Add(EBertaGSCAbilityReadinessState::NotFullyInspectable);
		}
		if (Inspection.bAbilityActive)
		{
			Entry.States.Add(EBertaGSCAbilityReadinessState::Active);
		}
		if (Inspection.bCostCheckAvailable && !Inspection.Cost.bCanPayCost)
		{
			Entry.States.Add(EBertaGSCAbilityReadinessState::BlockedByCost);
		}
		if (Inspection.bCooldownCheckAvailable && Inspection.Cooldown.bIsOnCooldown)
		{
			Entry.States.Add(EBertaGSCAbilityReadinessState::BlockedByCooldown);
		}
		if (Inspection.bActivationCheckAvailable && !Inspection.Activation.bCanActivate)
		{
			Entry.States.Add(EBertaGSCAbilityReadinessState::BlockedByActivation);
		}

		Entry.bReady = !Inspection.bAbilityActive
			&& bFullyInspectable
			&& Inspection.Cost.bCanPayCost
			&& !Inspection.Cooldown.bIsOnCooldown
			&& Inspection.Activation.bCanActivate;
		if (Entry.bReady)
		{
			Entry.States.Add(EBertaGSCAbilityReadinessState::Ready);
		}

		Entry.bHasProblem = Entry.States.Contains(EBertaGSCAbilityReadinessState::NotFullyInspectable)
			|| Entry.States.Contains(EBertaGSCAbilityReadinessState::BlockedByCost)
			|| Entry.States.Contains(EBertaGSCAbilityReadinessState::BlockedByCooldown)
			|| Entry.States.Contains(EBertaGSCAbilityReadinessState::BlockedByActivation);
	}

	void SortEntries(TArray<FBertaGSCAbilityReadinessEntry>& Entries)
	{
		Entries.Sort([](const FBertaGSCAbilityReadinessEntry& Left, const FBertaGSCAbilityReadinessEntry& Right)
		{
			const int32 PathComparison = Left.Inspection.AbilityClassPath.Compare(
				Right.Inspection.AbilityClassPath, ESearchCase::CaseSensitive);
			return PathComparison == 0
				? Left.AbilitySpecHandle.ToString() < Right.AbilitySpecHandle.ToString()
				: PathComparison < 0;
		});
	}

	FString StateText(const TArray<EBertaGSCAbilityReadinessState>& States)
	{
		TArray<FString> Names;
		Names.Reserve(States.Num());
		for (const EBertaGSCAbilityReadinessState State : States)
		{
			Names.Add(StateName(State));
		}
		return FString::Join(Names, TEXT(","));
	}
}

bool UBertaGSCAbilityReadinessLibrary::BuildAbilityReadinessReport(
	AActor* Actor,
	FBertaGSCAbilityReadinessReport& OutReport)
{
	OutReport = FBertaGSCAbilityReadinessReport();
	OutReport.ActorPath = IsValid(Actor) ? Actor->GetPathName() : TEXT("None");
	if (!IsInGameThread() || !IsValid(Actor))
	{
		OutReport.Summary = FormatAbilityReadinessReport(OutReport);
		OutReport.ProblemsSummary = FormatAbilityReadinessProblems(OutReport);
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	if (!AbilitySystemComponent)
	{
		OutReport.Summary = FormatAbilityReadinessReport(OutReport);
		OutReport.ProblemsSummary = FormatAbilityReadinessProblems(OutReport);
		return false;
	}

	OutReport.bAbilitySystemComponentFound = true;
	OutReport.AbilitySystemComponentPath = AbilitySystemComponent->GetPathName();
	TArray<FGameplayAbilitySpecHandle> Handles;
	AbilitySystemComponent->GetAllAbilities(Handles);
	TMap<const UClass*, int32> SpecCountsByClass;
	for (const FGameplayAbilitySpecHandle Handle : Handles)
	{
		if (const FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
			Spec && Spec->Ability)
		{
			++SpecCountsByClass.FindOrAdd(Spec->Ability->GetClass());
		}
	}
	const UGameplayAbility* QueuedAbility =
		BertaGSCAbilityActivation::GetCurrentQueuedAbility(Actor, *AbilitySystemComponent);
	OutReport.Entries.Reserve(Handles.Num());
	for (const FGameplayAbilitySpecHandle Handle : Handles)
	{
		const FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		if (!Spec)
		{
			continue;
		}

		FBertaGSCAbilityReadinessEntry& Entry = OutReport.Entries.AddDefaulted_GetRef();
		Entry.AbilitySpecHandle = Handle;
		Entry.AbilityClass = Spec->Ability ? Spec->Ability->GetClass() : nullptr;
		BertaGSCAbilityActivation::BuildForSpec(Actor, *AbilitySystemComponent, *Spec, Entry.Inspection);

		Entry.bQueueEligibilityKnown = Entry.Inspection.bAbilityQueueComponentFound;
		Entry.bAllowedByCurrentQueue = Entry.bQueueEligibilityKnown
			&& Entry.Inspection.bAbilityQueueEnabled
			&& (Entry.Inspection.bAbilityQueueAllowsAll
				|| Entry.Inspection.QueueAllowedAbilityClasses.Contains(Entry.AbilityClass));
		if (Entry.Inspection.bAbilityQueueComponentFound)
		{
			if (!QueuedAbility || QueuedAbility->GetClass() != Entry.AbilityClass.Get())
			{
				Entry.bCurrentQueuedSpecKnown = true;
			}
			else if (SpecCountsByClass.FindRef(Entry.AbilityClass.Get()) == 1)
			{
				Entry.bCurrentQueuedSpecKnown = true;
				Entry.bIsCurrentlyQueued = true;
			}
			else if (const UGameplayAbility* PrimaryInstance = Spec->GetPrimaryInstance(); PrimaryInstance == QueuedAbility)
			{
				Entry.bCurrentQueuedSpecKnown = true;
				Entry.bIsCurrentlyQueued = true;
			}
		}
		BertaGSCAbilityReadiness::Classify(Entry);
	}

	BertaGSCAbilityReadiness::SortEntries(OutReport.Entries);
	OutReport.TotalGrantedAbilities = OutReport.Entries.Num();
	for (const FBertaGSCAbilityReadinessEntry& Entry : OutReport.Entries)
	{
		OutReport.ReadyCount += Entry.bReady ? 1 : 0;
		OutReport.ActiveCount += Entry.Inspection.bAbilityActive ? 1 : 0;
		OutReport.ProblemCount += Entry.bHasProblem ? 1 : 0;
	}
	OutReport.Summary = FormatAbilityReadinessReport(OutReport);
	OutReport.ProblemsSummary = FormatAbilityReadinessProblems(OutReport);
	return true;
}

FString UBertaGSCAbilityReadinessLibrary::FormatAbilityReadinessReport(
	const FBertaGSCAbilityReadinessReport& Report)
{
	TArray<FString> Lines;
	Lines.Add(FString::Printf(
		TEXT("Actor: %s\nASC: %s\nGranted: %d | Ready: %d | Active: %d | Problems: %d"),
		*Report.ActorPath,
		Report.bAbilitySystemComponentFound ? *Report.AbilitySystemComponentPath : TEXT("Not found"),
		Report.TotalGrantedAbilities,
		Report.ReadyCount,
		Report.ActiveCount,
		Report.ProblemCount));
	for (const FBertaGSCAbilityReadinessEntry& Entry : Report.Entries)
	{
		Lines.Add(FormatEntry(Entry));
	}
	return FString::Join(Lines, TEXT("\n"));
}

FString UBertaGSCAbilityReadinessLibrary::FormatAbilityReadinessProblems(
	const FBertaGSCAbilityReadinessReport& Report)
{
	TArray<FString> Lines;
	Lines.Add(FString::Printf(
		TEXT("Actor: %s | Ability problems: %d"), *Report.ActorPath, Report.ProblemCount));
	for (const FBertaGSCAbilityReadinessEntry& Entry : Report.Entries)
	{
		if (Entry.bHasProblem)
		{
			Lines.Add(FormatEntry(Entry));
		}
	}
	return FString::Join(Lines, TEXT("\n"));
}
