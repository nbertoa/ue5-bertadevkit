#include "AI/BertaBTTask_WaitTargetGameplayTagQuery.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Actor.h"

UBertaBTTask_WaitTargetGameplayTagQuery::UBertaBTTask_WaitTargetGameplayTagQuery(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Wait Target Gameplay Tag Query");
	bCreateNodeInstance = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey),
		AActor::StaticClass());
}

void UBertaBTTask_WaitTargetGameplayTagQuery::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (const UBlackboardData* BlackboardAsset = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BlackboardAsset);
	}
	else
	{
		TargetActorKey.InvalidateResolvedKey();
	}
}

EBTNodeResult::Type UBertaBTTask_WaitTargetGameplayTagQuery::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterObservers();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard || !TargetActorKey.IsSet() || GameplayTagQuery.IsEmpty() ||
		GameplayTagQuery.GetGameplayTagArray().IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	if (IsCurrentTargetSatisfied(*Blackboard))
	{
		return EBTNodeResult::Succeeded;
	}

	ObservedBehaviorTreeComponent = &OwnerComp;
	ObservedBlackboardComponent = Blackboard;
	bIsWaiting = true;
	BlackboardObserverHandle = Blackboard->RegisterObserver(
		TargetActorKey.GetSelectedKeyID(),
		this,
		FOnBlackboardChangeNotification::CreateUObject(this, &ThisClass::HandleTargetActorChanged));
	if (!BlackboardObserverHandle.IsValid())
	{
		UnregisterObservers();
		return EBTNodeResult::Failed;
	}

	BindTargetAbilitySystemComponent(*Blackboard);
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBertaBTTask_WaitTargetGameplayTagQuery::AbortTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterObservers();
	return EBTNodeResult::Aborted;
}

void UBertaBTTask_WaitTargetGameplayTagQuery::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	const EBTNodeResult::Type TaskResult)
{
	UnregisterObservers();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBertaBTTask_WaitTargetGameplayTagQuery::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterObservers();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTTask_WaitTargetGameplayTagQuery::GetStaticDescription() const
{
	const TCHAR* WaitDescription = WaitCondition == EBertaGameplayTagQueryWaitCondition::Matches
		? TEXT("Matches")
		: TEXT("Does Not Match");
	const FString QueryDescription = GameplayTagQuery.IsEmpty() ? TEXT("None") : GameplayTagQuery.GetDescription();
	return FString::Printf(
		TEXT("%s\nTarget: %s\nWait Until: %s\nGameplay Tag Query:\n%s"),
		*Super::GetStaticDescription(),
		*TargetActorKey.SelectedKeyName.ToString(),
		WaitDescription,
		*QueryDescription);
}

UAbilitySystemComponent* UBertaBTTask_WaitTargetGameplayTagQuery::ResolveTargetAbilitySystemComponent(
	const UBlackboardComponent& Blackboard) const
{
	AActor* TargetActor = Cast<AActor>(Blackboard.GetValueAsObject(TargetActorKey.SelectedKeyName));
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
}

bool UBertaBTTask_WaitTargetGameplayTagQuery::IsWaitConditionSatisfied(const bool bQueryMatches) const
{
	return WaitCondition == EBertaGameplayTagQueryWaitCondition::Matches ? bQueryMatches : !bQueryMatches;
}

bool UBertaBTTask_WaitTargetGameplayTagQuery::IsCurrentTargetSatisfied(
	const UBlackboardComponent& Blackboard) const
{
	const UAbilitySystemComponent* AbilitySystemComponent = ResolveTargetAbilitySystemComponent(Blackboard);
	if (!AbilitySystemComponent)
	{
		return false;
	}

	FGameplayTagContainer OwnedTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);
	return IsWaitConditionSatisfied(GameplayTagQuery.Matches(OwnedTags));
}

