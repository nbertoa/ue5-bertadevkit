#include "Abilities/BertaGSCAbilityActivationReport.h"
#include "Abilities/BertaGSCAbilityActivationReportInternal.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbility.h"
#include "Components/GSCAbilityInputBindingComponent.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpec.h"
#include "InputAction.h"

namespace BertaGSCAbilityActivationPrivate
{
	template <typename TComponent>
	TComponent* FindRelevantComponent(AActor* Actor, UAbilitySystemComponent* AbilitySystemComponent)
	{
		if (TComponent* Component = Actor->FindComponentByClass<TComponent>())
		{
			return Component;
		}

		AActor* AvatarActor = AbilitySystemComponent ? AbilitySystemComponent->GetAvatarActor() : nullptr;
		return AvatarActor && AvatarActor != Actor ? AvatarActor->FindComponentByClass<TComponent>() : nullptr;
	}

	const FGameplayAbilitySpec* FindExactAbilitySpec(
		const UAbilitySystemComponent& AbilitySystemComponent,
		const UClass* ExactAbilityClass)
	{
		TArray<FGameplayAbilitySpecHandle> Handles;
		AbilitySystemComponent.GetAllAbilities(Handles);
		for (const FGameplayAbilitySpecHandle Handle : Handles)
		{
			const FGameplayAbilitySpec* Spec = AbilitySystemComponent.FindAbilitySpecFromHandle(Handle);
			if (Spec && Spec->Ability && Spec->Ability->GetClass() == ExactAbilityClass)
			{
				return Spec;
			}
		}

		return nullptr;
	}

}

namespace BertaGSCAbilityActivation
{
	FString BoolText(const bool bValue)
	{
		return bValue ? TEXT("Yes") : TEXT("No");
	}

	FString SortedTags(const FGameplayTagContainer& Tags)
	{
		TArray<FString> TagNames;
		for (const FGameplayTag& Tag : Tags.GetGameplayTagArray())
		{
			TagNames.Add(Tag.ToString());
		}
		TagNames.Sort();
		return FString::Join(TagNames, TEXT(","));
	}

	const UGameplayAbility* GetCurrentQueuedAbility(
		AActor* Actor,
		UAbilitySystemComponent& AbilitySystemComponent)
	{
		const UGSCAbilityQueueComponent* AbilityQueue =
			BertaGSCAbilityActivationPrivate::FindRelevantComponent<UGSCAbilityQueueComponent>(
				Actor, &AbilitySystemComponent);
		return AbilityQueue ? AbilityQueue->GetCurrentQueuedAbility() : nullptr;
	}

	bool BuildForSpec(
		AActor* Actor,
		UAbilitySystemComponent& AbilitySystemComponent,
		const FGameplayAbilitySpec& AbilitySpec,
		FBertaGSCAbilityActivationReport& OutReport)
	{
		OutReport = FBertaGSCAbilityActivationReport();
		OutReport.ActorPath = IsValid(Actor) ? Actor->GetPathName() : TEXT("None");
		OutReport.bAbilitySystemComponentFound = true;
		OutReport.bExactAbilityGranted = AbilitySpec.Ability != nullptr;
		OutReport.AbilityClassPath = AbilitySpec.Ability
			? AbilitySpec.Ability->GetClass()->GetPathName()
			: TEXT("None");

		const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent.AbilityActorInfo.Get();
		const UGameplayAbility* Ability = AbilitySpec.GetPrimaryInstance();
		if (!Ability)
		{
			Ability = AbilitySpec.Ability.Get();
		}

		if (Ability)
		{
			OutReport.GrantedLevel = AbilitySpec.Level;
			OutReport.bAbilityActive = AbilitySpec.IsActive();
			if (ActorInfo)
			{
				OutReport.bCostCheckAvailable = true;
				OutReport.Cost.bCanPayCost = Ability->CheckCost(
					AbilitySpec.Handle, ActorInfo, &OutReport.Cost.FailureTags);

				float TimeRemaining = 0.0f;
				float Duration = 0.0f;
				Ability->GetCooldownTimeRemainingAndDuration(
					AbilitySpec.Handle, ActorInfo, TimeRemaining, Duration);
				OutReport.bCooldownCheckAvailable = true;
				OutReport.Cooldown.bIsOnCooldown = !Ability->CheckCooldown(
					AbilitySpec.Handle, ActorInfo);
				OutReport.Cooldown.TimeRemainingSeconds = FMath::Max(0.0f, TimeRemaining);
				OutReport.Cooldown.DurationSeconds = FMath::Max(0.0f, Duration);
				OutReport.Cooldown.RemainingNormalized = OutReport.Cooldown.DurationSeconds > 0.0f
					? FMath::Clamp(
						OutReport.Cooldown.TimeRemainingSeconds / OutReport.Cooldown.DurationSeconds,
						0.0f,
						1.0f)
					: 0.0f;

				OutReport.bActivationCheckAvailable = true;
				OutReport.Activation.bCanActivate = Ability->CanActivateAbility(
					AbilitySpec.Handle,
					ActorInfo,
					nullptr,
					nullptr,
					&OutReport.Activation.FailureTags);
			}

			if (UGSCAbilityInputBindingComponent* InputBinding =
				BertaGSCAbilityActivationPrivate::FindRelevantComponent<UGSCAbilityInputBindingComponent>(
					Actor, &AbilitySystemComponent))
			{
				OutReport.BoundInputAction = InputBinding->GetBoundInputActionForAbilitySpec(&AbilitySpec);
			}
		}

		if (UGSCAbilityQueueComponent* AbilityQueue =
			BertaGSCAbilityActivationPrivate::FindRelevantComponent<UGSCAbilityQueueComponent>(
				Actor, &AbilitySystemComponent))
		{
			OutReport.bAbilityQueueComponentFound = true;
			OutReport.bAbilityQueueEnabled = AbilityQueue->bAbilityQueueEnabled;
			OutReport.bAbilityQueueOpened = AbilityQueue->IsAbilityQueueOpened();
			OutReport.bAbilityQueueAllowsAll = AbilityQueue->IsAllAbilitiesAllowedForAbilityQueue();
			OutReport.QueueAllowedAbilityClasses = AbilityQueue->GetQueuedAllowedAbilities();
			if (const UGameplayAbility* QueuedAbility = AbilityQueue->GetCurrentQueuedAbility())
			{
				OutReport.QueuedAbilityClassPath = QueuedAbility->GetClass()->GetPathName();
			}
		}

		OutReport.Summary = UBertaGSCAbilityActivationLibrary::FormatAbilityActivationReport(OutReport);
		return Ability
			&& OutReport.bCostCheckAvailable
			&& OutReport.bCooldownCheckAvailable
			&& OutReport.bActivationCheckAvailable;
	}
}

