#include "Audio/BertaAudioUtils.h"

#include "Audio/BertaRepeatedSoundHandle.h"
#include "Audio/BertaRepeatedSoundPolicy.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Log/BertaDevKitLog.h"
#include "Sound/SoundBase.h"
#include "UObject/UObjectGlobals.h"

UBertaRepeatedSoundHandle* UBertaAudioUtils::PlayRepeatedSound2D(const UObject* WorldContextObject, USoundBase* Sound,
	const FBertaRepeatedSoundOptions& Options, USoundConcurrency* ConcurrencySettings)
{
	return Start(WorldContextObject, Sound, Options, nullptr, false, false, {}, {}, NAME_None, nullptr, ConcurrencySettings);
}

UBertaRepeatedSoundHandle* UBertaAudioUtils::PlayRepeatedSoundAtLocation(const UObject* WorldContextObject, USoundBase* Sound,
	FVector Location, const FBertaRepeatedSoundOptions& Options, FRotator Rotation,
	USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings)
{
	return Start(WorldContextObject, Sound, Options, nullptr, false, true, Location, Rotation, NAME_None,
		AttenuationSettings, ConcurrencySettings);
}

UBertaRepeatedSoundHandle* UBertaAudioUtils::PlayRepeatedSoundAttached(const UObject* WorldContextObject, USoundBase* Sound,
	USceneComponent* AttachToComponent, const FBertaRepeatedSoundOptions& Options, FName AttachPointName,
	FVector Location, FRotator Rotation, USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings)
{
	return Start(WorldContextObject, Sound, Options, AttachToComponent, true, true, Location, Rotation, AttachPointName,
		AttenuationSettings, ConcurrencySettings);
}

UBertaRepeatedSoundHandle* UBertaAudioUtils::Start(const UObject* WorldContextObject, USoundBase* Sound,
	const FBertaRepeatedSoundOptions& Options, USceneComponent* AttachToComponent, bool bAttached,
	bool bSpatial, FVector Location, FRotator Rotation, FName AttachPointName,
	USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings)
{
	UWorld* World = IsValid(WorldContextObject) && GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!IsValid(Sound) || !IsValid(World) || World->bIsTearingDown || !IsValid(World->GetGameInstance()))
	{
		UE_LOG(LogBertaDevKit, Warning, TEXT("[BertaAudioUtils] A valid Sound and World with a GameInstance are required."));
		return nullptr;
	}
	if (!BertaRepeatedSoundPrivate::ValidateOptions(Options) || Location.ContainsNaN() || Rotation.ContainsNaN())
	{
		UE_LOG(LogBertaDevKit, Warning, TEXT("[BertaAudioUtils] RepeatCount must be positive; options and transform must be finite, with non-negative intervals, multipliers, variances and fade durations."));
		return nullptr;
	}
	if (bAttached && (!IsValid(AttachToComponent) || AttachToComponent->GetWorld() != World
		|| !IsValid(AttachToComponent->GetOwner()) || AttachToComponent->GetOwner()->IsActorBeingDestroyed()))
	{
		UE_LOG(LogBertaDevKit, Warning, TEXT("[BertaAudioUtils] Attached playback requires a live component/owner in the context World."));
		return nullptr;
	}
	if (!GEngine->UseSound() || !World->bAllowAudioPlayback || World->IsNetMode(NM_DedicatedServer) || !World->GetAudioDeviceRaw())
	{
		UE_LOG(LogBertaDevKit, Warning, TEXT("[BertaAudioUtils] Audio playback is unavailable in this World."));
		return nullptr;
	}

	UBertaRepeatedSoundHandle* Handle = NewObject<UBertaRepeatedSoundHandle>();
	Handle->Sound = Sound;
	Handle->World = World;
	Handle->Options = Options;
	Handle->AttachTarget = AttachToComponent;
	Handle->Mode = bAttached ? UBertaRepeatedSoundHandle::EMode::Attached
		: (bSpatial ? UBertaRepeatedSoundHandle::EMode::Location : UBertaRepeatedSoundHandle::EMode::TwoD);
	Handle->Location = Location;
	Handle->Rotation = Rotation;
	Handle->AttachPointName = AttachPointName;
	Handle->Attenuation = AttenuationSettings;
	Handle->Concurrency = ConcurrencySettings;
	Handle->Start(WorldContextObject);
	return Handle;
}

void UBertaAudioUtils::StopRepeatedSound(UBertaRepeatedSoundHandle* Handle, float FadeOutActiveSoundsDuration)
{
	if (!IsValid(Handle) || Handle->State != UBertaRepeatedSoundHandle::EState::Running)
	{
		return;
	}
	if (!FMath::IsFinite(FadeOutActiveSoundsDuration))
	{
		UE_LOG(LogBertaDevKit, Warning, TEXT("[BertaAudioUtils] Stop fade duration must be finite."));
		return;
	}
	Handle->Stop(FMath::Max(0.0f, FadeOutActiveSoundsDuration));
}

bool UBertaAudioUtils::IsRepeatedSoundActive(const UBertaRepeatedSoundHandle* Handle)
{
	return IsValid(Handle) && Handle->IsActive();
}
