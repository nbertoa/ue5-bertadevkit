#include "AI/BertaBTTask_DebugAIState.h"

#include "AI/BertaAIDebugUtils.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Debug/BertaDebugLog.h"

UBertaBTTask_DebugAIState::UBertaBTTask_DebugAIState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Debug AI State");
}

EBTNodeResult::Type UBertaBTTask_DebugAIState::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	FString Summary;
	if (!UBertaAIDebugUtils::GetDebugSummary(OwnerComp.GetAIOwner(), Summary))
	{
		return EBTNodeResult::Failed;
	}

	if (Label.IsEmpty())
	{
		UE_LOG(LogBertaDebug, Log, TEXT("AI Debug Summary\n%s"), *Summary);
	}
	else
	{
		UE_LOG(LogBertaDebug, Log, TEXT("AI Debug Summary [%s]\n%s"), *Label, *Summary);
	}
	return EBTNodeResult::Succeeded;
}

FString UBertaBTTask_DebugAIState::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nLabel: %s"),
		*Super::GetStaticDescription(),
		Label.IsEmpty() ? TEXT("None") : *Label);
}
