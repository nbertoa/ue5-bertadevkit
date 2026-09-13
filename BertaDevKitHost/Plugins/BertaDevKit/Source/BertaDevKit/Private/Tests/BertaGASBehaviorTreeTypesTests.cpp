#include "AI/BertaGASBehaviorTreeTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaNumericComparisonTest,
	"BertaDevKit.AI.GAS.NumericComparison",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaNumericComparisonTest::RunTest(const FString& Parameters)
{
	FBertaGameplayAttributeCondition Condition;
	Condition.Threshold = 10.0f;
	Condition.Tolerance = 0.1f;

	Condition.Comparison = EBertaNumericComparison::Less;
	TestTrue(TEXT("Less accepts a lower value"), Condition.IsSatisfied(9.0f));
	Condition.Comparison = EBertaNumericComparison::LessOrEqual;
	TestTrue(TEXT("LessOrEqual accepts equality"), Condition.IsSatisfied(10.0f));
	Condition.Comparison = EBertaNumericComparison::Equal;
	TestTrue(TEXT("Equal uses tolerance"), Condition.IsSatisfied(10.05f));
	Condition.Comparison = EBertaNumericComparison::NotEqual;
	TestFalse(TEXT("NotEqual uses the same tolerance"), Condition.IsSatisfied(10.05f));
	Condition.Comparison = EBertaNumericComparison::GreaterOrEqual;
	TestTrue(TEXT("GreaterOrEqual accepts equality"), Condition.IsSatisfied(10.0f));
	Condition.Comparison = EBertaNumericComparison::Greater;
	TestTrue(TEXT("Greater accepts a higher value"), Condition.IsSatisfied(11.0f));

	return true;
}

#endif
