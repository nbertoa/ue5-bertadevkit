#include "BlueprintAudit/BertaBlueprintAuditMetrics.h"

#include "EdGraph/EdGraph.h"
#include "EdGraphNode_Comment.h"
#include "Engine/Blueprint.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Knot.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_Switch.h"
#include "K2Node_Tunnel.h"

namespace
{
	constexpr int32 LargeFunctionThreshold = 50;
	constexpr int32 LargeMacroThreshold = 40;
	constexpr int32 LargeEventGraphThreshold = 75;
	constexpr int32 HighDecisionCountThreshold = 12;

	bool IsMeaningfulImplementationNode(const UEdGraphNode* Node)
	{
		return Node
			&& !Node->IsA<UEdGraphNode_Comment>()
			&& !Node->IsA<UK2Node_Knot>()
			&& !Node->IsA<UK2Node_FunctionEntry>()
			&& !Node->IsA<UK2Node_FunctionResult>()
			// Macro instances are executable implementation nodes despite deriving from UK2Node_Tunnel.
			&& (!Node->IsA<UK2Node_Tunnel>() || Node->IsA<UK2Node_MacroInstance>());
	}

	void AddGraphMetrics(const UEdGraph* Graph, const EBertaBlueprintAuditGraphKind Kind, TSet<const UEdGraph*>& SeenGraphs, TArray<FBertaBlueprintGraphMetrics>& OutMetrics)
	{
		if (!Graph || SeenGraphs.Contains(Graph))
		{
			return;
		}

		SeenGraphs.Add(Graph);
		FBertaBlueprintGraphMetrics& Metrics = OutMetrics.AddDefaulted_GetRef();
		Metrics.Graph = Graph;
		Metrics.Kind = Kind;
		for (const UEdGraphNode* Node : Graph->Nodes)
		{
			if (IsMeaningfulImplementationNode(Node))
			{
				++Metrics.MeaningfulNodeCount;
			}
			if (Node && (Node->IsA<UK2Node_IfThenElse>() || Node->IsA<UK2Node_Switch>()))
			{
				++Metrics.ConditionalDecisionCount;
			}
		}
	}
}

void BertaBlueprintAuditMetrics::Collect(const UBlueprint& Blueprint, TArray<FBertaBlueprintGraphMetrics>& OutMetrics)
{
	OutMetrics.Reset();
	TSet<const UEdGraph*> SeenGraphs;
	for (const UEdGraph* Graph : Blueprint.UbergraphPages)
	{
		AddGraphMetrics(Graph, EBertaBlueprintAuditGraphKind::EventGraph, SeenGraphs, OutMetrics);
	}
	for (const UEdGraph* Graph : Blueprint.FunctionGraphs)
	{
		AddGraphMetrics(Graph, EBertaBlueprintAuditGraphKind::Function, SeenGraphs, OutMetrics);
	}
	for (const UEdGraph* Graph : Blueprint.MacroGraphs)
	{
		AddGraphMetrics(Graph, EBertaBlueprintAuditGraphKind::Macro, SeenGraphs, OutMetrics);
	}
}

void BertaBlueprintAuditMetrics::EvaluateReviews(const FBertaBlueprintGraphMetrics& Metrics, TArray<EBertaBlueprintMaintainabilityReview>& OutReviews)
{
	OutReviews.Reset();
	if (Metrics.MeaningfulNodeCount >= GetSizeThreshold(Metrics.Kind))
	{
		switch (Metrics.Kind)
		{
		case EBertaBlueprintAuditGraphKind::Function: OutReviews.Add(EBertaBlueprintMaintainabilityReview::LargeFunction); break;
		case EBertaBlueprintAuditGraphKind::Macro: OutReviews.Add(EBertaBlueprintMaintainabilityReview::LargeMacro); break;
		case EBertaBlueprintAuditGraphKind::EventGraph: OutReviews.Add(EBertaBlueprintMaintainabilityReview::LargeEventGraph); break;
		default: break;
		}
	}
	if (Metrics.ConditionalDecisionCount >= HighDecisionCountThreshold)
	{
		OutReviews.Add(EBertaBlueprintMaintainabilityReview::HighDecisionCount);
	}
}

int32 BertaBlueprintAuditMetrics::GetSizeThreshold(const EBertaBlueprintAuditGraphKind Kind)
{
	switch (Kind)
	{
	case EBertaBlueprintAuditGraphKind::Function: return LargeFunctionThreshold;
	case EBertaBlueprintAuditGraphKind::Macro: return LargeMacroThreshold;
	case EBertaBlueprintAuditGraphKind::EventGraph: return LargeEventGraphThreshold;
	default: return LargeEventGraphThreshold;
	}
}

const TCHAR* BertaBlueprintAuditMetrics::GetGraphKindLabel(const EBertaBlueprintAuditGraphKind Kind)
{
	switch (Kind)
	{
	case EBertaBlueprintAuditGraphKind::Function: return TEXT("Function");
	case EBertaBlueprintAuditGraphKind::Macro: return TEXT("Macro");
	case EBertaBlueprintAuditGraphKind::EventGraph: return TEXT("Event Graph");
	default: return TEXT("Graph");
	}
}

const TCHAR* BertaBlueprintAuditMetrics::GetReviewLabel(const EBertaBlueprintMaintainabilityReview Review)
{
	switch (Review)
	{
	case EBertaBlueprintMaintainabilityReview::LargeFunction: return TEXT("Review: Large Function");
	case EBertaBlueprintMaintainabilityReview::LargeMacro: return TEXT("Review: Large Macro");
	case EBertaBlueprintMaintainabilityReview::LargeEventGraph: return TEXT("Review: Large Event Graph");
	case EBertaBlueprintMaintainabilityReview::HighDecisionCount: return TEXT("Review: High Decision Count");
	default: return TEXT("Review");
	}
}
