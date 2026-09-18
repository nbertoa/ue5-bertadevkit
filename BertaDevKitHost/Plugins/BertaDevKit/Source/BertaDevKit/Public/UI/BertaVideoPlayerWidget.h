#pragma once

#include "Blueprint/UserWidget.h"

#include "BertaVideoPlayerWidget.generated.h"

class AGameModeBase;
class UAudioComponent;
class UImage;
class UMediaPlayer;
class UMediaSoundComponent;
class UMediaSource;
class UMediaTexture;
class USoundBase;
class UBertaVideoPlayerWidget;

USTRUCT(BlueprintType)
struct BERTADEVKIT_API FBertaVideoPlaybackOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video")
	bool bAutoPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video")
	bool bPauseGame = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video")
	bool bRemoveOnCompletion = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video")
	bool bPlayAudio = true;

	/** Repeat the video without terminal completion or releasing the owned pause. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video")
	bool bLoop = false;

	/** Only affects external audio: restart each video loop, otherwise let it finish naturally. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video", meta = (EditCondition = "bLoop && bPlayAudio"))
	bool bRestartExternalAudioOnLoop = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBertaVideoPlaybackEvent,
	UBertaVideoPlayerWidget*, Widget);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FBertaVideoPlaybackFailedEvent,
	UBertaVideoPlayerWidget*, Widget,
	FString, ErrorMessage);

/** Fullscreen Runtime Media Framework playback with deterministic widget-owned cleanup. */
UCLASS(Blueprintable)
class BERTADEVKIT_API UBertaVideoPlayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Source opened when this widget is added to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video", meta = (ExposeOnSpawn = true))
	TObjectPtr<UMediaSource> MediaSource;

	/** Replaces embedded audio when audio is enabled. Asset-authored looping is preserved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video", meta = (ExposeOnSpawn = true))
	TObjectPtr<USoundBase> ExternalAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video", meta = (ExposeOnSpawn = true))
	FBertaVideoPlaybackOptions Options;

	/** Broadcast after Media Framework reports that playback actually resumed. */
	UPROPERTY(BlueprintAssignable, Category = "BertaDevKit|UI|Video")
	FBertaVideoPlaybackEvent OnPlaybackStarted;

	/** Broadcast once on terminal natural completion; not emitted for individual loops. */
	UPROPERTY(BlueprintAssignable, Category = "BertaDevKit|UI|Video")
	FBertaVideoPlaybackEvent OnPlaybackCompleted;

	/** Broadcast once before an unrecoverable playback failure removes the widget. */
	UPROPERTY(BlueprintAssignable, Category = "BertaDevKit|UI|Video")
	FBertaVideoPlaybackFailedEvent OnPlaybackFailed;

	/** Starts playback now, or queues it until an in-progress open completes. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|UI|Video")
	bool Play();

	/** Terminal, idempotent cleanup. Does not broadcast natural completion. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|UI|Video")
	void Close();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	enum class EPlaybackState : uint8
	{
		Inactive,
		Opening,
		Ready,
		Starting,
		Playing,
		Closed
	};

	bool ActivatePlayback();
	bool CreateMediaResources(FString& OutErrorMessage);
	bool AcquireRequestedPause(FString& OutErrorMessage);
	bool StartReadyMedia();
	bool CanReleaseOwnedPause() const;
	void ReleaseOwnedPause();
	void CleanupMediaResources();
	void FailPlayback(const FString& ErrorMessage);

	UFUNCTION()
	void HandleMediaOpened(FString OpenedUrl);

	UFUNCTION()
	void HandleMediaOpenFailed(FString FailedUrl);

	UFUNCTION()
	void HandlePlaybackResumed();

	UFUNCTION()
	void HandleEndReached();

	UFUNCTION()
	void HandleSeekCompleted();

	UPROPERTY(Transient)
	TObjectPtr<UImage> VideoImage;

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> InternalMediaPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> InternalMediaTexture;

	UPROPERTY(Transient)
	TObjectPtr<UMediaSoundComponent> InternalMediaSound;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> InternalExternalAudio;

	TWeakObjectPtr<AGameModeBase> PauseGameMode;
	EPlaybackState PlaybackState = EPlaybackState::Inactive;
	bool bPlayRequested = false;
	bool bRestartingLoop = false;
	bool bLoopSeekPending = false;
	bool bTerminal = false;
	bool bOwnsPause = false;
	bool bPauseReleaseRequested = false;
};
