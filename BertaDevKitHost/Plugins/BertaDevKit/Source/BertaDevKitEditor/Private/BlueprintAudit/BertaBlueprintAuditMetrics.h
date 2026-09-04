#pragma once

#include "CoreMinimal.h"

class UBlueprint;
class UEdGraph;

enum class EBertaBlueprintAuditGraphKind : uint8
{
	EventGraph,
	Function,
	Macro
};

enum class EBertaBlueprintMaintainabilityReview : uint8
{
	LargeFunction,
	LargeMacro,
	LargeEventGraph,
	HighDecisionCount
};

struct FBertaBlueprintGraphMetrics
{
	const UEdGraph* Graph = nullptr;
	EBertaBlueprintAuditGraphKind Kind = EBertaBlueprintAuditGraphKind::EventGraph;
	int32 MeaningfulNodeCount = 0;
	int32 ConditionalDecisionCount = 0;
};

namespace BertaBlueprintAuditMetrics
{
	void Collect(const UBlueprint& Blueprint, TArray<FBertaBlueprintGraphMetrics>& OutMetrics);
	void EvaluateReviews(const FBertaBlueprintGraphMetrics& Metrics, TArray<EBertaBlueprintMaintainabilityReview>& OutReviews);
	int32 GetSizeThreshold(EBertaBlueprintAuditGraphKind Kind);
	const TCHAR* GetGraphKindLabel(EBertaBlueprintAuditGraphKind Kind);
	const TCHAR* GetReviewLabel(EBertaBlueprintMaintainabilityReview Review);
}
