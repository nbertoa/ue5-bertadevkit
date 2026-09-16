#include "GameplayTags/BertaGameplayTagUsageFinder.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "GameplayTags/BertaGameplayTagUsageFinderInternal.h"
#include "GameplayTagsManager.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGameplayTagUsageFinderMatchingTest,
	"BertaDevKit.Editor.GameplayTags.UsageFinder.Matching",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGameplayTagUsageFinderMatchingTest::RunTest(const FString& Parameters)
{
	using namespace BertaGameplayTagUsagePrivate;
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	const auto GetOrAddTestTag = [&TagsManager](const FName TagName)
	{
		const FGameplayTag Existing = TagsManager.RequestGameplayTag(TagName, false);
		return Existing.IsValid() ? Existing : TagsManager.AddNativeGameplayTag(TagName, TEXT("Usage finder Matching automation test"));
	};

	// Test-only native tags remain in this Editor session; repeated runs reuse the same names.
	const FGameplayTag Parent = GetOrAddTestTag(TEXT("BertaDevKit.Tests.UsageFinder"));
	const FGameplayTag Child = GetOrAddTestTag(TEXT("BertaDevKit.Tests.UsageFinder.Child"));
	if (!TestTrue(TEXT("Matching test tags are registered"), Parent.IsValid() && Child.IsValid()))
	{
		return false;
	}
	TestTrue(TEXT("Exact tags match in Exact mode"), Matches(Parent, Parent, EBertaGameplayTagMatchMode::Exact));
	TestFalse(TEXT("Child does not match parent in Exact mode"), Matches(Child, Parent, EBertaGameplayTagMatchMode::Exact));
	TestTrue(TEXT("Child matches parent in ParentOrChild mode"), Matches(Child, Parent, EBertaGameplayTagMatchMode::ParentOrChild));
	TestTrue(TEXT("Parent matches child in ParentOrChild mode"), Matches(Parent, Child, EBertaGameplayTagMatchMode::ParentOrChild));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGameplayTagUsageFinderProjectRootTest,
	"BertaDevKit.Editor.GameplayTags.UsageFinder.ProjectRoots",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGameplayTagUsageFinderProjectRootTest::RunTest(const FString& Parameters)
{
	using namespace BertaGameplayTagUsagePrivate;
	TestTrue(TEXT("Game is always project content"), IsProjectContentRoot(TEXT("/Game/"), TEXT("X:/Project/Content"), TEXT("X:/Project")));
	TestTrue(TEXT("Project plugin content is included"), IsProjectContentRoot(TEXT("/ProjectPlugin/"), TEXT("X:/Project/Plugins/ProjectPlugin/Content"), TEXT("X:/Project")));
	TestTrue(TEXT("Project Game Feature content is included"), IsProjectContentRoot(TEXT("/Feature/"), TEXT("X:/Project/Plugins/GameFeatures/Feature/Content"), TEXT("X:/Project")));
	TestFalse(TEXT("Engine plugin content is excluded"), IsProjectContentRoot(TEXT("/EnginePlugin/"), TEXT("X:/Engine/Plugins/EnginePlugin/Content"), TEXT("X:/Project")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGameplayTagUsageFinderSummaryTest,
	"BertaDevKit.Editor.GameplayTags.UsageFinder.Summary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGameplayTagUsageFinderSummaryTest::RunTest(const FString& Parameters)
{
	BertaGameplayTagUsagePrivate::FSearchCounts Counts;
	Counts.CandidateAssets = 20;
	Counts.ProcessedAssets = 7;
	Counts.LoadedAssets = 5;
	Counts.FailedAssets = 1;
	Counts.SkippedAssets = 1;
	Counts.Results = 3;
	Counts.bCanceled = true;
	const FString Summary = BertaGameplayTagUsagePrivate::FormatSummary(
		TEXT("BertaDevKit.Tests.UsageFinder"),
		EBertaGameplayTagMatchMode::Exact,
		Counts);
	TestTrue(TEXT("Canceled summary is explicitly partial"), Summary.Contains(TEXT("Status=PartialCanceled")));
	TestTrue(TEXT("Summary contains candidate and processed counts"), Summary.Contains(TEXT("Candidates=20 Processed=7")));
	TestTrue(TEXT("Summary contains load failure and skip counts"), Summary.Contains(TEXT("LoadFailed=1 Skipped=1")));
	TestTrue(TEXT("Summary contains result count"), Summary.Contains(TEXT("Results=3")));

	Counts.bCanceled = false;
	const FString CompleteSummary = BertaGameplayTagUsagePrivate::FormatSummary(
		TEXT("BertaDevKit.Tests.UsageFinder"),
		EBertaGameplayTagMatchMode::Exact,
		Counts);
	TestTrue(TEXT("Load failures keep the summary partial"), CompleteSummary.Contains(TEXT("Status=PartialLoadFailures")));

	Counts.FailedAssets = 0;
	const FString SuccessfulSummary = BertaGameplayTagUsagePrivate::FormatSummary(
		TEXT("BertaDevKit.Tests.UsageFinder"),
		EBertaGameplayTagMatchMode::Exact,
		Counts);
	TestTrue(TEXT("Completed summary is explicit"), SuccessfulSummary.Contains(TEXT("Status=Complete")));
	return true;
}

#endif
