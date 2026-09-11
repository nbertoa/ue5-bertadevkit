#include "AI/BertaBTDecorator_GameplayTag.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"

UBertaBTDecorator_GameplayTag::UBertaBTDecorator_GameplayTag(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Gameplay Tag");
	bCreateNodeInstance = true;
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
}

bool UBertaBTDecorator_GameplayTag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	return GameplayTag.IsValid()
		&& AbilitySystemComponent
		&& AbilitySystemComponent->HasMatchingGameplayTag(GameplayTag);
}

void UBertaBTDecorator_GameplayTag::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	UnregisterGameplayTagEvent();

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!GameplayTag.IsValid() || !AbilitySystemComponent)
	{
		return;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
	ObservedBehaviorTreeComponent = &OwnerComp;
	RegisteredGameplayTag = GameplayTag;
	GameplayTagEventHandle = AbilitySystemComponent
		->RegisterGameplayTagEvent(RegisteredGameplayTag, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ThisClass::HandleGameplayTagChanged);
}

void UBertaBTDecorator_GameplayTag::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnregisterGameplayTagEvent();
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBertaBTDecorator_GameplayTag::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterGameplayTagEvent();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTDecorator_GameplayTag::GetStaticDescription() const
{
	const FString TagDescription = GameplayTag.IsValid() ? GameplayTag.ToString() : TEXT("None");
	return FString::Printf(TEXT("%s\nGameplay Tag: %s"), *Super::GetStaticDescription(), *TagDescription);
}

UAbilitySystemComponent* UBertaBTDecorator_GameplayTag::ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

void UBertaBTDecorator_GameplayTag::HandleGameplayTagChanged(const FGameplayTag CallbackTag, const int32 NewCount)
{
	if (CallbackTag != RegisteredGameplayTag)
	{
		return;
	}

	if (UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get())
	{
		ConditionalFlowAbort(*BehaviorTreeComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	}
	else
	{
		UnregisterGameplayTagEvent();
	}
}

void UBertaBTDecorator_GameplayTag::UnregisterGameplayTagEvent()
{
	if (GameplayTagEventHandle.IsValid())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
			AbilitySystemComponent && RegisteredGameplayTag.IsValid())
		{
			AbilitySystemComponent->UnregisterGameplayTagEvent(
				GameplayTagEventHandle,
				RegisteredGameplayTag,
				EGameplayTagEventType::NewOrRemoved);
		}

		GameplayTagEventHandle.Reset();
	}

	ObservedAbilitySystemComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
	RegisteredGameplayTag = FGameplayTag();
}
