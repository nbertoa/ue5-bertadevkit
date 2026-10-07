#include "Audio/BertaRepeatedSoundHandle.h"

#include "Audio/BertaRepeatedSoundPolicy.h"
#include "AudioDevice.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundConcurrency.h"
#include "Templates/UnrealTemplate.h"
#include "TimerManager.h"

void UBertaRepeatedSoundHandle::Start(const UObject* Context)
{
	check(World.IsValid());
	RegisterWithGameInstance(Context);
	State = EState::Running;
	RemainingStarts = Options.RepeatCount;
	TearDownDelegate = FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &ThisClass::OnWorldTearDown);
	CleanupDelegate = FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::OnWorldCleanup);

	// Handles invalid weak components and destroyed attachment targets even without audio callbacks.
	FTimerManagerTimerParameters TimerParameters;
	TimerParameters.bLoop = true;
	TimerParameters.bMaxOncePerFrame = true;
	World->GetTimerManager().SetTimer(SweepTimer, this, &ThisClass::Sweep, 0.1f, TimerParameters);
	if (Options.bPlayImmediately)
	{
		StartNext();
	}
	else
	{
		ScheduleNext();
	}
}

bool UBertaRepeatedSoundHandle::HasValidTarget() const
{
	if (Mode != EMode::Attached)
	{
		return true;
	}
	const USceneComponent* Target = AttachTarget.Get();
	return Target && Target->GetWorld() == World.Get() && IsValid(Target->GetOwner())
		&& !Target->GetOwner()->IsActorBeingDestroyed();
}

void UBertaRepeatedSoundHandle::ScheduleNext()
{
	const float Delay = BertaRepeatedSoundPrivate::SampleNonNegative(Options.Interval, Options.IntervalVariance, FMath::FRand());
	World->GetTimerManager().SetTimer(NextStartTimer, this, &ThisClass::StartNext,
		BertaRepeatedSoundPrivate::TimerDelay(Delay), false);
}

void UBertaRepeatedSoundHandle::StartNext()
{
	NextStartTimer.Invalidate();
	if (State != EState::Running)
	{
		return;
	}
	if (!World.IsValid() || World->bIsTearingDown || !HasValidTarget())
	{
		Stop(0.0f);
		return;
	}
	check(RemainingStarts > 0);
	--RemainingStarts;
	if (RemainingStarts > 0)
	{
		ScheduleNext();
	}

	{
		// Play can synchronously report failure/completion. Do not release the session mid-start.
		TGuardValue<bool> StartingGuard(bStartingPlayback, true);
		if (UAudioComponent* Component = CreatePlayback())
		{
			const TWeakObjectPtr<UAudioComponent> WeakComponent(Component);
			FPlayback& Playback = Playbacks.AddDefaulted_GetRef();
			Playback.Component = Component;
			Playback.PlayStateDelegate = Component->OnAudioPlayStateChangedNative.AddUObject(this, &ThisClass::OnPlayStateChanged);
			if (Options.PlaybackDuration > 0.0f)
			{
				World->GetTimerManager().SetTimer(Playback.DeadlineTimer,
					FTimerDelegate::CreateUObject(this, &ThisClass::StopPlayback, WeakComponent), Options.PlaybackDuration, false);
				const float FadeLength = BertaRepeatedSoundPrivate::FadeOutLength(Options);
				const float FadeStart = Options.PlaybackDuration - FadeLength;
				if (FadeLength > 0.0f && FadeStart > 0.0f)
				{
					World->GetTimerManager().SetTimer(Playback.FadeTimer,
						FTimerDelegate::CreateUObject(this, &ThisClass::FadePlayback, WeakComponent, FadeLength), FadeStart, false);
				}
			}

			if (Options.bUseFadeIn && Options.FadeInDuration > 0.0f)
			{
				// FadeIn seeds the native active-sound fader at zero BEFORE the audio-thread enqueue.
				// Its target is relative to VolumeMultiplier, which already contains our sampled volume.
				Component->FadeIn(Options.FadeInDuration, 1.0f, 0.0f, EAudioFaderCurve::Linear);
			}
			else
			{
				Component->Play();
			}
			if (!Component->IsPlaying())
			{
				RemovePlayback(WeakComponent);
				if (IsValid(Component))
				{
					Component->DestroyComponent();
				}
			}
			else if (BertaRepeatedSoundPrivate::FadeOutLength(Options) > 0.0f
				&& Options.FadeOutDuration >= Options.PlaybackDuration)
			{
				FadePlayback(WeakComponent, Options.PlaybackDuration);
			}
		}
	}
	FinishIfIdle();
}

