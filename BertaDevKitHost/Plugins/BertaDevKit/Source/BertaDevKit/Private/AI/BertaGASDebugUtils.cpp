#include "AI/BertaGASDebugUtils.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"

namespace BertaGASDebugUtilsPrivate
{
FString FormatSection(const TCHAR* Heading, const TArray<FString>& Items)
{
	if (Items.IsEmpty())
	{
		return FString::Printf(TEXT("%s (0): None"), Heading);
	}

	return FString::Printf(
		TEXT("%s (%d):\n- %s"),
		Heading,
		Items.Num(),
		*FString::Join(Items, TEXT("\n- ")));
}
}

bool UBertaGASDebugUtils::GetDebugSummary(AActor* Actor, FString& OutSummary)
{
	OutSummary.Reset();
	if (!IsValid(Actor))
	{
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	if (!AbilitySystemComponent)
	{
		return false;
	}

	TArray<FString> TagNames;
	FGameplayTagContainer OwnedTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);
	for (const FGameplayTag& Tag : OwnedTags.GetGameplayTagArray())
	{
		TagNames.Add(Tag.ToString());
	}
	TagNames.Sort();

	TArray<FString> AbilityDescriptions;
	TArray<FGameplayAbilitySpecHandle> AbilityHandles;
	AbilitySystemComponent->GetAllAbilities(AbilityHandles);
	for (const FGameplayAbilitySpecHandle AbilityHandle : AbilityHandles)
	{
		const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilityHandle);
		if (!AbilitySpec || !AbilitySpec->Ability)
		{
			continue;
		}

		AbilityDescriptions.Add(FString::Printf(
			TEXT("%s | %s | Level %d"),
			*AbilitySpec->Ability->GetClass()->GetName(),
			AbilitySpec->IsActive() ? TEXT("Active") : TEXT("Inactive"),
			AbilitySpec->Level));
	}
	AbilityDescriptions.Sort();

	TArray<FString> GameplayEffectDescriptions;
	const TArray<FActiveGameplayEffectHandle> ActiveEffectHandles =
		AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery());
	const UWorld* World = AbilitySystemComponent->GetWorld();
	const float WorldTime = World ? World->GetTimeSeconds() : 0.0f;
	for (const FActiveGameplayEffectHandle ActiveEffectHandle : ActiveEffectHandles)
	{
		const FActiveGameplayEffect* ActiveEffect =
			AbilitySystemComponent->GetActiveGameplayEffect(ActiveEffectHandle);
		if (!ActiveEffect || !ActiveEffect->Spec.Def)
		{
			continue;
		}

		const float Duration = ActiveEffect->GetDuration();
		const FString Timing = Duration == FGameplayEffectConstants::INFINITE_DURATION
			? TEXT("Infinite")
			: FString::Printf(
				TEXT("Duration %.2fs | Remaining %.2fs"),
				Duration,
				World ? ActiveEffect->GetTimeRemaining(WorldTime) : 0.0f);
		GameplayEffectDescriptions.Add(FString::Printf(
			TEXT("%s | Stacks %d | %s"),
			*ActiveEffect->Spec.Def->GetClass()->GetName(),
			ActiveEffect->Spec.GetStackCount(),
			*Timing));
	}
	GameplayEffectDescriptions.Sort();

	TArray<FString> AttributeDescriptions;
	TArray<FGameplayAttribute> Attributes;
	AbilitySystemComponent->GetAllAttributes(Attributes);
	for (const FGameplayAttribute& Attribute : Attributes)
	{
		bool bFound = false;
		const float Value = AbilitySystemComponent->GetGameplayAttributeValue(Attribute, bFound);
		if (bFound)
		{
			AttributeDescriptions.Add(FString::Printf(TEXT("%s = %.3f"), *Attribute.GetName(), Value));
		}
	}
	AttributeDescriptions.Sort();

	TArray<FString> Sections;
	Sections.Reserve(6);
	Sections.Add(FString::Printf(TEXT("Actor: %s"), *Actor->GetPathName()));
	Sections.Add(FString::Printf(TEXT("Ability System Component: %s"), *AbilitySystemComponent->GetName()));
	Sections.Add(BertaGASDebugUtilsPrivate::FormatSection(TEXT("Owned Gameplay Tags"), TagNames));
	Sections.Add(BertaGASDebugUtilsPrivate::FormatSection(TEXT("Granted Abilities"), AbilityDescriptions));
	Sections.Add(BertaGASDebugUtilsPrivate::FormatSection(TEXT("Active Gameplay Effects"), GameplayEffectDescriptions));
	Sections.Add(BertaGASDebugUtilsPrivate::FormatSection(TEXT("Gameplay Attributes"), AttributeDescriptions));
	OutSummary = FString::Join(Sections, TEXT("\n"));
	return true;
}
