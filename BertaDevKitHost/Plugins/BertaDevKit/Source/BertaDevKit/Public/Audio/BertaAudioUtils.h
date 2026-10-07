#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaAudioUtils.generated.h"

class UBertaRepeatedSoundHandle;
class USceneComponent;
class USoundAttenuation;
class USoundBase;
class USoundConcurrency;

USTRUCT(BlueprintType)
struct BERTADEVKIT_API FBertaRepeatedSoundOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "1"))
	int32 RepeatCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Timing", meta = (ClampMin = "0.0"))
	float Interval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Timing", meta = (ClampMin = "0.0"))
	float IntervalVariance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Timing")
	bool bPlayImmediately = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Variation", meta = (ClampMin = "0.0"))
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Variation", meta = (ClampMin = "0.0"))
	float VolumeVariance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Variation", meta = (ClampMin = "0.0"))
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Variation", meta = (ClampMin = "0.0"))
	float PitchVariance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fade")
	bool bUseFadeIn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fade", meta = (ClampMin = "0.0", EditCondition = "bUseFadeIn"))
	float FadeInDuration = 0.2f;

	/** <= 0 lets the sound finish naturally, including authored loops. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Lifetime", meta = (ClampMin = "0.0"))
	float PlaybackDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fade")
	bool bUseFadeOut = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fade", meta = (ClampMin = "0.0", EditCondition = "bUseFadeOut"))
	float FadeOutDuration = 0.2f;
};

/** Local audio playback using world/game-time scheduling. No replication. */
UCLASS()
class BERTADEVKIT_API UBertaAudioUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** RepeatCount is the total starts; each interval is measured between starts, allowing overlap. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Audio",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Play Repeated Sound 2D",
			ReturnDisplayName = "Repeated Sound Handle", AdvancedDisplay = "ConcurrencySettings"))
	static UBertaRepeatedSoundHandle* PlayRepeatedSound2D(const UObject* WorldContextObject, USoundBase* Sound,
		const FBertaRepeatedSoundOptions& Options, USoundConcurrency* ConcurrencySettings = nullptr);

	/** Captures this transform once and reuses it for every start. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Audio",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Play Repeated Sound at Location",
			ReturnDisplayName = "Repeated Sound Handle", AdvancedDisplay = "Rotation,AttenuationSettings,ConcurrencySettings"))
	static UBertaRepeatedSoundHandle* PlayRepeatedSoundAtLocation(const UObject* WorldContextObject, USoundBase* Sound,
		FVector Location, const FBertaRepeatedSoundOptions& Options, FRotator Rotation = FRotator::ZeroRotator,
		USoundAttenuation* AttenuationSettings = nullptr, USoundConcurrency* ConcurrencySettings = nullptr);

	/** Relative to the supplied component/socket. Losing the target cancels the session. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Audio",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Play Repeated Sound Attached",
			ReturnDisplayName = "Repeated Sound Handle", AdvancedDisplay = "AttachPointName,Location,Rotation,AttenuationSettings,ConcurrencySettings"))
	static UBertaRepeatedSoundHandle* PlayRepeatedSoundAttached(const UObject* WorldContextObject, USoundBase* Sound,
		USceneComponent* AttachToComponent, const FBertaRepeatedSoundOptions& Options, FName AttachPointName = NAME_None,
		FVector Location = FVector::ZeroVector, FRotator Rotation = FRotator::ZeroRotator,
		USoundAttenuation* AttenuationSettings = nullptr, USoundConcurrency* ConcurrencySettings = nullptr);

	/** Cancels future starts. A positive duration fades active sounds; subsequent calls are harmless. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Audio", meta = (DisplayName = "Stop Repeated Sound"))
	static void StopRepeatedSound(UBertaRepeatedSoundHandle* Handle, float FadeOutActiveSoundsDuration = 0.0f);

	/** True while future starts or owned sounds/fades remain, including fades requested by Stop. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Audio", meta = (DisplayName = "Is Repeated Sound Active"))
	static bool IsRepeatedSoundActive(const UBertaRepeatedSoundHandle* Handle);

private:
	static UBertaRepeatedSoundHandle* Start(const UObject* WorldContextObject, USoundBase* Sound,
		const FBertaRepeatedSoundOptions& Options, USceneComponent* AttachToComponent, bool bAttached,
		bool bSpatial, FVector Location, FRotator Rotation, FName AttachPointName,
		USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings);
};
