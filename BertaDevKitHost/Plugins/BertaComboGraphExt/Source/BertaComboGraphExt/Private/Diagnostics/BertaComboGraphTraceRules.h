#pragma once

#include "Diagnostics/BertaComboGraphTraceComponent.h"

namespace BertaComboGraphTraceRules
{
	inline void AppendBounded(TArray<FBertaComboGraphTraceEvent>& Events, FBertaComboGraphTraceEvent&& Event, const int32 MaximumEventCount)
	{
		const int32 Limit = FMath::Clamp(MaximumEventCount, 1, 10000);
		if (Events.Num() >= Limit)
		{
			Events.RemoveAt(0, Events.Num() - Limit + 1, EAllowShrinking::No);
		}
		Events.Add(MoveTemp(Event));
	}
}
