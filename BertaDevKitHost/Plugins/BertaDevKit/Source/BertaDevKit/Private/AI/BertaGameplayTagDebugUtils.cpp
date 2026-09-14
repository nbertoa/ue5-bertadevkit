#include "AI/BertaGameplayTagDebugUtils.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"

namespace BertaGameplayTagDebugUtilsPrivate
{
void SortTags(TArray<FGameplayTag>& Tags)
{
	Tags.Sort([](const FGameplayTag& Left, const FGameplayTag& Right)
	{
		return Left.ToString() < Right.ToString();
	});
}
}

void UBertaGameplayTagDebugUtils::GetGameplayTagContainerSummary(
	const FGameplayTagContainer& Tags,
	FString& OutSummary)
{
	TArray<FGameplayTag> SortedTags = Tags.GetGameplayTagArray();
	BertaGameplayTagDebugUtilsPrivate::SortTags(SortedTags);

	TArray<FString> TagNames;
	TagNames.Reserve(SortedTags.Num());
	for (const FGameplayTag& Tag : SortedTags)
	{
		TagNames.Add(Tag.ToString());
	}

	OutSummary = TagNames.IsEmpty() ? TEXT("None") : FString::Join(TagNames, TEXT("\n"));
}

void UBertaGameplayTagDebugUtils::GetGameplayTagQuerySummary(
	const FGameplayTagQuery& Query,
	FString& OutSummary)
{
	OutSummary = Query.IsEmpty() ? TEXT("None") : Query.GetDescription();
}

void UBertaGameplayTagDebugUtils::DiffGameplayTagContainers(
	const FGameplayTagContainer& Before,
	const FGameplayTagContainer& After,
	TArray<FGameplayTag>& OutAdded,
	TArray<FGameplayTag>& OutRemoved)
{
	OutAdded.Reset();
	OutRemoved.Reset();

	for (const FGameplayTag& Tag : After.GetGameplayTagArray())
	{
		if (!Before.HasTagExact(Tag))
		{
			OutAdded.Add(Tag);
		}
	}

	for (const FGameplayTag& Tag : Before.GetGameplayTagArray())
	{
		if (!After.HasTagExact(Tag))
		{
			OutRemoved.Add(Tag);
		}
	}

	BertaGameplayTagDebugUtilsPrivate::SortTags(OutAdded);
	BertaGameplayTagDebugUtilsPrivate::SortTags(OutRemoved);
}

bool UBertaGameplayTagDebugUtils::GetActorGameplayTagSummary(AActor* Actor, FString& OutSummary)
{
	OutSummary.Reset();
	if (!IsValid(Actor))
	{
		return false;
	}

	FGameplayTagContainer OwnedTags;
	if (const IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(Actor))
	{
		TagAssetInterface->GetOwnedGameplayTags(OwnedTags);
	}
	else
	{
		const UAbilitySystemComponent* AbilitySystemComponent =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
		if (!AbilitySystemComponent)
		{
			return false;
		}

		AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);
	}

	GetGameplayTagContainerSummary(OwnedTags, OutSummary);
	return true;
}
