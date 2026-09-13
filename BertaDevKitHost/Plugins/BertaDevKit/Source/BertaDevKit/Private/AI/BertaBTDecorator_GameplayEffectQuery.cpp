#include "AI/BertaBTDecorator_GameplayEffectQuery.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ActiveGameplayEffectHandle.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectTypes.h"

UBertaBTDecorator_GameplayEffectQuery::UBertaBTDecorator_GameplayEffectQuery(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Gameplay Effect Query");
	bCreateNodeInstance = true;
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
}

bool UBertaBTDecorator_GameplayEffectQuery::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	if (GameplayEffectQuery.IsEmpty())
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	return AbilitySystemComponent && !AbilitySystemComponent->GetActiveEffects(GameplayEffectQuery).IsEmpty();
}

void UBertaBTDecorator_GameplayEffectQuery::OnBecomeRelevant(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UnregisterGameplayEffectEvents();
	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!AbilitySystemComponent || GameplayEffectQuery.IsEmpty())
	{
		return;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
	ObservedBehaviorTreeComponent = &OwnerComp;
	GameplayEffectAddedDelegateHandle = AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(
		this,
		&ThisClass::HandleGameplayEffectAdded);
	GameplayEffectRemovedDelegateHandle = AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(
		this,
		&ThisClass::HandleGameplayEffectRemoved);
}

void UBertaBTDecorator_GameplayEffectQuery::OnCeaseRelevant(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterGameplayEffectEvents();
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBertaBTDecorator_GameplayEffectQuery::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterGameplayEffectEvents();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTDecorator_GameplayEffectQuery::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nGameplay Effect Query: %s"),
		*Super::GetStaticDescription(),
		GameplayEffectQuery.IsEmpty() ? TEXT("None") : TEXT("Configured"));
}

UAbilitySystemComponent* UBertaBTDecorator_GameplayEffectQuery::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

void UBertaBTDecorator_GameplayEffectQuery::HandleGameplayEffectAdded(
	UAbilitySystemComponent* Target,
	const FGameplayEffectSpec& AppliedSpec,
	const FActiveGameplayEffectHandle ActiveHandle)
{
	RequestConditionReevaluation();
}

void UBertaBTDecorator_GameplayEffectQuery::HandleGameplayEffectRemoved(
	const FActiveGameplayEffect& RemovedEffect)
{
	RequestConditionReevaluation();
}

void UBertaBTDecorator_GameplayEffectQuery::RequestConditionReevaluation()
{
	if (UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get())
	{
		ConditionalFlowAbort(*BehaviorTreeComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	}
	else
	{
		UnregisterGameplayEffectEvents();
	}
}

void UBertaBTDecorator_GameplayEffectQuery::UnregisterGameplayEffectEvents()
{
	if (UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get())
	{
		if (GameplayEffectAddedDelegateHandle.IsValid())
		{
			AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.Remove(
				GameplayEffectAddedDelegateHandle);
		}
		if (GameplayEffectRemovedDelegateHandle.IsValid())
		{
			AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().Remove(
				GameplayEffectRemovedDelegateHandle);
		}
	}

	GameplayEffectAddedDelegateHandle.Reset();
	GameplayEffectRemovedDelegateHandle.Reset();
	ObservedAbilitySystemComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
}
