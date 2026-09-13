#include "AI/BertaBTTask_WaitGameplayTagQuery.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"

UBertaBTTask_WaitGameplayTagQuery::UBertaBTTask_WaitGameplayTagQuery(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Wait Gameplay Tag Query");
	bCreateNodeInstance = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBertaBTTask_WaitGameplayTagQuery::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnregisterGameplayTagEvents();

	if (GameplayTagQuery.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!AbilitySystemComponent)
	{
		return EBTNodeResult::Failed;
	}

	FGameplayTagContainer OwnedGameplayTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedGameplayTags);
	if (IsWaitConditionSatisfied(GameplayTagQuery.Matches(OwnedGameplayTags)))
	{
		return EBTNodeResult::Succeeded;
	}

	const TArray<FGameplayTag>& QueryGameplayTags = GameplayTagQuery.GetGameplayTagArray();
	if (QueryGameplayTags.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
	ObservedBehaviorTreeComponent = &OwnerComp;
	RegisteredGameplayTagEvents.Reserve(QueryGameplayTags.Num());

	for (const FGameplayTag& GameplayTag : QueryGameplayTags)
	{
		if (!GameplayTag.IsValid())
		{
			continue;
		}

		FRegisteredGameplayTagEvent& Registration = RegisteredGameplayTagEvents.AddDefaulted_GetRef();
		Registration.Tag = GameplayTag;
		Registration.Handle = AbilitySystemComponent
			->RegisterGameplayTagEvent(GameplayTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ThisClass::HandleGameplayTagChanged);
	}

	if (RegisteredGameplayTagEvents.IsEmpty())
	{
		UnregisterGameplayTagEvents();
		return EBTNodeResult::Failed;
	}

	bIsWaiting = true;
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBertaBTTask_WaitGameplayTagQuery::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnregisterGameplayTagEvents();
	return EBTNodeResult::Aborted;
}

void UBertaBTTask_WaitGameplayTagQuery::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	const EBTNodeResult::Type TaskResult)
{
	UnregisterGameplayTagEvents();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBertaBTTask_WaitGameplayTagQuery::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterGameplayTagEvents();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTTask_WaitGameplayTagQuery::GetStaticDescription() const
{
	const TCHAR* WaitConditionDescription = WaitCondition == EBertaGameplayTagQueryWaitCondition::Matches
		? TEXT("Matches")
		: TEXT("Does Not Match");
	const FString QueryDescription = GameplayTagQuery.IsEmpty() ? TEXT("None") : GameplayTagQuery.GetDescription();

	return FString::Printf(
		TEXT("%s\nWait Until: %s\nGameplay Tag Query:\n%s"),
		*Super::GetStaticDescription(),
		WaitConditionDescription,
		*QueryDescription);
}

UAbilitySystemComponent* UBertaBTTask_WaitGameplayTagQuery::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

bool UBertaBTTask_WaitGameplayTagQuery::IsWaitConditionSatisfied(const bool bQueryMatches) const
{
	return WaitCondition == EBertaGameplayTagQueryWaitCondition::Matches ? bQueryMatches : !bQueryMatches;
}

void UBertaBTTask_WaitGameplayTagQuery::HandleGameplayTagChanged(const FGameplayTag CallbackTag, const int32 NewCount)
{
	if (!bIsWaiting)
	{
		return;
	}

	UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get();
	UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
	if (!BehaviorTreeComponent)
	{
		UnregisterGameplayTagEvents();
		return;
	}

	if (!AbilitySystemComponent)
	{
		UnregisterGameplayTagEvents();
		FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Failed);
		return;
	}

	FGameplayTagContainer OwnedGameplayTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedGameplayTags);
	if (!IsWaitConditionSatisfied(GameplayTagQuery.Matches(OwnedGameplayTags)))
	{
		return;
	}

	UnregisterGameplayTagEvents();
	FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
}

void UBertaBTTask_WaitGameplayTagQuery::UnregisterGameplayTagEvents()
{
	bIsWaiting = false;

	if (UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get())
	{
		for (FRegisteredGameplayTagEvent& Registration : RegisteredGameplayTagEvents)
		{
			if (Registration.Handle.IsValid() && Registration.Tag.IsValid())
			{
				AbilitySystemComponent->UnregisterGameplayTagEvent(
					Registration.Handle,
					Registration.Tag,
					EGameplayTagEventType::NewOrRemoved);
			}

			Registration.Handle.Reset();
		}
	}

	RegisteredGameplayTagEvents.Reset();
	ObservedAbilitySystemComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
}
