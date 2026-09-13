#include "AI/BertaBTDecorator_AbilityActive.h"

#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpec.h"

UBertaBTDecorator_AbilityActive::UBertaBTDecorator_AbilityActive(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Ability Active");
	bCreateNodeInstance = true;
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
}

bool UBertaBTDecorator_AbilityActive::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	if (!AbilityClass)
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent
		? AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass)
		: nullptr;
	return AbilitySpec && AbilitySpec->IsActive();
}

void UBertaBTDecorator_AbilityActive::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UnregisterAbilityEvents();

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent && AbilityClass
		? AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass)
		: nullptr;
	if (!AbilitySpec)
	{
		return;
	}

	AbilitySpecHandle = AbilitySpec->Handle;
	ObservedAbilitySystemComponent = AbilitySystemComponent;
	ObservedBehaviorTreeComponent = &OwnerComp;
	AbilityActivatedDelegateHandle = AbilitySystemComponent->AbilityActivatedCallbacks.AddUObject(
		this,
		&ThisClass::HandleAbilityActivated);
	AbilityEndedDelegateHandle = AbilitySystemComponent->OnAbilityEnded.AddUObject(
		this,
		&ThisClass::HandleAbilityEnded);
}

void UBertaBTDecorator_AbilityActive::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnregisterAbilityEvents();
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBertaBTDecorator_AbilityActive::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterAbilityEvents();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTDecorator_AbilityActive::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\nAbility: %s"), *Super::GetStaticDescription(), *GetNameSafe(AbilityClass));
}

UAbilitySystemComponent* UBertaBTDecorator_AbilityActive::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

void UBertaBTDecorator_AbilityActive::HandleAbilityActivated(UGameplayAbility* Ability)
{
	if (Ability && Ability->GetCurrentAbilitySpecHandle() == AbilitySpecHandle)
	{
		RequestConditionReevaluation();
	}
}

void UBertaBTDecorator_AbilityActive::HandleAbilityEnded(const FAbilityEndedData& EndedData)
{
	if (EndedData.AbilitySpecHandle == AbilitySpecHandle)
	{
		RequestConditionReevaluation();
	}
}

void UBertaBTDecorator_AbilityActive::RequestConditionReevaluation()
{
	if (UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get())
	{
		ConditionalFlowAbort(*BehaviorTreeComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	}
	else
	{
		UnregisterAbilityEvents();
	}
}

void UBertaBTDecorator_AbilityActive::UnregisterAbilityEvents()
{
	if (UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get())
	{
		if (AbilityActivatedDelegateHandle.IsValid())
		{
			AbilitySystemComponent->AbilityActivatedCallbacks.Remove(AbilityActivatedDelegateHandle);
		}
		if (AbilityEndedDelegateHandle.IsValid())
		{
			AbilitySystemComponent->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
		}
	}

	AbilityActivatedDelegateHandle.Reset();
	AbilityEndedDelegateHandle.Reset();
	AbilitySpecHandle = FGameplayAbilitySpecHandle();
	ObservedAbilitySystemComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
}
