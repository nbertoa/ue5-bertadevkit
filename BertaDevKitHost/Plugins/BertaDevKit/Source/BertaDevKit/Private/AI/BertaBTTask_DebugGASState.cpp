#include "AI/BertaBTTask_DebugGASState.h"

#include "AI/BertaGASDebugUtils.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Debug/BertaDebugLog.h"
#include "GameFramework/Pawn.h"

UBertaBTTask_DebugGASState::UBertaBTTask_DebugGASState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Debug GAS State");
}

EBTNodeResult::Type UBertaBTTask_DebugGASState::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	FString Summary;
	if (!UBertaGASDebugUtils::GetDebugSummary(ControlledPawn, Summary))
	{
		return EBTNodeResult::Failed;
	}

	if (Label.IsEmpty())
	{
		UE_LOG(LogBertaDebug, Log, TEXT("GAS Debug Summary\n%s"), *Summary);
	}
	else
	{
		UE_LOG(LogBertaDebug, Log, TEXT("GAS Debug Summary [%s]\n%s"), *Label, *Summary);
	}

	return EBTNodeResult::Succeeded;
}

FString UBertaBTTask_DebugGASState::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nLabel: %s"),
		*Super::GetStaticDescription(),
		Label.IsEmpty() ? TEXT("None") : *Label);
}
