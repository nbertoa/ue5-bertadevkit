#include "AI/BertaBTTask_DebugTargetGASState.h"

#include "AI/BertaGASDebugUtils.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Debug/BertaDebugLog.h"
#include "GameFramework/Actor.h"

UBertaBTTask_DebugTargetGASState::UBertaBTTask_DebugTargetGASState(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Debug Target GAS State");
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey),
		AActor::StaticClass());
}

void UBertaBTTask_DebugTargetGASState::InitializeFromAsset(UBehaviorTree& Asset)
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

EBTNodeResult::Type UBertaBTTask_DebugTargetGASState::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Blackboard && TargetActorKey.IsSet()
		? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName))
		: nullptr;
	FString Summary;
	if (!UBertaGASDebugUtils::GetDebugSummary(TargetActor, Summary))
	{
		return EBTNodeResult::Failed;
	}

	if (Label.IsEmpty())
	{
		UE_LOG(LogBertaDebug, Log, TEXT("Target GAS Debug Summary\n%s"), *Summary);
	}
	else
	{
		UE_LOG(LogBertaDebug, Log, TEXT("Target GAS Debug Summary [%s]\n%s"), *Label, *Summary);
	}
	return EBTNodeResult::Succeeded;
}

FString UBertaBTTask_DebugTargetGASState::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nTarget: %s\nLabel: %s"),
		*Super::GetStaticDescription(),
		*TargetActorKey.SelectedKeyName.ToString(),
		Label.IsEmpty() ? TEXT("None") : *Label);
}
