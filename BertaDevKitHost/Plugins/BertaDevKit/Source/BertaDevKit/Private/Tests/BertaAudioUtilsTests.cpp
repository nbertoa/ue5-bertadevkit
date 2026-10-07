#include "Audio/BertaAudioUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Audio/BertaRepeatedSoundHandle.h"
#include "Audio/BertaRepeatedSoundPolicy.h"
#include "AudioDevice.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "CoreGlobals.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Sound/SoundWave.h"
#include "Templates/UnrealTemplate.h"
#include "TimerManager.h"
#include "UObject/GarbageCollection.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include <limits>

// Exercises the real handle and TimerManager without an audio device or audible playback.
struct FBertaRepeatedSoundTestAccess
{
	static UBertaRepeatedSoundHandle* Start(UWorld* World, FBertaRepeatedSoundOptions Options, USceneComponent* Target = nullptr)
	{
		UBertaRepeatedSoundHandle* Handle = NewObject<UBertaRepeatedSoundHandle>();
		Handle->World = World;
		Handle->Sound = NewObject<USoundWave>();
		Handle->Options = Options;
		Handle->AttachTarget = Target;
		Handle->Mode = Target ? UBertaRepeatedSoundHandle::EMode::Attached : UBertaRepeatedSoundHandle::EMode::TwoD;
		Handle->Start(World);
		return Handle;
	}
	static int32 Remaining(const UBertaRepeatedSoundHandle* Handle) { return Handle->RemainingStarts; }
	static bool Registered(const UBertaRepeatedSoundHandle* Handle) { return Handle->RegisteredWithGameInstance.IsValid(); }
	static void Track(UBertaRepeatedSoundHandle* Handle, UAudioComponent* Component)
	{
		UBertaRepeatedSoundHandle::FPlayback& Playback = Handle->Playbacks.AddDefaulted_GetRef();
		Playback.Component = Component;
		Playback.PlayStateDelegate = Component->OnAudioPlayStateChangedNative.AddUObject(Handle, &UBertaRepeatedSoundHandle::OnPlayStateChanged);
		const TWeakObjectPtr<UAudioComponent> WeakComponent(Component);
		Handle->World->GetTimerManager().SetTimer(Playback.FadeTimer,
			FTimerDelegate::CreateUObject(Handle, &UBertaRepeatedSoundHandle::FadePlayback, WeakComponent, 0.2f), 0.8f, false);
		Handle->World->GetTimerManager().SetTimer(Playback.DeadlineTimer,
			FTimerDelegate::CreateUObject(Handle, &UBertaRepeatedSoundHandle::StopPlayback, WeakComponent), 1.0f, false);
	}
	static void ClearSweep(UBertaRepeatedSoundHandle* Handle) { Handle->World->GetTimerManager().ClearTimer(Handle->SweepTimer); }
	static int32 Tracked(const UBertaRepeatedSoundHandle* Handle) { return Handle->Playbacks.Num(); }
	static FTimerHandle Deadline(const UBertaRepeatedSoundHandle* Handle) { return Handle->Playbacks[0].DeadlineTimer; }
	static FTimerHandle Fade(const UBertaRepeatedSoundHandle* Handle) { return Handle->Playbacks[0].FadeTimer; }
};