bool UBertaGSCAbilityActivationLibrary::BuildAbilityActivationReport(
	AActor* Actor,
	const TSubclassOf<UGameplayAbility> ExactAbilityClass,
	FBertaGSCAbilityActivationReport& OutReport)
{
	OutReport = FBertaGSCAbilityActivationReport();
	OutReport.ActorPath = IsValid(Actor) ? Actor->GetPathName() : TEXT("None");
	OutReport.AbilityClassPath = ExactAbilityClass ? ExactAbilityClass->GetPathName() : TEXT("None");

	if (!IsInGameThread() || !IsValid(Actor) || !ExactAbilityClass)
	{
		OutReport.Summary = FormatAbilityActivationReport(OutReport);
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	OutReport.bAbilitySystemComponentFound = AbilitySystemComponent != nullptr;
	if (!AbilitySystemComponent)
	{
		OutReport.Summary = FormatAbilityActivationReport(OutReport);
		return false;
	}

	const FGameplayAbilitySpec* AbilitySpec =
		BertaGSCAbilityActivationPrivate::FindExactAbilitySpec(*AbilitySystemComponent, ExactAbilityClass.Get());
	if (!AbilitySpec)
	{
		OutReport.Summary = FormatAbilityActivationReport(OutReport);
		return false;
	}

	return BertaGSCAbilityActivation::BuildForSpec(
		Actor, *AbilitySystemComponent, *AbilitySpec, OutReport);
}

FString UBertaGSCAbilityActivationLibrary::FormatAbilityActivationReport(
	const FBertaGSCAbilityActivationReport& Report)
{
	TArray<FString> AllowedAbilityPaths;
	AllowedAbilityPaths.Reserve(Report.QueueAllowedAbilityClasses.Num());
	for (const TSubclassOf<UGameplayAbility> AbilityClass : Report.QueueAllowedAbilityClasses)
	{
		AllowedAbilityPaths.Add(AbilityClass ? AbilityClass->GetPathName() : TEXT("None"));
	}
	AllowedAbilityPaths.Sort();

	return FString::Printf(
		TEXT("Actor=%s | Ability=%s | ASC=%s | Granted=%s | Level=%d | Active=%s | CostAvailable=%s Cost=%s [%s] | CooldownAvailable=%s Cooldown=%s Remaining=%.3fs Duration=%.3fs Normalized=%.3f | ActivationAvailable=%s CanActivate=%s [%s] | InputAction=%s | Queue=%s Enabled=%s Opened=%s AllowAll=%s Queued=%s Allowed=[%s]"),
		*Report.ActorPath,
		*Report.AbilityClassPath,
		*BertaGSCAbilityActivation::BoolText(Report.bAbilitySystemComponentFound),
		*BertaGSCAbilityActivation::BoolText(Report.bExactAbilityGranted),
		Report.GrantedLevel,
		*BertaGSCAbilityActivation::BoolText(Report.bAbilityActive),
		*BertaGSCAbilityActivation::BoolText(Report.bCostCheckAvailable),
		*BertaGSCAbilityActivation::BoolText(Report.Cost.bCanPayCost),
		*BertaGSCAbilityActivation::SortedTags(Report.Cost.FailureTags),
		*BertaGSCAbilityActivation::BoolText(Report.bCooldownCheckAvailable),
		*BertaGSCAbilityActivation::BoolText(Report.Cooldown.bIsOnCooldown),
		Report.Cooldown.TimeRemainingSeconds,
		Report.Cooldown.DurationSeconds,
		Report.Cooldown.RemainingNormalized,
		*BertaGSCAbilityActivation::BoolText(Report.bActivationCheckAvailable),
		*BertaGSCAbilityActivation::BoolText(Report.Activation.bCanActivate),
		*BertaGSCAbilityActivation::SortedTags(Report.Activation.FailureTags),
		*GetPathNameSafe(Report.BoundInputAction),
		*BertaGSCAbilityActivation::BoolText(Report.bAbilityQueueComponentFound),
		*BertaGSCAbilityActivation::BoolText(Report.bAbilityQueueEnabled),
		*BertaGSCAbilityActivation::BoolText(Report.bAbilityQueueOpened),
		*BertaGSCAbilityActivation::BoolText(Report.bAbilityQueueAllowsAll),
		Report.QueuedAbilityClassPath.IsEmpty() ? TEXT("None") : *Report.QueuedAbilityClassPath,
		*FString::Join(AllowedAbilityPaths, TEXT(",")));
}
