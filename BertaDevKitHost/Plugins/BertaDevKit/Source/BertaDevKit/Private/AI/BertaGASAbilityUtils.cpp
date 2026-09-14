#include "AI/BertaGASAbilityUtils.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpec.h"

namespace BertaGASAbilityUtils::Private
{
struct FResolvedAbility
{
	UAbilitySystemComponent* AbilitySystemComponent = nullptr;
	const FGameplayAbilitySpec* AbilitySpec = nullptr;
	const UGameplayAbility* Ability = nullptr;
	const FGameplayAbilityActorInfo* ActorInfo = nullptr;
};

bool ResolveAbility(
	AActor* Actor,
	const TSubclassOf<UGameplayAbility> AbilityClass,
	FResolvedAbility& OutResolvedAbility)
{
	OutResolvedAbility = FResolvedAbility();
	if (!IsInGameThread() || !IsValid(Actor) || !AbilityClass)
	{
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent
		? AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass)
		: nullptr;
	const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent
		? AbilitySystemComponent->AbilityActorInfo.Get()
		: nullptr;
	if (!AbilitySystemComponent || !AbilitySpec || !ActorInfo)
	{
		return false;
	}

	const UGameplayAbility* Ability = AbilitySpec->GetPrimaryInstance();
	if (!Ability)
	{
		Ability = AbilitySpec->Ability.Get();
	}
	if (!Ability)
	{
		return false;
	}

	OutResolvedAbility.AbilitySystemComponent = AbilitySystemComponent;
	OutResolvedAbility.AbilitySpec = AbilitySpec;
	OutResolvedAbility.Ability = Ability;
	OutResolvedAbility.ActorInfo = ActorInfo;
	return true;
}
}

bool UBertaGASAbilityUtils::GetAbilityCooldownInfo(
	AActor* Actor,
	const TSubclassOf<UGameplayAbility> AbilityClass,
	FBertaAbilityCooldownInfo& OutInfo)
{
	OutInfo = FBertaAbilityCooldownInfo();
	BertaGASAbilityUtils::Private::FResolvedAbility ResolvedAbility;
	if (!BertaGASAbilityUtils::Private::ResolveAbility(Actor, AbilityClass, ResolvedAbility))
	{
		return false;
	}

	float TimeRemaining = 0.0f;
	float Duration = 0.0f;
	ResolvedAbility.Ability->GetCooldownTimeRemainingAndDuration(
		ResolvedAbility.AbilitySpec->Handle,
		ResolvedAbility.ActorInfo,
		TimeRemaining,
		Duration);
	OutInfo.bIsOnCooldown = !ResolvedAbility.Ability->CheckCooldown(
		ResolvedAbility.AbilitySpec->Handle,
		ResolvedAbility.ActorInfo);
	OutInfo.TimeRemainingSeconds = FMath::Max(0.0f, TimeRemaining);
	OutInfo.DurationSeconds = FMath::Max(0.0f, Duration);
	OutInfo.RemainingNormalized = OutInfo.DurationSeconds > 0.0f
		? FMath::Clamp(OutInfo.TimeRemainingSeconds / OutInfo.DurationSeconds, 0.0f, 1.0f)
		: 0.0f;
	return true;
}

bool UBertaGASAbilityUtils::CheckAbilityCost(
	AActor* Actor,
	const TSubclassOf<UGameplayAbility> AbilityClass,
	FBertaAbilityCostCheckResult& OutResult)
{
	OutResult = FBertaAbilityCostCheckResult();
	BertaGASAbilityUtils::Private::FResolvedAbility ResolvedAbility;
	if (!BertaGASAbilityUtils::Private::ResolveAbility(Actor, AbilityClass, ResolvedAbility))
	{
		return false;
	}

	OutResult.bCanPayCost = ResolvedAbility.Ability->CheckCost(
		ResolvedAbility.AbilitySpec->Handle,
		ResolvedAbility.ActorInfo,
		&OutResult.FailureTags);
	return true;
}

bool UBertaGASAbilityUtils::CheckAbilityActivation(
	AActor* Actor,
	const TSubclassOf<UGameplayAbility> AbilityClass,
	FBertaAbilityActivationCheckResult& OutResult)
{
	OutResult = FBertaAbilityActivationCheckResult();
	BertaGASAbilityUtils::Private::FResolvedAbility ResolvedAbility;
	if (!BertaGASAbilityUtils::Private::ResolveAbility(Actor, AbilityClass, ResolvedAbility))
	{
		return false;
	}

	OutResult.bCanActivate = ResolvedAbility.Ability->CanActivateAbility(
		ResolvedAbility.AbilitySpec->Handle,
		ResolvedAbility.ActorInfo,
		nullptr,
		nullptr,
		&OutResult.FailureTags);
	return true;
}
