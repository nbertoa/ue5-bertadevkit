#include "BlueprintAudit/BertaBlueprintAuditMetrics.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaBlueprintAuditMetricsThresholdTest, "BertaDevKit.BlueprintAudit.MaintainabilityThresholds", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaBlueprintAuditMetricsThresholdTest::RunTest(const FString& Parameters)
{
	FBertaBlueprintGraphMetrics FunctionMetrics;
	FunctionMetrics.Kind = EBertaBlueprintAuditGraphKind::Function;
	FunctionMetrics.MeaningfulNodeCount = 49;
	TArray<EBertaBlueprintMaintainabilityReview> Reviews;
	BertaBlueprintAuditMetrics::EvaluateReviews(FunctionMetrics, Reviews);
	TestTrue(TEXT("Function below threshold has no size review"), Reviews.IsEmpty());

	FunctionMetrics.MeaningfulNodeCount = 50;
	BertaBlueprintAuditMetrics::EvaluateReviews(FunctionMetrics, Reviews);
	TestTrue(TEXT("Function at threshold has a large-function review"), Reviews.Contains(EBertaBlueprintMaintainabilityReview::LargeFunction));

	FBertaBlueprintGraphMetrics MacroMetrics;
	MacroMetrics.Kind = EBertaBlueprintAuditGraphKind::Macro;
	MacroMetrics.MeaningfulNodeCount = 40;
	BertaBlueprintAuditMetrics::EvaluateReviews(MacroMetrics, Reviews);
	TestTrue(TEXT("Macro at threshold has a large-macro review"), Reviews.Contains(EBertaBlueprintMaintainabilityReview::LargeMacro));

	FBertaBlueprintGraphMetrics EventMetrics;
	EventMetrics.Kind = EBertaBlueprintAuditGraphKind::EventGraph;
	EventMetrics.MeaningfulNodeCount = 75;
	EventMetrics.ConditionalDecisionCount = 12;
	BertaBlueprintAuditMetrics::EvaluateReviews(EventMetrics, Reviews);
	TestEqual(TEXT("Event graph threshold produces two maintainability reviews"), Reviews.Num(), 2);
	TestTrue(TEXT("Event graph at threshold has a large-event-graph review"), Reviews.Contains(EBertaBlueprintMaintainabilityReview::LargeEventGraph));
	TestTrue(TEXT("Twelve decisions has a high-decision-count review"), Reviews.Contains(EBertaBlueprintMaintainabilityReview::HighDecisionCount));
	return true;
}

#endif
