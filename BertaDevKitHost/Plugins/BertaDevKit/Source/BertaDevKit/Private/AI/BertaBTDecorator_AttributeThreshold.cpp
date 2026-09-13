#include "AI/BertaBTDecorator_AttributeThreshold.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectTypes.h"

UBertaBTDecorator_AttributeThreshold::UBertaBTDecorator_AttributeThreshold(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Attribute Threshold");
	bCreateNodeInstance = true;
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
}

bool UBertaBTDecorator_AttributeThreshold::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	if (!Condition.Attribute.IsValid())
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!AbilitySystemComponent)
	{
		return false;
	}

	bool bFound = false;
	const float Value = AbilitySystemComponent->GetGameplayAttributeValue(Condition.Attribute, bFound);
	return bFound && Condition.IsSatisfied(Value);
}

void UBertaBTDecorator_AttributeThreshold::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UnregisterAttributeEvent();

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!AbilitySystemComponent || !Condition.Attribute.IsValid() ||
		!AbilitySystemComponent->HasAttributeSetForAttribute(Condition.Attribute))
	{
		return;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
	ObservedBehaviorTreeComponent = &OwnerComp;
	ObservedAttribute = Condition.Attribute;
	AttributeChangedDelegateHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(ObservedAttribute)
		.AddUObject(this, &ThisClass::HandleAttributeChanged);
}

void UBertaBTDecorator_AttributeThreshold::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnregisterAttributeEvent();
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBertaBTDecorator_AttributeThreshold::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterAttributeEvent();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTDecorator_AttributeThreshold::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\n%s"), *Super::GetStaticDescription(), *Condition.ToString());
}

UAbilitySystemComponent* UBertaBTDecorator_AttributeThreshold::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

void UBertaBTDecorator_AttributeThreshold::HandleAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	if (UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get())
	{
		ConditionalFlowAbort(*BehaviorTreeComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	}
	else
	{
		UnregisterAttributeEvent();
	}
}

void UBertaBTDecorator_AttributeThreshold::UnregisterAttributeEvent()
{
	if (UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
		AbilitySystemComponent && AttributeChangedDelegateHandle.IsValid() && ObservedAttribute.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(ObservedAttribute)
			.Remove(AttributeChangedDelegateHandle);
	}

	AttributeChangedDelegateHandle.Reset();
	ObservedAttribute = FGameplayAttribute();
	ObservedAbilitySystemComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
}
