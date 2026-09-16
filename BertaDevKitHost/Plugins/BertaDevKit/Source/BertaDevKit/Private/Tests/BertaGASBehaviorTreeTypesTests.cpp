#include "AI/BertaGASBehaviorTreeTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AI/BertaBTTask_ActivateGameplayAbilityAndWait.h"
#include "AI/BertaBTTask_WaitAbilityEnd.h"
#include "AI/BertaBTTask_WaitAbilityReady.h"
#include "AI/BertaBTTask_WaitAttributeThreshold.h"
#include "AI/BertaBTTask_WaitGameplayEffectApplied.h"
#include "AI/BertaBTTask_WaitGameplayEffectRemoved.h"
#include "AI/BertaBTTask_WaitGameplayEvent.h"
#include "AI/BertaBTTask_WaitGameplayTagQuery.h"
#include "AI/BertaBTTask_WaitTargetAttributeThreshold.h"
#include "AI/BertaBTTask_WaitTargetGameplayTagQuery.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaLatentGASTaskInstancingTest,
	"BertaDevKit.AI.GAS.LatentTasksRequireInstances",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaLatentGASTaskInstancingTest::RunTest(const FString& Parameters)
{
	// These tasks hold per-execution delegate/observer state in the node itself.
	TestTrue(TEXT("ActivateGameplayAbilityAndWait requires per-tree instances"), GetDefault<UBertaBTTask_ActivateGameplayAbilityAndWait>()->HasInstance());
	TestTrue(TEXT("WaitAbilityEnd requires per-tree instances"), GetDefault<UBertaBTTask_WaitAbilityEnd>()->HasInstance());
	TestTrue(TEXT("WaitAbilityReady requires per-tree instances"), GetDefault<UBertaBTTask_WaitAbilityReady>()->HasInstance());
	TestTrue(TEXT("WaitAttributeThreshold requires per-tree instances"), GetDefault<UBertaBTTask_WaitAttributeThreshold>()->HasInstance());
	TestTrue(TEXT("WaitGameplayEffectApplied requires per-tree instances"), GetDefault<UBertaBTTask_WaitGameplayEffectApplied>()->HasInstance());
	TestTrue(TEXT("WaitGameplayEffectRemoved requires per-tree instances"), GetDefault<UBertaBTTask_WaitGameplayEffectRemoved>()->HasInstance());
	TestTrue(TEXT("WaitGameplayEvent requires per-tree instances"), GetDefault<UBertaBTTask_WaitGameplayEvent>()->HasInstance());
	TestTrue(TEXT("WaitGameplayTagQuery requires per-tree instances"), GetDefault<UBertaBTTask_WaitGameplayTagQuery>()->HasInstance());
	TestTrue(TEXT("WaitTargetAttributeThreshold requires per-tree instances"), GetDefault<UBertaBTTask_WaitTargetAttributeThreshold>()->HasInstance());
	TestTrue(TEXT("WaitTargetGameplayTagQuery requires per-tree instances"), GetDefault<UBertaBTTask_WaitTargetGameplayTagQuery>()->HasInstance());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaNumericComparisonBoundariesTest,
	"BertaDevKit.AI.GAS.NumericComparisonBoundaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaNumericComparisonBoundariesTest::RunTest(const FString& Parameters)
{
	FBertaGameplayAttributeCondition Condition;
	Condition.Threshold = 10.0f;
	Condition.Tolerance = 0.25f;
	Condition.Comparison = EBertaNumericComparison::Less;
	TestFalse(TEXT("Less rejects equality"), Condition.IsSatisfied(10.0f));
	Condition.Comparison = EBertaNumericComparison::Greater;
	TestFalse(TEXT("Greater rejects equality"), Condition.IsSatisfied(10.0f));
	Condition.Comparison = EBertaNumericComparison::LessOrEqual;
	TestFalse(TEXT("LessOrEqual rejects values above the threshold"), Condition.IsSatisfied(10.25f));
	Condition.Comparison = EBertaNumericComparison::GreaterOrEqual;
	TestFalse(TEXT("GreaterOrEqual rejects values below the threshold"), Condition.IsSatisfied(9.75f));

	for (const float Value : { 9.75f, 10.25f, 9.5f, 10.5f })
	{
		const bool bWithinTolerance = Value == 9.75f || Value == 10.25f;
		Condition.Comparison = EBertaNumericComparison::Equal;
		TestEqual(TEXT("Equality includes both tolerance boundaries"), Condition.IsSatisfied(Value), bWithinTolerance);
		Condition.Comparison = EBertaNumericComparison::NotEqual;
		TestEqual(TEXT("Inequality is complementary at the tolerance boundaries"), Condition.IsSatisfied(Value), !bWithinTolerance);
	}

	Condition.Tolerance = -1.0f;
	Condition.Comparison = EBertaNumericComparison::Equal;
	TestTrue(TEXT("Negative tolerance still accepts exact equality"), Condition.IsSatisfied(10.0f));
	TestFalse(TEXT("Negative tolerance is clamped to zero"), Condition.IsSatisfied(10.125f));
	Condition.Comparison = static_cast<EBertaNumericComparison>(255);
	TestFalse(TEXT("Unknown comparisons fail closed"), Condition.IsSatisfied(10.0f));
	return true;
}

#endif
