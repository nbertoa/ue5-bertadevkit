#include "AI/BertaBTDecorator_GameplayTagQuery.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"

UBertaBTDecorator_GameplayTagQuery::UBertaBTDecorator_GameplayTagQuery(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Gameplay Tag Query");
	bCreateNodeInstance = true;
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
}

bool UBertaBTDecorator_GameplayTagQuery::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	if (GameplayTagQuery.IsEmpty())
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!AbilitySystemComponent)
	{
		return false;
	}

	FGameplayTagContainer OwnedGameplayTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedGameplayTags);
	return GameplayTagQuery.Matches(OwnedGameplayTags);
}

void UBertaBTDecorator_GameplayTagQuery::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	UnregisterGameplayTagEvents();

	if (GameplayTagQuery.IsEmpty())
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	const TArray<FGameplayTag>& QueryGameplayTags = GameplayTagQuery.GetGameplayTagArray();
	if (!AbilitySystemComponent || QueryGameplayTags.IsEmpty())
	{
		return;
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
		ObservedAbilitySystemComponent.Reset();
		ObservedBehaviorTreeComponent.Reset();
	}
}

void UBertaBTDecorator_GameplayTagQuery::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnregisterGameplayTagEvents();
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBertaBTDecorator_GameplayTagQuery::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterGameplayTagEvents();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTDecorator_GameplayTagQuery::GetStaticDescription() const
{
	const FString QueryDescription = GameplayTagQuery.IsEmpty() ? TEXT("None") : GameplayTagQuery.GetDescription();
	return FString::Printf(TEXT("%s\nGameplay Tag Query:\n%s"), *Super::GetStaticDescription(), *QueryDescription);
}

UAbilitySystemComponent* UBertaBTDecorator_GameplayTagQuery::ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

void UBertaBTDecorator_GameplayTagQuery::HandleGameplayTagChanged(const FGameplayTag CallbackTag, const int32 NewCount)
{
	if (UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get())
	{
		ConditionalFlowAbort(*BehaviorTreeComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	}
	else
	{
		UnregisterGameplayTagEvents();
	}
}

void UBertaBTDecorator_GameplayTagQuery::UnregisterGameplayTagEvents()
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

			Registration.Handle.Reset();
		}
	}

	RegisteredGameplayTagEvents.Reset();
	ObservedAbilitySystemComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
}
