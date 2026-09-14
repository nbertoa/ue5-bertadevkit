#pragma once

#include "Abilities/BertaGSCAbilityReadiness.h"

namespace BertaGSCAbilityReadiness
{
	void Classify(FBertaGSCAbilityReadinessEntry& Entry);
	void SortEntries(TArray<FBertaGSCAbilityReadinessEntry>& Entries);
	FString StateText(const TArray<EBertaGSCAbilityReadinessState>& States);
}
