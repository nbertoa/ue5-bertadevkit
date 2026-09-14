#include "GameplayTags/BertaGameplayTagUsageFinder.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGameplayTagUsageFinderInvalidInputTest,
	"BertaDevKit.Editor.GameplayTags.UsageFinder.InvalidInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGameplayTagUsageFinderInvalidInputTest::RunTest(const FString& Parameters)
{
	TArray<FBertaGameplayTagUsageResult> Results;
	FString Summary;
	TestFalse(
		TEXT("Invalid tag fails explicitly"),
		UBertaGameplayTagUsageFinder::FindGameplayTagUsages(
			FGameplayTag(),
			EBertaGameplayTagUsageScope::WholeGame,
			EBertaGameplayTagMatchMode::Exact,
			TEXT(""),
			Results,
			Summary));
	TestTrue(TEXT("Failure explains invalid tag"), Summary.Contains(TEXT("invalid")));
	return true;
}

#endif
