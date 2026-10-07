#pragma once

#include "CoreMinimal.h"
#include "Audio/BertaAudioUtils.h"
#include "Engine/TimerHandle.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "BertaRepeatedSoundHandle.generated.h"

class UAudioComponent;
class UWorld;
enum class EAudioComponentPlayState : uint8;

/** Opaque per-sequence lifetime. The GameInstance keeps it alive until all owned work ends. */
UCLASS(BlueprintType, NotBlueprintable, Transient)
class BERTADEVKIT_API UBertaRepeatedSoundHandle : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

private:
	friend class UBertaAudioUtils;
	friend struct FBertaRepeatedSoundTestAccess;

	enum class EState : uint8 { Running, Stopping, Finished };
	enum class EMode : uint8 { TwoD, Location, Attached };

	struct FPlayback
	{
		TWeakObjectPtr<UAudioComponent> Component;
		FDelegateHandle PlayStateDelegate;
		FTimerHandle FadeTimer;
		FTimerHandle DeadlineTimer;
	};

	void Start(const UObject* Context);
	void StartNext();
	void ScheduleNext();
	UAudioComponent* CreatePlayback(float Volume, float Pitch) const;
	void FadePlayback(TWeakObjectPtr<UAudioComponent> Component, float Duration);
	void StopPlayback(TWeakObjectPtr<UAudioComponent> Component);
	void OnPlayStateChanged(const UAudioComponent* Component, EAudioComponentPlayState PlayState);
	void RemovePlayback(TWeakObjectPtr<UAudioComponent> Component);
	void Sweep();
	void Stop(float FadeDuration);
	void FinishIfIdle();
	void Finish();
	void OnWorldTearDown(UWorld* InWorld);
	void OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources);
	bool IsActive() const;
	bool HasValidTarget() const;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(Transient)
	TObjectPtr<USoundAttenuation> Attenuation;

	UPROPERTY(Transient)
	TObjectPtr<USoundConcurrency> Concurrency;

	FBertaRepeatedSoundOptions Options;
	TWeakObjectPtr<UWorld> World;
	TWeakObjectPtr<USceneComponent> AttachTarget;
	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	FName AttachPointName = NAME_None;
	EMode Mode = EMode::TwoD;
	EState State = EState::Finished;
	int32 RemainingStarts = 0;
	bool bStartingPlayback = false;
	TArray<FPlayback> Playbacks;
	FTimerHandle NextStartTimer;
	FTimerHandle SweepTimer;
	FDelegateHandle TearDownDelegate;
	FDelegateHandle CleanupDelegate;
};
