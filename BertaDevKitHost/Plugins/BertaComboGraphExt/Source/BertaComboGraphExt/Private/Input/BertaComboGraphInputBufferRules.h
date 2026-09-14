#pragma once

namespace BertaComboGraphInputBufferRules
{
	inline int32 SelectOldestUnexpired(const TArray<float>& CaptureTimes, const float Now, const float Duration)
	{
		if (Duration <= 0.0f)
		{
			return INDEX_NONE;
		}
		for (int32 Index = 0; Index < CaptureTimes.Num(); ++Index)
		{
			if (Now - CaptureTimes[Index] <= Duration)
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}
}
