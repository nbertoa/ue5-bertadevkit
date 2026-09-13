#include "AI/BertaBTTask_WaitAttributeThreshold.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectTypes.h"

UBertaBTTask_WaitAttributeThreshold::UBertaBTTask_WaitAttributeThreshold(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Wait Attribute Threshold");
	bCreateNodeInstance = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBertaBTTask_WaitAttributeThreshold::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterAttributeEvent();

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!AbilitySystemComponent || !Condition.Attribute.IsValid() ||
		!AbilitySystemComponent->HasAttributeSetForAttribute(Condition.Attribute))
	{
		return EBTNodeResult::Failed;
	}

	if (IsConditionSatisfied(*AbilitySystemComponent))
	{
		return EBTNodeResult::Succeeded;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
	ObservedBehaviorTreeComponent = &OwnerComp;
	ObservedAttribute = Condition.Attribute;
	bIsWaiting = true;
	AttributeChangedDelegateHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(ObservedAttribute)
		.AddUObject(this, &ThisClass::HandleAttributeChanged);

	if (!AttributeChangedDelegateHandle.IsValid())
	{
		UnregisterAttributeEvent();
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBertaBTTask_WaitAttributeThreshold::AbortTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterAttributeEvent();
	return EBTNodeResult::Aborted;
}

void UBertaBTTask_WaitAttributeThreshold::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	const EBTNodeResult::Type TaskResult)
{
	UnregisterAttributeEvent();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBertaBTTask_WaitAttributeThreshold::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterAttributeEvent();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTTask_WaitAttributeThreshold::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\nWait Until: %s"), *Super::GetStaticDescription(), *Condition.ToString());
}

UAbilitySystemComponent* UBertaBTTask_WaitAttributeThreshold::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}

bool UBertaBTTask_WaitAttributeThreshold::IsConditionSatisfied(
	const UAbilitySystemComponent& AbilitySystemComponent) const
{
	bool bFound = false;
	const float Value = AbilitySystemComponent.GetGameplayAttributeValue(Condition.Attribute, bFound);
	return bFound && Condition.IsSatisfied(Value);
}

void UBertaBTTask_WaitAttributeThreshold::HandleAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	if (!bIsWaiting)
	{
		return;
	}

	UBehaviorTreeComponent* BehaviorTreeComponent = ObservedBehaviorTreeComponent.Get();
	UAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get();
	if (!BehaviorTreeComponent || !AbilitySystemComponent)
	{
		UnregisterAttributeEvent();
		return;
	}

	if (!IsConditionSatisfied(*AbilitySystemComponent))
	{
		return;
	}

	UnregisterAttributeEvent();
	FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
}

void UBertaBTTask_WaitAttributeThreshold::UnregisterAttributeEvent()
{
	bIsWaiting = false;

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