namespace
{
	struct FAudioTimerWorld
	{
		FAudioTimerWorld()
		{
			const UWorld::InitializationValues Values = UWorld::InitializationValues()
				.CreatePhysicsScene(false).CreateNavigation(false).CreateAISystem(false)
				.ShouldSimulatePhysics(false).EnableTraceCollision(false);
			World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
			GameInstance.Reset(NewObject<UGameInstance>());
			World->SetGameInstance(GameInstance.Get());
			World->bAllowAudioPlayback = false;
			World->GetTimerManager().Tick(0.0f);
		}
		~FAudioTimerWorld() { World->DestroyWorld(false); }
		void Tick(float Delta)
		{
			++GFrameCounter;
			World->GetTimerManager().Tick(Delta);
		}
		TGuardValue<uint64> FrameGuard { GFrameCounter, GFrameCounter };
		TStrongObjectPtr<UGameInstance> GameInstance;
		UWorld* World = nullptr;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaRepeatedSoundPolicyTest, "BertaDevKit.Audio.RepeatedSound.Policy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaRepeatedSoundPolicyTest::RunTest(const FString& Parameters)
{
	using namespace BertaRepeatedSoundPrivate;
	FBertaRepeatedSoundOptions Options;
	TestTrue(TEXT("Defaults are valid"), ValidateOptions(Options));
	for (const float Unit : { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f })
	{
		const float Gap = SampleNonNegative(0.5f, 0.1f, Unit);
		TestTrue(TEXT("Every independent gap is in its specified range"), Gap >= 0.4f && Gap <= 0.6f);
		const float Volume = SampleNonNegative(0.2f, 0.5f, Unit);
		TestTrue(TEXT("Volume variation cannot go negative"), Volume >= 0.0f && Volume <= 0.7f);
	}
	TestEqual(TEXT("Variance lower endpoint"), SampleNonNegative(0.5f, 0.1f, 0.0f), 0.4f);
	TestEqual(TEXT("Variance upper endpoint"), SampleNonNegative(0.5f, 0.1f, 1.0f), 0.6f);
	TestTrue(TEXT("Finite extreme samples remain finite"), FMath::IsFinite(SampleNonNegative(MAX_flt, MAX_flt, 1.0f)));
	TestTrue(TEXT("A zero gap schedules a positive delay"), TimerDelay(0.0f) > 0.0f);
	if (FAudioDevice* Device = GEngine ? GEngine->GetMainAudioDeviceRaw() : nullptr)
	{
		const TRange<float> PitchRange = Device->GetGlobalPitchRange();
		for (const float Unit : { 0.0f, 0.5f, 1.0f })
		{
			const float Pitch = Device->ClampPitch(SampleNonNegative(1.0f, 10.0f, Unit));
			TestTrue(TEXT("Pitch uses the configured native positive range"), Pitch > 0.0f && Pitch >= PitchRange.GetLowerBoundValue() && Pitch <= PitchRange.GetUpperBoundValue());
		}
	}
	Options.PlaybackDuration = 1.0f;
	Options.bUseFadeOut = true;
	Options.FadeOutDuration = 0.2f;
	TestEqual(TEXT("Fade starts before the deadline"), Options.PlaybackDuration - FadeOutLength(Options), 0.8f);
	Options.FadeOutDuration = 2.0f;
	TestEqual(TEXT("Long fade is constrained to the lifetime"), FadeOutLength(Options), 1.0f);
	Options.FadeOutDuration = 0.0f;
	TestEqual(TEXT("Zero fade is immediate"), FadeOutLength(Options), 0.0f);
	Options.PlaybackDuration = 0.0f;
	TestEqual(TEXT("No inferred deadline/fade for natural playback"), FadeOutLength(Options), 0.0f);
	Options.PlaybackDuration = -1.0f;
	TestTrue(TEXT("Negative playback duration means natural playback"), ValidateOptions(Options));

	float FBertaRepeatedSoundOptions::* Fields[] = {
		&FBertaRepeatedSoundOptions::Interval, &FBertaRepeatedSoundOptions::IntervalVariance,
		&FBertaRepeatedSoundOptions::VolumeMultiplier, &FBertaRepeatedSoundOptions::VolumeVariance,
		&FBertaRepeatedSoundOptions::PitchMultiplier, &FBertaRepeatedSoundOptions::PitchVariance,
		&FBertaRepeatedSoundOptions::FadeInDuration, &FBertaRepeatedSoundOptions::FadeOutDuration,
		&FBertaRepeatedSoundOptions::PlaybackDuration
	};
	for (auto Field : Fields)
	{
		FBertaRepeatedSoundOptions Invalid;
		Invalid.*Field = std::numeric_limits<float>::quiet_NaN();
		TestFalse(TEXT("Every public float rejects NaN"), ValidateOptions(Invalid));
		Invalid.*Field = std::numeric_limits<float>::infinity();
		TestFalse(TEXT("Every public float rejects infinity"), ValidateOptions(Invalid));
	}
	Options = {};
	Options.IntervalVariance = -0.1f;
	TestFalse(TEXT("Negative variance is rejected"), ValidateOptions(Options));
	Options = {};
	Options.RepeatCount = 0;
	TestFalse(TEXT("Zero repeats is invalid"), ValidateOptions(Options));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaRepeatedSoundSchedulingTest, "BertaDevKit.Audio.RepeatedSound.SchedulingAndLifetime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaRepeatedSoundSchedulingTest::RunTest(const FString& Parameters)
{
	FAudioTimerWorld Scope;
	FBertaRepeatedSoundOptions Options;
	Options.RepeatCount = 3;
	TStrongObjectPtr<UBertaRepeatedSoundHandle> Immediate(FBertaRepeatedSoundTestAccess::Start(Scope.World, Options));
	TestEqual(TEXT("Immediate playback consumes exactly one start"), FBertaRepeatedSoundTestAccess::Remaining(Immediate.Get()), 2);
	Scope.Tick(0.49f);
	TestEqual(TEXT("No start before its interval"), FBertaRepeatedSoundTestAccess::Remaining(Immediate.Get()), 2);
	Scope.Tick(0.02f);
	TestEqual(TEXT("Second start does not wait for sound completion"), FBertaRepeatedSoundTestAccess::Remaining(Immediate.Get()), 1);
	Scope.Tick(0.51f);
	TestEqual(TEXT("RepeatCount is total starts"), FBertaRepeatedSoundTestAccess::Remaining(Immediate.Get()), 0);
	TestFalse(TEXT("Rejected audio starts still release the session"), UBertaAudioUtils::IsRepeatedSoundActive(Immediate.Get()));
	TestFalse(TEXT("Completion unregisters the handle"), FBertaRepeatedSoundTestAccess::Registered(Immediate.Get()));

	Options.bPlayImmediately = false;
	TStrongObjectPtr<UBertaRepeatedSoundHandle> Delayed(FBertaRepeatedSoundTestAccess::Start(Scope.World, Options));
	TestEqual(TEXT("Delayed first start waits a generated interval"), FBertaRepeatedSoundTestAccess::Remaining(Delayed.Get()), 3);
	Scope.Tick(0.51f);
	TestEqual(TEXT("Delayed first playback consumes one start"), FBertaRepeatedSoundTestAccess::Remaining(Delayed.Get()), 2);
	UBertaAudioUtils::StopRepeatedSound(Delayed.Get());
	UBertaAudioUtils::StopRepeatedSound(Delayed.Get(), 0.2f);
	Scope.Tick(2.0f);
	TestEqual(TEXT("Stop cancels every future start"), FBertaRepeatedSoundTestAccess::Remaining(Delayed.Get()), 0);
	TestFalse(TEXT("Stop is terminal and idempotent"), UBertaAudioUtils::IsRepeatedSoundActive(Delayed.Get()));
	UBertaAudioUtils::StopRepeatedSound(nullptr);
	TestFalse(TEXT("Null handle is inactive"), UBertaAudioUtils::IsRepeatedSoundActive(nullptr));

	Options.Interval = 0.0f;
	Options.RepeatCount = 100;
	TStrongObjectPtr<UBertaRepeatedSoundHandle> Zero(FBertaRepeatedSoundTestAccess::Start(Scope.World, Options));
	Scope.Tick(1.0f);
	TestEqual(TEXT("Zero interval cannot drain repetitions in one timer tick"), FBertaRepeatedSoundTestAccess::Remaining(Zero.Get()), 99);
	Scope.Tick(1.0f);
	TestEqual(TEXT("Zero interval advances once on the following tick"), FBertaRepeatedSoundTestAccess::Remaining(Zero.Get()), 98);
	UBertaAudioUtils::StopRepeatedSound(Zero.Get());

	Options.Interval = 60.0f;
	TWeakObjectPtr<UBertaRepeatedSoundHandle> Ignored(FBertaRepeatedSoundTestAccess::Start(Scope.World, Options));
	CollectGarbage(RF_NoFlags);
	TestTrue(TEXT("GameInstance retains a handle ignored by its caller"), Ignored.IsValid());
	UBertaAudioUtils::StopRepeatedSound(Ignored.Get());
	CollectGarbage(RF_NoFlags);
	TestFalse(TEXT("Idle session can be collected after unregistration"), Ignored.IsValid());

	TStrongObjectPtr<UBertaRepeatedSoundHandle> Teardown(FBertaRepeatedSoundTestAccess::Start(Scope.World, Options));
	FWorldDelegates::OnWorldBeginTearDown.Broadcast(Scope.World);
	TestFalse(TEXT("World teardown cancels registered sessions"), UBertaAudioUtils::IsRepeatedSoundActive(Teardown.Get()));
	TestFalse(TEXT("World teardown releases registration"), FBertaRepeatedSoundTestAccess::Registered(Teardown.Get()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaRepeatedSoundCleanupTest, "BertaDevKit.Audio.RepeatedSound.ComponentCleanup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaRepeatedSoundCleanupTest::RunTest(const FString& Parameters)
{
	FAudioTimerWorld Scope;
	FBertaRepeatedSoundOptions Options;
	Options.bPlayImmediately = false;
	Options.Interval = 60.0f;
	TStrongObjectPtr<UBertaRepeatedSoundHandle> Handle(FBertaRepeatedSoundTestAccess::Start(Scope.World, Options));
	TStrongObjectPtr<UAudioComponent> Component(NewObject<UAudioComponent>());
	FBertaRepeatedSoundTestAccess::Track(Handle.Get(), Component.Get());
	const FTimerHandle Deadline = FBertaRepeatedSoundTestAccess::Deadline(Handle.Get());
	const FTimerHandle Fade = FBertaRepeatedSoundTestAccess::Fade(Handle.Get());

	// Simulate the native stopped notification, shared by completion and failed starts.
	Component->OnAudioPlayStateChangedNative.Broadcast(Component.Get(), EAudioComponentPlayState::Stopped);
	Component->OnAudioPlayStateChangedNative.Broadcast(Component.Get(), EAudioComponentPlayState::Stopped);
	TestEqual(TEXT("Duplicate completion removes the playback only once"), FBertaRepeatedSoundTestAccess::Tracked(Handle.Get()), 0);
	TestFalse(TEXT("Completion cancels the forced stop"), Scope.World->GetTimerManager().TimerExists(Deadline));
	TestFalse(TEXT("Completion cancels its fade start"), Scope.World->GetTimerManager().TimerExists(Fade));
	TestFalse(TEXT("Completion unbinds its delegate"), Component->OnAudioPlayStateChangedNative.IsBound());
	TestTrue(TEXT("Future starts preserve the session"), UBertaAudioUtils::IsRepeatedSoundActive(Handle.Get()));

	FBertaRepeatedSoundTestAccess::Track(Handle.Get(), Component.Get());
	Component->DestroyComponent();
	Scope.Tick(0.11f);
	TestEqual(TEXT("Sweep releases destroyed components without a completion callback"), FBertaRepeatedSoundTestAccess::Tracked(Handle.Get()), 0);
	UBertaAudioUtils::StopRepeatedSound(Handle.Get());
	TestFalse(TEXT("Stop releases remaining scheduling work"), FBertaRepeatedSoundTestAccess::Registered(Handle.Get()));

	TStrongObjectPtr<UBertaRepeatedSoundHandle> Forced(FBertaRepeatedSoundTestAccess::Start(Scope.World, Options));
	TStrongObjectPtr<UAudioComponent> ForcedComponent(NewObject<UAudioComponent>());
	FBertaRepeatedSoundTestAccess::Track(Forced.Get(), ForcedComponent.Get());
	FBertaRepeatedSoundTestAccess::ClearSweep(Forced.Get());
	Scope.Tick(1.01f);
	TestEqual(TEXT("Forced deadline releases tracked work"), FBertaRepeatedSoundTestAccess::Tracked(Forced.Get()), 0);
	UBertaAudioUtils::StopRepeatedSound(Forced.Get());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaRepeatedSoundInvalidInputTest, "BertaDevKit.Audio.RepeatedSound.InvalidInputAndAttachment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaRepeatedSoundInvalidInputTest::RunTest(const FString& Parameters)
{
	FAudioTimerWorld Scope;
	AddExpectedError(TEXT("A valid Sound and World"), EAutomationExpectedErrorFlags::Contains, 2);
	TestNull(TEXT("Null sound fails safely"), UBertaAudioUtils::PlayRepeatedSound2D(Scope.World, nullptr, {}));
	TestNull(TEXT("Invalid context fails safely"), UBertaAudioUtils::PlayRepeatedSound2D(nullptr, NewObject<USoundWave>(), {}));
	FBertaRepeatedSoundOptions Invalid;
	Invalid.RepeatCount = 0;
	AddExpectedError(TEXT("RepeatCount must be positive"), EAutomationExpectedErrorFlags::Contains, 1);
	TestNull(TEXT("Invalid options fail safely"), UBertaAudioUtils::PlayRepeatedSound2D(Scope.World, NewObject<USoundWave>(), Invalid));
	AddExpectedError(TEXT("Attached playback requires"), EAutomationExpectedErrorFlags::Contains, 1);
	TestNull(TEXT("Null attachment fails safely"), UBertaAudioUtils::PlayRepeatedSoundAttached(Scope.World, NewObject<USoundWave>(), nullptr, {}));

	AActor* Owner = Scope.World->SpawnActor<AActor>();
	USceneComponent* Target = NewObject<USceneComponent>(Owner);
	Target->RegisterComponent();
	FBertaRepeatedSoundOptions Options;
	Options.RepeatCount = 3;
	Options.bPlayImmediately = false;
	TStrongObjectPtr<UBertaRepeatedSoundHandle> Attached(FBertaRepeatedSoundTestAccess::Start(Scope.World, Options, Target));
	Target->DestroyComponent();
	Scope.Tick(0.51f);
	TestFalse(TEXT("Invalid attachment cancels future repetitions"), UBertaAudioUtils::IsRepeatedSoundActive(Attached.Get()));
	TestEqual(TEXT("Invalid attachment clears remaining work"), FBertaRepeatedSoundTestAccess::Remaining(Attached.Get()), 0);
	return true;
}

#endif
