#include "AI/BertaBTTask_WaitTargetAttributeThreshold.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"

UBertaBTTask_WaitTargetAttributeThreshold::UBertaBTTask_WaitTargetAttributeThreshold(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Wait Target Attribute Threshold");
	bCreateNodeInstance = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey),
		AActor::StaticClass());
}

void UBertaBTTask_WaitTargetAttributeThreshold::InitializeFromAsset(UBehaviorTree& Asset)
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

EBTNodeResult::Type UBertaBTTask_WaitTargetAttributeThreshold::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterObservers();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard || !TargetActorKey.IsSet() || !Condition.Attribute.IsValid())
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

EBTNodeResult::Type UBertaBTTask_WaitTargetAttributeThreshold::AbortTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterObservers();
	return EBTNodeResult::Aborted;
}

void UBertaBTTask_WaitTargetAttributeThreshold::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	const EBTNodeResult::Type TaskResult)
{
	UnregisterObservers();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBertaBTTask_WaitTargetAttributeThreshold::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterObservers();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTTask_WaitTargetAttributeThreshold::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nTarget: %s\n%s"),
		*Super::GetStaticDescription(),
		*TargetActorKey.SelectedKeyName.ToString(),
		*Condition.ToString());
}

UAbilitySystemComponent* UBertaBTTask_WaitTargetAttributeThreshold::ResolveTargetAbilitySystemComponent(
	const UBlackboardComponent& Blackboard) const
{
	AActor* TargetActor = Cast<AActor>(Blackboard.GetValueAsObject(TargetActorKey.SelectedKeyName));
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
}

bool UBertaBTTask_WaitTargetAttributeThreshold::IsCurrentTargetSatisfied(
	const UBlackboardComponent& Blackboard) const
{
	const UAbilitySystemComponent* AbilitySystemComponent = ResolveTargetAbilitySystemComponent(Blackboard);
	if (!AbilitySystemComponent || !AbilitySystemComponent->HasAttributeSetForAttribute(Condition.Attribute))
	{
		return false;
	}

	bool bFound = false;
	const float Value = AbilitySystemComponent->GetGameplayAttributeValue(Condition.Attribute, bFound);
	return bFound && Condition.IsSatisfied(Value);
}

void UBertaBTTask_WaitTargetAttributeThreshold::BindTargetAbilitySystemComponent(
	UBlackboardComponent& Blackboard)
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

void UBertaBTTask_WaitTargetAttributeThreshold::CompleteIfSatisfied()
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

void UBertaBTTask_WaitTargetAttributeThreshold::HandleAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	CompleteIfSatisfied();
}

EBlackboardNotificationResult UBertaBTTask_WaitTargetAttributeThreshold::HandleTargetActorChanged(
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
		UnbindTargetAttributeEvent();
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
	UnbindTargetAttributeEvent();
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

void UBertaBTTask_WaitTargetAttributeThreshold::UnbindTargetAttributeEvent()
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

void UBertaBTTask_WaitTargetAttributeThreshold::UnregisterObservers()
{
	bIsWaiting = false;
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
