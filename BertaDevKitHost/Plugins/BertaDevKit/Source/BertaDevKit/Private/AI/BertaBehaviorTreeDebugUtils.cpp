#include "AI/BertaBehaviorTreeDebugUtils.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BTNode.h"

bool UBertaBehaviorTreeDebugUtils::GetDebugSummary(
	const UBehaviorTreeComponent* BehaviorTreeComponent,
	FString& OutSummary)
{
	OutSummary.Reset();
	if (!IsValid(BehaviorTreeComponent))
	{
		return false;
	}

	const UBehaviorTree* RootTree = BehaviorTreeComponent->GetRootTree();
	const UBehaviorTree* CurrentTree = BehaviorTreeComponent->GetCurrentTree();
	const UBTNode* ActiveNode = BehaviorTreeComponent->GetActiveNode();

	TArray<FString> Lines;
	Lines.Reserve(9);
	Lines.Add(FString::Printf(TEXT("Component: %s"), *BehaviorTreeComponent->GetName()));
	Lines.Add(FString::Printf(TEXT("Root Tree: %s"), *GetNameSafe(RootTree)));
	Lines.Add(FString::Printf(TEXT("Current Tree: %s"), *GetNameSafe(CurrentTree)));
	Lines.Add(FString::Printf(
		TEXT("Running: %s"),
		BehaviorTreeComponent->IsRunning() ? TEXT("true") : TEXT("false")));
	Lines.Add(FString::Printf(
		TEXT("Paused: %s"),
		BehaviorTreeComponent->IsPaused() ? TEXT("true") : TEXT("false")));
	Lines.Add(FString::Printf(
		TEXT("Active Node: %s"),
		ActiveNode ? *ActiveNode->GetNodeName() : TEXT("None")));
	Lines.Add(FString::Printf(
		TEXT("Active Tasks: %s"),
		*BehaviorTreeComponent->DescribeActiveTasks()));
	Lines.Add(FString::Printf(
		TEXT("Active Trees: %s"),
		*BehaviorTreeComponent->DescribeActiveTrees()));

	FString ActivePath = BehaviorTreeComponent->GetDebugInfoString();
	ActivePath.TrimEndInline();
	Lines.Add(FString::Printf(
		TEXT("Active Path:\n%s"),
		ActivePath.IsEmpty() ? TEXT("None") : *ActivePath));

	OutSummary = FString::Join(Lines, TEXT("\n"));
	return true;
}
