#pragma once

#include "CoreMinimal.h"

namespace BertaComboGraphValidationRules
{
	bool HasCycle(const TMap<FString, TArray<FString>>& Adjacency);
	TSet<FString> ReachableFrom(const TMap<FString, TArray<FString>>& Adjacency, const FString& Root);
	bool HasDuplicateExactSignatures(const TArray<FString>& Signatures);
}