void UBertaBTTask_WaitTargetGameplayTagQuery::BindTargetAbilitySystemComponent(UBlackboardComponent& Blackboard)
{
	UnbindTargetGameplayTagEvents();
	UAbilitySystemComponent* AbilitySystemComponent = ResolveTargetAbilitySystemComponent(Blackboard);
	if (!AbilitySystemComponent)
	{
		return;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
	const TArray<FGameplayTag>& QueryTags = GameplayTagQuery.GetGameplayTagArray();
	RegisteredGameplayTagEvents.Reserve(QueryTags.Num());
	for (const FGameplayTag& Tag : QueryTags)
	{
		if (!Tag.IsValid())
		{
			continue;
		}

		FRegisteredGameplayTagEvent& Registration = RegisteredGameplayTagEvents.AddDefaulted_GetRef();
		Registration.Tag = Tag;
		Registration.Handle = AbilitySystemComponent
			->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ThisClass::HandleGameplayTagChanged);
	}
}

void UBertaBTTask_WaitTargetGameplayTagQuery::CompleteIfSatisfied()
{
	if (!bIsWaiting)
	{
		return;
	}

	UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get();
	UBlackboardComponent* Blackboard = ObservedBlackboardComponent.Get();
	if (!BehaviorTreeComponent || !Blackboard || !IsCurrentTargetSatisfied(*Blackboard))
	{
		return;
	}

	UnregisterObservers();
	FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
}

void UBertaBTTask_WaitTargetGameplayTagQuery::HandleGameplayTagChanged(
	const FGameplayTag CallbackTag,
	const int32 NewCount)
{
	CompleteIfSatisfied();
}

EBlackboardNotificationResult UBertaBTTask_WaitTargetGameplayTagQuery::HandleTargetActorChanged(
	const UBlackboardComponent& Blackboard,
	const FBlackboard::FKey ChangedKeyId)
{
	if (!bIsWaiting || ChangedKeyId != TargetActorKey.GetSelectedKeyID())
	{
		return bIsWaiting
			? EBlackboardNotificationResult::ContinueObserving
			: EBlackboardNotificationResult::RemoveObserver;
	}

	UBlackboardComponent* MutableBlackboard = ObservedBlackboardComponent.Get();
	if (!MutableBlackboard || !ObservedBehaviorTreeComponent.IsValid())
	{
		UnbindTargetGameplayTagEvents();
		BlackboardObserverHandle.Reset();
		ObservedBlackboardComponent.Reset();
		ObservedBehaviorTreeComponent.Reset();
		bIsWaiting = false;
		return EBlackboardNotificationResult::RemoveObserver;
	}

	BindTargetAbilitySystemComponent(*MutableBlackboard);
	if (!IsCurrentTargetSatisfied(*MutableBlackboard))
	{
		return EBlackboardNotificationResult::ContinueObserving;
	}

	UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get();
	UnbindTargetGameplayTagEvents();
	BlackboardObserverHandle.Reset();
	ObservedBlackboardComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
	bIsWaiting = false;
	if (BehaviorTreeComponent)
	{
		FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
	}
	return EBlackboardNotificationResult::RemoveObserver;
}

void UBertaBTTask_WaitTargetGameplayTagQuery::UnbindTargetGameplayTagEvents()
{
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
		}
	}

	RegisteredGameplayTagEvents.Reset();
	ObservedAbilitySystemComponent.Reset();
}

void UBertaBTTask_WaitTargetGameplayTagQuery::UnregisterObservers()
{
	bIsWaiting = false;
	UnbindTargetGameplayTagEvents();
	if (UBlackboardComponent* Blackboard = ObservedBlackboardComponent.Get();
		Blackboard && BlackboardObserverHandle.IsValid() && TargetActorKey.IsSet())
	{
		Blackboard->UnregisterObserver(TargetActorKey.GetSelectedKeyID(), BlackboardObserverHandle);
	}

	BlackboardObserverHandle.Reset();
	ObservedBlackboardComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
}
