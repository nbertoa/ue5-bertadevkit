#include "AI/BertaAIDebugUtils.h"

#include "AI/BertaBehaviorTreeDebugUtils.h"
#include "AI/BertaBlackboardDebugUtils.h"
#include "AI/BertaGASDebugUtils.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

bool UBertaAIDebugUtils::GetDebugSummary(AAIController* AIController, FString& OutSummary)
{
	OutSummary.Reset();
	if (!IsValid(AIController))
	{
		return false;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	TArray<FString> Sections;
	Sections.Reserve(4);
	Sections.Add(FString::Printf(
		TEXT("[AI]\nController: %s\nControlled Pawn: %s"),
		*AIController->GetPathName(),
		ControlledPawn ? *ControlledPawn->GetPathName() : TEXT("None")));

	FString SubsystemSummary;
	const UBehaviorTreeComponent* BehaviorTreeComponent =
		Cast<UBehaviorTreeComponent>(AIController->GetBrainComponent());
	Sections.Add(UBertaBehaviorTreeDebugUtils::GetDebugSummary(BehaviorTreeComponent, SubsystemSummary)
		? FString::Printf(TEXT("[Behavior Tree]\n%s"), *SubsystemSummary)
		: TEXT("[Behavior Tree]\nNone"));

	const UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
	Sections.Add(UBertaBlackboardDebugUtils::GetDebugSummary(Blackboard, SubsystemSummary)
		? FString::Printf(TEXT("[Blackboard]\n%s"), *SubsystemSummary)
		: TEXT("[Blackboard]\nNone"));

	Sections.Add(UBertaGASDebugUtils::GetDebugSummary(ControlledPawn, SubsystemSummary)
		? FString::Printf(TEXT("[GAS]\n%s"), *SubsystemSummary)
		: TEXT("[GAS]\nNo Ability System Component"));

	OutSummary = FString::Join(Sections, TEXT("\n\n"));
	return true;
}
