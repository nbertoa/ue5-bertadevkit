#include "AI/BertaBTService_TraceGameplayTags.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Debug/BertaDebugLog.h"
#include "GameFramework/Pawn.h"

UBertaBTService_TraceGameplayTags::UBertaBTService_TraceGameplayTags(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Trace Gameplay Tags");
	bCreateNodeInstance = true;
	INIT_SERVICE_NODE_NOTIFY_FLAGS();
	bNotifyTick = false;
}

void UBertaBTService_TraceGameplayTags::OnBecomeRelevant(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UnregisterGameplayTagEvents();

	const TArray<FGameplayTag>& Tags = ObservedTags.GetGameplayTagArray();
	if (Tags.IsEmpty())
	{
		UE_LOG(
			LogBertaDebug,
			Warning,
			TEXT("Trace Gameplay Tags%s: no tags are configured."),
			Label.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" [%s]"), *Label));
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
	if (!AbilitySystemComponent)
	{
		return;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
	Registrations.Reserve(Tags.Num());
	for (const FGameplayTag& Tag : Tags)
	{
		if (!Tag.IsValid())
		{
			continue;
		}

		FGameplayTagEventRegistration& Registration = Registrations.AddDefaulted_GetRef();
		Registration.Tag = Tag;
		Registration.Handle = AbilitySystemComponent
			->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::AnyCountChange)
			.AddUObject(this, &ThisClass::HandleGameplayTagChanged);
	}
	bIsObserving = !Registrations.IsEmpty();
}

void UBertaBTService_TraceGameplayTags::OnCeaseRelevant(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterGameplayTagEvents();
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBertaBTService_TraceGameplayTags::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterGameplayTagEvents();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTService_TraceGameplayTags::GetStaticDescription() const
{
	TArray<FString> TagNames;
	for (const FGameplayTag& Tag : ObservedTags.GetGameplayTagArray())
	{
		TagNames.Add(Tag.ToString());
	}
	TagNames.Sort();

	return FString::Printf(
		TEXT("%s\nTags: %s\nLabel: %s"),
		*Super::GetStaticDescription(),
		TagNames.IsEmpty() ? TEXT("None") : *FString::Join(TagNames, TEXT(", ")),
		Label.IsEmpty() ? TEXT("None") : *Label);
}

void UBertaBTService_TraceGameplayTags::HandleGameplayTagChanged(
	const FGameplayTag Tag,
	const int32 NewCount)
{
	if (!bIsObserving || !ObservedAbilitySystemComponent.IsValid())
	{
		return;
	}

	const TCHAR* Present = NewCount > 0 ? TEXT("true") : TEXT("false");
	if (Label.IsEmpty())
	{
		UE_LOG(
			LogBertaDebug,
			Log,
			TEXT("Gameplay Tag %s Count=%d Present=%s"),
			*Tag.ToString(),
			NewCount,
			Present);
	}
	else
	{
		UE_LOG(
			LogBertaDebug,
			Log,
			TEXT("[%s] Gameplay Tag %s Count=%d Present=%s"),
			*Label,
			*Tag.ToString(),
			NewCount,
			Present);
	}
}

void UBertaBTService_TraceGameplayTags::UnregisterGameplayTagEvents()
{
	bIsObserving = false;
	if (UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get())
	{
		for (const FGameplayTagEventRegistration& Registration : Registrations)
		{
			if (Registration.Tag.IsValid() && Registration.Handle.IsValid())
			{
				AbilitySystemComponent->UnregisterGameplayTagEvent(
					Registration.Handle,
					Registration.Tag,
					EGameplayTagEventType::AnyCountChange);
			}
		}
	}

	Registrations.Reset();
	ObservedAbilitySystemComponent.Reset();
}