UAudioComponent* UBertaRepeatedSoundHandle::CreatePlayback() const
{
	UWorld* PlaybackWorld = World.Get();
	FAudioDevice* AudioDevice = PlaybackWorld->GetAudioDeviceRaw();
	if (!AudioDevice)
	{
		return nullptr;
	}
	UAudioComponent* Component = nullptr;
	if (Mode == EMode::TwoD)
	{
		Component = UGameplayStatics::CreateSound2D(PlaybackWorld, Sound, 1.0f, 1.0f, 0.0f, Concurrency, false, true);
	}
	else
	{
		USceneComponent* Target = AttachTarget.Get();
		FAudioDevice::FCreateComponentParams Params(PlaybackWorld, Target ? Target->GetOwner() : nullptr);
		Params.AttenuationSettings = Attenuation;
		Params.bPlay = false;
		Params.bStopWhenOwnerDestroyed = Mode == EMode::Attached;
		if (Concurrency)
		{
			Params.ConcurrencySet.Add(Concurrency);
		}
		const FVector SpawnLocation = Target
			? Target->GetSocketTransform(AttachPointName).TransformPosition(Location) : Location;
		Params.SetLocation(SpawnLocation);
		Component = FAudioDevice::CreateComponent(Sound, Params);
		if (Component)
		{
			Component->bAllowSpatialization = Params.ShouldUseAttenuation();
			Component->bIsUISound = !PlaybackWorld->IsGameWorld();
			Component->bAutoDestroy = true;
			Component->SubtitlePriority = Sound->GetSubtitlePriority();
			if (Target)
			{
				Component->AttachToComponent(Target, FAttachmentTransformRules::KeepRelativeTransform, AttachPointName);
				Component->SetRelativeLocationAndRotation(Location, Rotation);
			}
			else
			{
				Component->SetWorldLocationAndRotation(Location, Rotation);
			}
		}
	}
	if (Component)
	{
		Component->SetVolumeMultiplier(BertaRepeatedSoundPrivate::SampleNonNegative(
			Options.VolumeMultiplier, Options.VolumeVariance, FMath::FRand()));
		const float SampledPitch = BertaRepeatedSoundPrivate::SampleNonNegative(
			Options.PitchMultiplier, Options.PitchVariance, FMath::FRand());
		Component->SetPitchMultiplier(AudioDevice->ClampPitch(SampledPitch));
	}
	return Component;
}

void UBertaRepeatedSoundHandle::FadePlayback(TWeakObjectPtr<UAudioComponent> Component, float Duration)
{
	if (UAudioComponent* Audio = Component.Get())
	{
		Audio->FadeOut(Duration, 0.0f, EAudioFaderCurve::Linear);
	}
}

void UBertaRepeatedSoundHandle::StopPlayback(TWeakObjectPtr<UAudioComponent> Component)
{
	if (UAudioComponent* Audio = Component.Get())
	{
		Audio->Stop();
	}
	RemovePlayback(Component);
}

void UBertaRepeatedSoundHandle::OnPlayStateChanged(const UAudioComponent* Component, EAudioComponentPlayState PlayState)
{
	if (PlayState == EAudioComponentPlayState::Stopped)
	{
		// Unlike OnAudioFinishedNative, this also covers backend failures to start.
		RemovePlayback(const_cast<UAudioComponent*>(Component));
	}
}

void UBertaRepeatedSoundHandle::RemovePlayback(TWeakObjectPtr<UAudioComponent> Component)
{
	const int32 Index = Playbacks.IndexOfByPredicate([Component](const FPlayback& Playback) { return Playback.Component == Component; });
	if (Index == INDEX_NONE)
	{
		return;
	}
	FPlayback Playback = Playbacks[Index];
	Playbacks.RemoveAtSwap(Index);
	if (UAudioComponent* Audio = Playback.Component.Get())
	{
		Audio->OnAudioPlayStateChangedNative.Remove(Playback.PlayStateDelegate);
	}
	if (UWorld* PlaybackWorld = World.Get())
	{
		PlaybackWorld->GetTimerManager().ClearTimer(Playback.FadeTimer);
		PlaybackWorld->GetTimerManager().ClearTimer(Playback.DeadlineTimer);
	}
	FinishIfIdle();
}

