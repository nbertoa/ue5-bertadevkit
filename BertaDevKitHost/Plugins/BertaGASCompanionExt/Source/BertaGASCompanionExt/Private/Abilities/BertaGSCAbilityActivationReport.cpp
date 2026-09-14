#include "Abilities/BertaGSCAbilityActivationReport.h"

#include "AI/BertaGASAbilityUtils.h"
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
	OutReport.bExactAbilityGranted = AbilitySpec != nullptr;
	if (AbilitySpec)
	{
		OutReport.GrantedLevel = AbilitySpec->Level;
		OutReport.bAbilityActive = AbilitySpec->IsActive();
		OutReport.bCostCheckAvailable = UBertaGASAbilityUtils::CheckAbilityCost(Actor, ExactAbilityClass, OutReport.Cost);
		OutReport.bCooldownCheckAvailable = UBertaGASAbilityUtils::GetAbilityCooldownInfo(Actor, ExactAbilityClass, OutReport.Cooldown);
		OutReport.bActivationCheckAvailable = UBertaGASAbilityUtils::CheckAbilityActivation(Actor, ExactAbilityClass, OutReport.Activation);

		if (UGSCAbilityInputBindingComponent* InputBinding =
			BertaGSCAbilityActivationPrivate::FindRelevantComponent<UGSCAbilityInputBindingComponent>(Actor, AbilitySystemComponent))
		{
			OutReport.BoundInputAction = InputBinding->GetBoundInputActionForAbilitySpec(AbilitySpec);
		}
	}

	if (UGSCAbilityQueueComponent* AbilityQueue =
		BertaGSCAbilityActivationPrivate::FindRelevantComponent<UGSCAbilityQueueComponent>(Actor, AbilitySystemComponent))
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

	OutReport.Summary = FormatAbilityActivationReport(OutReport);
	return OutReport.bExactAbilityGranted
		&& OutReport.bCostCheckAvailable
		&& OutReport.bCooldownCheckAvailable
		&& OutReport.bActivationCheckAvailable;
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
		*BertaGSCAbilityActivationPrivate::BoolText(Report.bAbilitySystemComponentFound),
		*BertaGSCAbilityActivationPrivate::BoolText(Report.bExactAbilityGranted),
		Report.GrantedLevel,
		*BertaGSCAbilityActivationPrivate::BoolText(Report.bAbilityActive),
		*BertaGSCAbilityActivationPrivate::BoolText(Report.bCostCheckAvailable),
		*BertaGSCAbilityActivationPrivate::BoolText(Report.Cost.bCanPayCost),
		*BertaGSCAbilityActivationPrivate::SortedTags(Report.Cost.FailureTags),
		*BertaGSCAbilityActivationPrivate::BoolText(Report.bCooldownCheckAvailable),
		*BertaGSCAbilityActivationPrivate::BoolText(Report.Cooldown.bIsOnCooldown),
		Report.Cooldown.TimeRemainingSeconds,
		Report.Cooldown.DurationSeconds,
		Report.Cooldown.RemainingNormalized,
		*BertaGSCAbilityActivationPrivate::BoolText(Report.bActivationCheckAvailable),
		*BertaGSCAbilityActivationPrivate::BoolText(Report.Activation.bCanActivate),
		*BertaGSCAbilityActivationPrivate::SortedTags(Report.Activation.FailureTags),
		*GetPathNameSafe(Report.BoundInputAction),
		*BertaGSCAbilityActivationPrivate::BoolText(Report.bAbilityQueueComponentFound),
		*BertaGSCAbilityActivationPrivate::BoolText(Report.bAbilityQueueEnabled),
		*BertaGSCAbilityActivationPrivate::BoolText(Report.bAbilityQueueOpened),
		*BertaGSCAbilityActivationPrivate::BoolText(Report.bAbilityQueueAllowsAll),
		Report.QueuedAbilityClassPath.IsEmpty() ? TEXT("None") : *Report.QueuedAbilityClassPath,
		*FString::Join(AllowedAbilityPaths, TEXT(",")));
}
