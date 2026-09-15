#pragma once

#include "DelegateInspector/BertaDelegateInspectorSnapshot.h"

class AActor;

namespace BertaDelegateInspector
{
	// A one-shot game-thread inspection. Returned data contains only copied values and weak UObject links.
	bool Inspect(AActor* Target, FSnapshot& OutSnapshot, FString& OutError);
	// Public membership queries reveal names, but not which repeated invocation owns which name.
	// NAME_None marks an occurrence whose exact function cannot be proven.
	TArray<FName> AssignFunctionOccurrences(int32 OccurrenceCount, TArray<FName> MatchingFunctions);
}
