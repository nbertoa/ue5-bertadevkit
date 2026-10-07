#pragma once

#include "Audio/BertaAudioUtils.h"
#include "Math/UnrealMathUtility.h"

namespace BertaRepeatedSoundPrivate
{
	inline bool ValidateOptions(const FBertaRepeatedSoundOptions& Options)
	{
		const float NonNegative[] = {
			Options.Interval, Options.IntervalVariance, Options.VolumeMultiplier, Options.VolumeVariance,
			Options.PitchMultiplier, Options.PitchVariance, Options.FadeInDuration, Options.FadeOutDuration
		};
		for (const float Value : NonNegative)
		{
			if (!FMath::IsFinite(Value) || Value < 0.0f)
			{
				return false;
			}
		}
		return Options.RepeatCount > 0 && FMath::IsFinite(Options.PlaybackDuration);
	}

	// Double arithmetic avoids overflow when two otherwise finite float inputs are added.
	inline float SampleNonNegative(float Base, float Variance, float UnitSample)
	{
		const double Value = static_cast<double>(Base) + (2.0 * UnitSample - 1.0) * Variance;
		return static_cast<float>(FMath::Clamp(Value, 0.0, static_cast<double>(MAX_flt)));
	}

	inline float TimerDelay(float Delay)
	{
		// UE 5.8 SetTimerForNextTick can execute within the current timer tick by default.
		// A positive one-shot timer scheduled during Tick is pending until the next manager tick.
		return FMath::Max(Delay, UE_SMALL_NUMBER);
	}

	inline float FadeOutLength(const FBertaRepeatedSoundOptions& Options)
	{
		return Options.bUseFadeOut && Options.PlaybackDuration > 0.0f
			? FMath::Min(Options.FadeOutDuration, Options.PlaybackDuration) : 0.0f;
	}
}
