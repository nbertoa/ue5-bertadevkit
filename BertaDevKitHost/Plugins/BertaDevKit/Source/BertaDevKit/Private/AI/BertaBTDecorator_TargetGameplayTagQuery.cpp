#include "AI/BertaBTDecorator_TargetGameplayTagQuery.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Actor.h"

UBertaBTDecorator_TargetGameplayTagQuery::UBertaBTDecorator_TargetGameplayTagQuery(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Target Gameplay Tag Query");
	bCreateNodeInstance = true;
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey),
		AActor::StaticClass());
}

void UBertaBTDecorator_TargetGameplayTagQuery::InitializeFromAsset(UBehaviorTree& Asset)
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

bool UBertaBTDecorator_TargetGameplayTagQuery::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	if (GameplayTagQuery.IsEmpty())
	{
		return false;
	}

	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const UAbilitySystemComponent* AbilitySystemComponent = Blackboard
		? ResolveTargetAbilitySystemComponent(*Blackboard)
		: nullptr;
	if (!AbilitySystemComponent)
	{
		return false;
	}

	FGameplayTagContainer OwnedTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);
	return GameplayTagQuery.Matches(OwnedTags);
}

void UBertaBTDecorator_TargetGameplayTagQuery::OnBecomeRelevant(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UnregisterObservers();

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard || !TargetActorKey.IsSet() || GameplayTagQuery.IsEmpty())
	{
		return;
	}

	ObservedBehaviorTreeComponent = &OwnerComp;
	ObservedBlackboardComponent = Blackboard;
	BlackboardObserverHandle = Blackboard->RegisterObserver(
		TargetActorKey.GetSelectedKeyID(),
		this,
		FOnBlackboardChangeNotification::CreateUObject(this, &ThisClass::HandleTargetActorChanged));
	BindTargetAbilitySystemComponent(*Blackboard);
}

void UBertaBTDecorator_TargetGameplayTagQuery::OnCeaseRelevant(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterObservers();
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBertaBTDecorator_TargetGameplayTagQuery::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterObservers();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTDecorator_TargetGameplayTagQuery::GetStaticDescription() const
{
	const FString QueryDescription = GameplayTagQuery.IsEmpty() ? TEXT("None") : GameplayTagQuery.GetDescription();
	return FString::Printf(
		TEXT("%s\nTarget: %s\nGameplay Tag Query:\n%s"),
		*Super::GetStaticDescription(),
		*TargetActorKey.SelectedKeyName.ToString(),
		*QueryDescription);
}

UAbilitySystemComponent* UBertaBTDecorator_TargetGameplayTagQuery::ResolveTargetAbilitySystemComponent(
	const UBlackboardComponent& Blackboard) const
{
	AActor* TargetActor = Cast<AActor>(Blackboard.GetValueAsObject(TargetActorKey.SelectedKeyName));
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
}

void UBertaBTDecorator_TargetGameplayTagQuery::BindTargetAbilitySystemComponent(UBlackboardComponent& Blackboard)
{
	UnbindTargetGameplayTagEvents();

	UAbilitySystemComponent* AbilitySystemComponent = ResolveTargetAbilitySystemComponent(Blackboard);
	const TArray<FGameplayTag>& QueryTags = GameplayTagQuery.GetGameplayTagArray();
	if (!AbilitySystemComponent || QueryTags.IsEmpty())
	{
		return;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
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

void UBertaBTDecorator_TargetGameplayTagQuery::HandleGameplayTagChanged(
	const FGameplayTag CallbackTag,
	const int32 NewCount)
{
	if (UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get())
	{
		ConditionalFlowAbort(*BehaviorTreeComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	}
	else
	{
		UnregisterObservers();
	}
}

EBlackboardNotificationResult UBertaBTDecorator_TargetGameplayTagQuery::HandleTargetActorChanged(
	const UBlackboardComponent& Blackboard,
	const FBlackboard::FKey ChangedKeyId)
{
	if (ChangedKeyId != TargetActorKey.GetSelectedKeyID())
	{
		return EBlackboardNotificationResult::ContinueObserving;
	}

	UBlackboardComponent* MutableBlackboard = ObservedBlackboardComponent.Get();
	UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get();
	if (!MutableBlackboard || !BehaviorTreeComponent)
	{
		UnbindTargetGameplayTagEvents();
		BlackboardObserverHandle.Reset();
		ObservedBlackboardComponent.Reset();
		ObservedBehaviorTreeComponent.Reset();
		return EBlackboardNotificationResult::RemoveObserver;
	}

	BindTargetAbilitySystemComponent(*MutableBlackboard);
	ConditionalFlowAbort(*BehaviorTreeComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBertaBTDecorator_TargetGameplayTagQuery::UnbindTargetGameplayTagEvents()
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

void UBertaBTDecorator_TargetGameplayTagQuery::UnregisterObservers()
{
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
