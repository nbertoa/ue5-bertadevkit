#pragma once

#include "GameplayTags/BertaGameplayTagUsageFinder.h"

namespace BertaGameplayTagUsagePrivate
{
	struct FSearchCounts
	{
		int32 CandidateAssets = 0;
		int32 ProcessedAssets = 0;
		int32 LoadedAssets = 0;
		int32 FailedAssets = 0;
		int32 SkippedAssets = 0;
		int32 Results = 0;
		bool bCanceled = false;
	};

	bool Matches(FGameplayTag StoredTag, FGameplayTag SearchTag, EBertaGameplayTagMatchMode MatchMode);
	bool IsProjectContentRoot(const FString& MountedRoot, const FString& LocalContentPath, const FString& ProjectDirectory);
	FString FormatSummary(const FString& TagName, EBertaGameplayTagMatchMode MatchMode, const FSearchCounts& Counts);
}