void UBertaRepeatedSoundHandle::Sweep()
{
	if (!World.IsValid() || World->bIsTearingDown || !HasValidTarget())
	{
		Stop(0.0f);
		return;
	}
	const TArray<FPlayback> Snapshot = Playbacks;
	for (const FPlayback& Playback : Snapshot)
	{
		const UAudioComponent* Audio = Playback.Component.Get();
		if (!Audio || !Audio->IsPlaying())
		{
			RemovePlayback(Playback.Component);
		}
	}
	FinishIfIdle();
}

void UBertaRepeatedSoundHandle::Stop(float FadeDuration)
{
	if (State == EState::Finished)
	{
		return;
	}
	State = EState::Stopping;
	RemainingStarts = 0;
	UWorld* PlaybackWorld = World.Get();
	if (PlaybackWorld)
	{
		PlaybackWorld->GetTimerManager().ClearTimer(NextStartTimer);
	}
	const TArray<FPlayback> Snapshot = Playbacks;
	for (const FPlayback& Playback : Snapshot)
	{
		if (PlaybackWorld)
		{
			FTimerHandle FadeTimer = Playback.FadeTimer;
			FTimerHandle DeadlineTimer = Playback.DeadlineTimer;
			PlaybackWorld->GetTimerManager().ClearTimer(FadeTimer);
			PlaybackWorld->GetTimerManager().ClearTimer(DeadlineTimer);
		}
		UAudioComponent* Audio = Playback.Component.Get();
		if (Audio && Audio->IsPlaying() && PlaybackWorld && FadeDuration > 0.0f)
		{
			FadePlayback(Playback.Component, FadeDuration);
			// Native completion normally cleans up first; this is also a deterministic game-time stop bound.
			const int32 Index = Playbacks.IndexOfByPredicate([&Playback](const FPlayback& Entry) { return Entry.Component == Playback.Component; });
			if (Index != INDEX_NONE)
			{
				World->GetTimerManager().SetTimer(Playbacks[Index].DeadlineTimer,
					FTimerDelegate::CreateUObject(this, &ThisClass::StopPlayback, Playback.Component), FadeDuration, false);
			}
		}
		else
		{
			StopPlayback(Playback.Component);
		}
	}
	FinishIfIdle();
}

bool UBertaRepeatedSoundHandle::IsActive() const
{
	if (State == EState::Finished || !World.IsValid() || World->bIsTearingDown || !HasValidTarget())
	{
		return false;
	}
	return RemainingStarts > 0 || Playbacks.ContainsByPredicate([](const FPlayback& Playback)
	{
		const UAudioComponent* Audio = Playback.Component.Get();
		return Audio && Audio->IsPlaying();
	});
}

void UBertaRepeatedSoundHandle::FinishIfIdle()
{
	if (!bStartingPlayback && RemainingStarts == 0 && Playbacks.IsEmpty())
	{
		Finish();
	}
}

void UBertaRepeatedSoundHandle::Finish()
{
	if (State == EState::Finished)
	{
		return;
	}
	State = EState::Finished;
	if (UWorld* PlaybackWorld = World.Get())
	{
		PlaybackWorld->GetTimerManager().ClearAllTimersForObject(this);
	}
	FWorldDelegates::OnWorldBeginTearDown.Remove(TearDownDelegate);
	FWorldDelegates::OnWorldCleanup.Remove(CleanupDelegate);
	Sound = nullptr;
	Attenuation = nullptr;
	Concurrency = nullptr;
	AttachTarget.Reset();
	World.Reset();
	SetReadyToDestroy();
}

void UBertaRepeatedSoundHandle::OnWorldTearDown(UWorld* InWorld)
{
	if (InWorld == World.Get())
	{
		Stop(0.0f);
	}
}

void UBertaRepeatedSoundHandle::OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources)
{
	OnWorldTearDown(InWorld);
}
