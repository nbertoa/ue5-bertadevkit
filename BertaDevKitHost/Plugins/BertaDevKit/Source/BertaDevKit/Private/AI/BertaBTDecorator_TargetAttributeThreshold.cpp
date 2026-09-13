#include "AI/BertaBTDecorator_TargetAttributeThreshold.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"

UBertaBTDecorator_TargetAttributeThreshold::UBertaBTDecorator_TargetAttributeThreshold(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Target Attribute Threshold");
	bCreateNodeInstance = true;
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey),
		AActor::StaticClass());
}

void UBertaBTDecorator_TargetAttributeThreshold::InitializeFromAsset(UBehaviorTree& Asset)
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

bool UBertaBTDecorator_TargetAttributeThreshold::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const UAbilitySystemComponent* AbilitySystemComponent = Blackboard && Condition.Attribute.IsValid()
		? ResolveTargetAbilitySystemComponent(*Blackboard)
		: nullptr;
	return AbilitySystemComponent && IsConditionSatisfied(*AbilitySystemComponent);
}

void UBertaBTDecorator_TargetAttributeThreshold::OnBecomeRelevant(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UnregisterObservers();

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard || !TargetActorKey.IsSet() || !Condition.Attribute.IsValid())
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

void UBertaBTDecorator_TargetAttributeThreshold::OnCeaseRelevant(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterObservers();
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBertaBTDecorator_TargetAttributeThreshold::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterObservers();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTDecorator_TargetAttributeThreshold::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nTarget: %s\n%s"),
		*Super::GetStaticDescription(),
		*TargetActorKey.SelectedKeyName.ToString(),
		*Condition.ToString());
}

UAbilitySystemComponent* UBertaBTDecorator_TargetAttributeThreshold::ResolveTargetAbilitySystemComponent(
	const UBlackboardComponent& Blackboard) const
{
	AActor* TargetActor = Cast<AActor>(Blackboard.GetValueAsObject(TargetActorKey.SelectedKeyName));
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
}

bool UBertaBTDecorator_TargetAttributeThreshold::IsConditionSatisfied(
	const UAbilitySystemComponent& AbilitySystemComponent) const
{
	bool bFound = false;
	const float Value = AbilitySystemComponent.GetGameplayAttributeValue(Condition.Attribute, bFound);
	return bFound && Condition.IsSatisfied(Value);
}

void UBertaBTDecorator_TargetAttributeThreshold::BindTargetAbilitySystemComponent(UBlackboardComponent& Blackboard)
{
	UnbindTargetAttributeEvent();
	UAbilitySystemComponent* AbilitySystemComponent = ResolveTargetAbilitySystemComponent(Blackboard);
	if (!AbilitySystemComponent || !AbilitySystemComponent->HasAttributeSetForAttribute(Condition.Attribute))
	{
		return;
	}

	ObservedAbilitySystemComponent = AbilitySystemComponent;
	ObservedAttribute = Condition.Attribute;
	AttributeChangedDelegateHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(ObservedAttribute)
		.AddUObject(this, &ThisClass::HandleAttributeChanged);
}

void UBertaBTDecorator_TargetAttributeThreshold::HandleAttributeChanged(const FOnAttributeChangeData& ChangeData)
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

EBlackboardNotificationResult UBertaBTDecorator_TargetAttributeThreshold::HandleTargetActorChanged(
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
		UnbindTargetAttributeEvent();
		BlackboardObserverHandle.Reset();
		ObservedBlackboardComponent.Reset();
		ObservedBehaviorTreeComponent.Reset();
		return EBlackboardNotificationResult::RemoveObserver;
	}

	BindTargetAbilitySystemComponent(*MutableBlackboard);
	ConditionalFlowAbort(*BehaviorTreeComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBertaBTDecorator_TargetAttributeThreshold::UnbindTargetAttributeEvent()
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
}

void UBertaBTDecorator_TargetAttributeThreshold::UnregisterObservers()
{
	UnbindTargetAttributeEvent();
	if (UBlackboardComponent* Blackboard = ObservedBlackboardComponent.Get();
		Blackboard && BlackboardObserverHandle.IsValid() && TargetActorKey.IsSet())
	{
		Blackboard->UnregisterObserver(TargetActorKey.GetSelectedKeyID(), BlackboardObserverHandle);
	}

	BlackboardObserverHandle.Reset();
	ObservedBlackboardComponent.Reset();
	ObservedBehaviorTreeComponent.Reset();
}
