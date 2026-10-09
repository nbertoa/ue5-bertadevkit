#pragma once

#include "Blueprint/UserWidget.h"

#include "BertaVideoPlayerWidget.generated.h"

class AGameModeBase;
class UAudioComponent;
class UBorder;
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

	/** Fade the level to black before playback, then reveal the playing video. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video|Transitions")
	bool bUseStartFade = false;

	/** Duration in real seconds of each half of the start transition. Zero is immediate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video|Transitions", meta = (ClampMin = "0.0", EditCondition = "bUseStartFade"))
	float StartFadeDuration = 0.5f;

	/** Fade the final portion of the video to black before natural completion, then reveal the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video|Transitions")
	bool bUseEndFade = false;

	/** Duration in real seconds of each half of the end transition. Zero is immediate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video|Transitions", meta = (ClampMin = "0.0", EditCondition = "bUseEndFade"))
	float EndFadeDuration = 0.5f;

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
	/** Set Media Source opens a fresh run using current Options/ExternalAudio; null clears without closing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetMediaSource, Category = "Video", meta = (ExposeOnSpawn = true))
	TObjectPtr<UMediaSource> MediaSource;

	/** Replaces embedded audio when audio is enabled. Asset-authored looping is preserved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video", meta = (ExposeOnSpawn = true))
	TObjectPtr<USoundBase> ExternalAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video", meta = (ExposeOnSpawn = true))
	FBertaVideoPlaybackOptions Options;

	/** Broadcast on a new playback start, not on transport Resume or loop restarts. */
	UPROPERTY(BlueprintAssignable, Category = "BertaDevKit|UI|Video")
	FBertaVideoPlaybackEvent OnPlaybackStarted;

	/** Broadcast once on terminal natural completion, after owned media, audio, and pause are released; not emitted for loops. */
	UPROPERTY(BlueprintAssignable, Category = "BertaDevKit|UI|Video")
	FBertaVideoPlaybackEvent OnPlaybackCompleted;

	/** Broadcast once before an unrecoverable playback failure removes the widget. */
	UPROPERTY(BlueprintAssignable, Category = "BertaDevKit|UI|Video")
	FBertaVideoPlaybackFailedEvent OnPlaybackFailed;

	/** Same-source assignment restarts; pre-construction only configures. Close stays terminal. */
	UFUNCTION(BlueprintSetter, Category = "BertaDevKit|UI|Video")
	void SetMediaSource(UMediaSource* NewMediaSource);

	/** Starts/queues playback, resumes transport Pause, or reopens a stopped run even without autoplay. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|UI|Video")
	bool Play();

	/** Preserves position/resources and freezes media fades. False when not playing or already paused. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|UI|Video")
	bool Pause();

	/** Continues a transport-paused run without repeating Started or the start fade. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|UI|Video")
	bool Resume();

	/** Nonterminal cleanup; retains parent/source/configuration and waits for explicit Play or source replacement. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|UI|Video")
	void Stop();

	/** Terminal, idempotent cleanup. Does not broadcast natural completion. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|UI|Video")
	void Close();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	friend class FBertaVideoPlayerWidgetReplacementTest;
	friend class FBertaVideoPlayerWidgetTransportTest;

	enum class EPlaybackState : uint8
	{
		Inactive,
		Stopped,
		Opening,
		Ready,
		FadingOutLevel,
		Starting,
		StartingBehindBlack,
		FadingInVideo,
		Playing,
		FadingOutVideoAtEnd,
		WaitingForEndBehindBlack,
		FadingInLevel,
		Closed
	};

	void ResetPlayback();
	bool ActivatePlayback(bool bExplicitPlay = false);
	void ApplyTransportPause(bool bPaused, double Now);
	void AdvanceFade(double Now);
	bool CreateMediaResources(FString& OutErrorMessage);
	bool AcquireRequestedPause(FString& OutErrorMessage);
	bool StartRequestedPlayback();
	bool StartReadyMedia(EPlaybackState StartingState = EPlaybackState::Starting);
	void TryStartEndFade();
	void BeginFade(EPlaybackState FadeState, float Duration);
	void FinishFade();
	void ResetVisuals();
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
	TObjectPtr<UBorder> BlackVisual;

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> InternalMediaPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> InternalMediaTexture;

	UPROPERTY(Transient)
	TObjectPtr<UMediaSoundComponent> InternalMediaSound;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> InternalExternalAudio;

	FBertaVideoPlaybackOptions ActiveOptions;
	uint64 PlaybackGeneration = 0;
	bool bNativeConstructed = false;
	bool bSourceCleared = false;
	bool bClosedExplicitly = false;
	TWeakObjectPtr<AGameModeBase> PauseGameMode;
	EPlaybackState PlaybackState = EPlaybackState::Inactive;
	double FadeStartTime = 0.0;
	double TransportPauseStartTime = 0.0;
	bool bTransportPaused = false;
	bool bEndReachedWhilePaused = false;
	float ActiveFadeDuration = 0.0f;
	float TransitionHalfDuration = 0.0f;
	float FadeStartOpacity = 0.0f;
	float FadeEndOpacity = 1.0f;
	bool bPlayRequested = false;
	bool bRestartingLoop = false;
	bool bLoopSeekPending = false;
	bool bTerminal = false;
	bool bOwnsPause = false;
	bool bPauseReleaseRequested = false;
};
