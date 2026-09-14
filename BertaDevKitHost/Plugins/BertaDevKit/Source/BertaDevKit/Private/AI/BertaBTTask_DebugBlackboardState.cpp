#include "AI/BertaBTTask_DebugBlackboardState.h"

#include "AI/BertaBlackboardDebugUtils.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Debug/BertaDebugLog.h"

UBertaBTTask_DebugBlackboardState::UBertaBTTask_DebugBlackboardState(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Debug Blackboard State");
}

EBTNodeResult::Type UBertaBTTask_DebugBlackboardState::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	FString Summary;
	if (!UBertaBlackboardDebugUtils::GetDebugSummary(OwnerComp.GetBlackboardComponent(), Summary))
	{
		return EBTNodeResult::Failed;
	}

	if (Label.IsEmpty())
	{
		UE_LOG(LogBertaDebug, Log, TEXT("Blackboard Debug Summary\n%s"), *Summary);
	}
	else
	{
		UE_LOG(LogBertaDebug, Log, TEXT("Blackboard Debug Summary [%s]\n%s"), *Label, *Summary);
	}
	return EBTNodeResult::Succeeded;
}

FString UBertaBTTask_DebugBlackboardState::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nLabel: %s"),
		*Super::GetStaticDescription(),
		Label.IsEmpty() ? TEXT("None") : *Label);
}
