#include "UI/BertaVideoPlayerWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/AudioComponent.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Log/BertaDevKitLog.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaSource.h"
#include "MediaTexture.h"
#include "Misc/Timespan.h"
#include "Sound/SoundBase.h"

namespace BertaVideoPlayerWidgetPrivate
{
const FName RootWidgetName(TEXT("BertaVideoRoot"));
const FName ImageWidgetName(TEXT("BertaVideoImage"));
const FName BlackVisualName(TEXT("BertaVideoBlackVisual"));
}

TSharedRef<SWidget> UBertaVideoPlayerWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	VideoImage = Cast<UImage>(WidgetTree->FindWidget(BertaVideoPlayerWidgetPrivate::ImageWidgetName));
	BlackVisual = Cast<UBorder>(WidgetTree->FindWidget(BertaVideoPlayerWidgetPrivate::BlackVisualName));
	if (!VideoImage)
	{
		UWidget* ExistingRoot = WidgetTree->RootWidget;
		UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(),
			BertaVideoPlayerWidgetPrivate::RootWidgetName);
		VideoImage = WidgetTree->ConstructWidget<UImage>(
			UImage::StaticClass(),
			BertaVideoPlayerWidgetPrivate::ImageWidgetName);
		VideoImage->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (UOverlaySlot* VideoSlot = RootOverlay->AddChildToOverlay(VideoImage))
		{
			VideoSlot->SetHorizontalAlignment(HAlign_Fill);
			VideoSlot->SetVerticalAlignment(VAlign_Fill);
		}

		if (ExistingRoot)
		{
			if (UOverlaySlot* ContentSlot = RootOverlay->AddChildToOverlay(ExistingRoot))
			{
				ContentSlot->SetHorizontalAlignment(HAlign_Fill);
				ContentSlot->SetVerticalAlignment(VAlign_Fill);
			}
		}

		WidgetTree->RootWidget = RootOverlay;
	}

	if (!BlackVisual)
	{
		UOverlay* RootOverlay = Cast<UOverlay>(WidgetTree->RootWidget);
		if (RootOverlay)
		{
			BlackVisual = WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(),
				BertaVideoPlayerWidgetPrivate::BlackVisualName);
			BlackVisual->SetBrushColor(FLinearColor::Black);
			BlackVisual->SetPadding(FMargin(0.0f));
			BlackVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
			BlackVisual->SetRenderOpacity(0.0f);
			if (UOverlaySlot* FadeSlot = RootOverlay->AddChildToOverlay(BlackVisual))
			{
				FadeSlot->SetHorizontalAlignment(HAlign_Fill);
				FadeSlot->SetVerticalAlignment(VAlign_Fill);
			}
		}
	}

	return Super::RebuildWidget();
}

void UBertaVideoPlayerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsDesignTime() && !bTerminal && PlaybackState == EPlaybackState::Inactive)
	{
		ActivatePlayback();
	}
}

void UBertaVideoPlayerWidget::NativeDestruct()
{
	CleanupMediaResources();
	PlaybackState = bTerminal ? EPlaybackState::Closed : EPlaybackState::Inactive;
	ResetVisuals();
	Super::NativeDestruct();
}

void UBertaVideoPlayerWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	TryStartEndFade();

	const bool bFadingOut = PlaybackState == EPlaybackState::FadingOutLevel || PlaybackState == EPlaybackState::FadingOutVideoAtEnd;
	const bool bFadingIn = PlaybackState == EPlaybackState::FadingInVideo || PlaybackState == EPlaybackState::FadingInLevel;
	if ((!bFadingOut && !bFadingIn) || !BlackVisual)
	{
		return;
	}

	const double ElapsedSeconds = FMath::Max(0.0, FPlatformTime::Seconds() - FadeStartTime);
	const float Alpha = FMath::Clamp(static_cast<float>(ElapsedSeconds / ActiveFadeDuration), 0.0f, 1.0f);
	BlackVisual->SetRenderOpacity(FMath::Lerp(FadeStartOpacity, FadeEndOpacity, Alpha));
	if (Alpha >= 1.0f)
	{
		FinishFade();
	}
}

bool UBertaVideoPlayerWidget::Play()
{
	if (bTerminal)
	{
		return false;
	}

	switch (PlaybackState)
	{
	case EPlaybackState::Opening:
		bPlayRequested = true;
		return true;

	case EPlaybackState::Ready:
		bPlayRequested = true;
		return StartRequestedPlayback();

	case EPlaybackState::FadingOutLevel:
	case EPlaybackState::Starting:
	case EPlaybackState::StartingBehindBlack:
	case EPlaybackState::FadingInVideo:
	case EPlaybackState::Playing:
		return true;

	default:
		return false;
	}
}

void UBertaVideoPlayerWidget::Close()
{
	bTerminal = true;
	PlaybackState = EPlaybackState::Closed;
	CleanupMediaResources();
	ResetVisuals();
	RemoveFromParent();
}

bool UBertaVideoPlayerWidget::ActivatePlayback()
{
	if (!IsValid(MediaSource))
	{
		FailPlayback(TEXT("No valid Media Source was configured."));
		return false;
	}

	if (!GetWorld())
	{
		FailPlayback(TEXT("The video widget has no valid World context."));
		return false;
	}

	if (Options.bUseStartFade && VideoImage)
	{
		VideoImage->SetVisibility(ESlateVisibility::Hidden);
	}

	FString ErrorMessage;
	if (!CreateMediaResources(ErrorMessage))
	{
		FailPlayback(ErrorMessage);
		return false;
	}

	if (!AcquireRequestedPause(ErrorMessage))
	{
		FailPlayback(ErrorMessage);
		return false;
	}

	bPlayRequested = Options.bAutoPlay;
	PlaybackState = EPlaybackState::Opening;
	UMediaPlayer* MediaPlayer = InternalMediaPlayer;
	const bool bOpenStarted = MediaPlayer->OpenSource(MediaSource);
	if (!bOpenStarted && !bTerminal)
	{
		FailPlayback(FString::Printf(
			TEXT("Failed to begin opening Media Source '%s'."),
			*GetNameSafe(MediaSource)));
		return false;
	}

	return !bTerminal;
}

bool UBertaVideoPlayerWidget::CreateMediaResources(FString& OutErrorMessage)
{
	OutErrorMessage.Reset();
	if (!VideoImage || ((Options.bUseStartFade || Options.bUseEndFade) && !BlackVisual))
	{
		OutErrorMessage = TEXT("Failed to create the native video visuals.");
		return false;
	}

	InternalMediaPlayer = NewObject<UMediaPlayer>(this, NAME_None, RF_Transient);
	InternalMediaTexture = NewObject<UMediaTexture>(this, NAME_None, RF_Transient);
	if (!InternalMediaPlayer || !InternalMediaTexture)
	{
		OutErrorMessage = TEXT("Failed to create Media Framework playback resources.");
		return false;
	}

	InternalMediaPlayer->PlayOnOpen = false;
	InternalMediaPlayer->NativeAudioOut = false;
	// Backends do not all emit OnEndReached for native loops. Seek/replay each lap
	// so external audio restart and terminal completion use the same boundary.
	InternalMediaPlayer->SetLooping(false);
	InternalMediaPlayer->OnMediaOpened.AddDynamic(this, &ThisClass::HandleMediaOpened);
	InternalMediaPlayer->OnMediaOpenFailed.AddDynamic(this, &ThisClass::HandleMediaOpenFailed);
	InternalMediaPlayer->OnPlaybackResumed.AddDynamic(this, &ThisClass::HandlePlaybackResumed);
	InternalMediaPlayer->OnEndReached.AddDynamic(this, &ThisClass::HandleEndReached);
	InternalMediaPlayer->OnSeekCompleted.AddDynamic(this, &ThisClass::HandleSeekCompleted);

	InternalMediaTexture->AutoClear = true;
	InternalMediaTexture->ClearColor = FLinearColor::Black;
	InternalMediaTexture->SetMediaPlayer(InternalMediaPlayer);
	InternalMediaTexture->UpdateResource();
	VideoImage->SetBrushResourceObject(InternalMediaTexture);

	if (Options.bPlayAudio && IsValid(ExternalAudio))
	{
		InternalExternalAudio = NewObject<UAudioComponent>(this, NAME_None, RF_Transient);
		InternalExternalAudio->SetAutoActivate(false);
		InternalExternalAudio->bAutoDestroy = false;
		InternalExternalAudio->bCanPlayMultipleInstances = false;
		InternalExternalAudio->bAllowSpatialization = false;
		InternalExternalAudio->SetUISound(true);
		InternalExternalAudio->SetSound(ExternalAudio);
		InternalExternalAudio->RegisterComponentWithWorld(GetWorld());
		if (!InternalExternalAudio->IsRegistered())
		{
			OutErrorMessage = TEXT("Failed to register the external audio component.");
			return false;
		}
	}
	else if (Options.bPlayAudio)
	{
		InternalMediaSound = NewObject<UMediaSoundComponent>(this, NAME_None, RF_Transient);
		if (!InternalMediaSound)
		{
			OutErrorMessage = TEXT("Failed to create the media audio component.");
			return false;
		}

		InternalMediaSound->bIsUISound = true;
		InternalMediaSound->SetMediaPlayer(InternalMediaPlayer);
		InternalMediaSound->RegisterComponentWithWorld(GetWorld());
		if (!InternalMediaSound->IsRegistered())
		{
			OutErrorMessage = TEXT("Failed to register the media audio component.");
			return false;
		}
		InternalMediaSound->Start();
		InternalMediaSound->AddClockSink();
		InternalMediaSound->UpdatePlayer();
	}

	return true;
}

bool UBertaVideoPlayerWidget::AcquireRequestedPause(FString& OutErrorMessage)
{
	OutErrorMessage.Reset();
	if (!Options.bPauseGame)
	{
		return true;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = GetOwningPlayer();
	AGameModeBase* GameMode = World ? World->GetAuthGameMode() : nullptr;
	if (!PlayerController || !GameMode)
	{
		OutErrorMessage = TEXT("Pausing playback requires an owning Player Controller and authoritative Game Mode.");
		return false;
	}

	if (GameMode->IsPaused())
	{
		return true;
	}

	bPauseReleaseRequested = false;
	if (!PlayerController->SetPause(
		true,
		FCanUnpause::CreateUObject(this, &ThisClass::CanReleaseOwnedPause)))
	{
		OutErrorMessage = TEXT("The Game Mode rejected the video widget's pause request.");
		return false;
	}

	bOwnsPause = true;
	PauseGameMode = GameMode;
	if (!GameMode->IsPaused())
	{
		ReleaseOwnedPause();
		OutErrorMessage = TEXT("The video widget's pause request did not pause the World.");
		return false;
	}

	return true;
}

bool UBertaVideoPlayerWidget::StartReadyMedia(const EPlaybackState StartingState)
{
	if (bTerminal || PlaybackState != EPlaybackState::Ready || !InternalMediaPlayer)
	{
		return false;
	}

	PlaybackState = StartingState;
	if (!InternalMediaPlayer->Play())
	{
		FailPlayback(FString::Printf(
			TEXT("Failed to start playback for Media Source '%s'."),
			*GetNameSafe(MediaSource)));
		return false;
	}

	return !bTerminal;
}

bool UBertaVideoPlayerWidget::StartRequestedPlayback()
{
	if (Options.bUseStartFade && !bRestartingLoop)
	{
		TransitionHalfDuration = FMath::Max(0.0f, Options.StartFadeDuration);
		BeginFade(EPlaybackState::FadingOutLevel, TransitionHalfDuration);
		return !bTerminal;
	}

	return StartReadyMedia();
}

void UBertaVideoPlayerWidget::TryStartEndFade()
{
	if (!Options.bUseEndFade || Options.bLoop || Options.EndFadeDuration <= 0.0f || bTerminal
		|| (PlaybackState != EPlaybackState::Playing && PlaybackState != EPlaybackState::FadingInVideo)
		|| !InternalMediaPlayer || !InternalMediaPlayer->IsPlaying())
	{
		return;
	}

	const FTimespan Duration = InternalMediaPlayer->GetDuration();
	const FTimespan CurrentTime = InternalMediaPlayer->GetTime();
	if (Duration <= FTimespan::Zero() || CurrentTime < FTimespan::Zero() || CurrentTime > Duration)
	{
		return;
	}

	const double RemainingSeconds = (Duration - CurrentTime).GetTotalSeconds();
	if (RemainingSeconds <= Options.EndFadeDuration)
	{
		TransitionHalfDuration = FMath::Max(0.0f, Options.EndFadeDuration);
		// A late UI tick uses the remaining media time so black is reached near the natural end.
		BeginFade(EPlaybackState::FadingOutVideoAtEnd, static_cast<float>(FMath::Max(0.0, RemainingSeconds)));
	}
}

void UBertaVideoPlayerWidget::BeginFade(const EPlaybackState FadeState, const float Duration)
{
	PlaybackState = FadeState;
	ActiveFadeDuration = FMath::Max(0.0f, Duration);
	const bool bFadingOut = FadeState == EPlaybackState::FadingOutLevel || FadeState == EPlaybackState::FadingOutVideoAtEnd;
	FadeStartOpacity = FadeState == EPlaybackState::FadingOutVideoAtEnd ? BlackVisual->GetRenderOpacity() : (bFadingOut ? 0.0f : 1.0f);
	FadeEndOpacity = bFadingOut ? 1.0f : 0.0f;
	BlackVisual->SetRenderOpacity(FadeStartOpacity);
	if (ActiveFadeDuration <= 0.0f)
	{
		FinishFade();
		return;
	}

	FadeStartTime = FPlatformTime::Seconds();
}

void UBertaVideoPlayerWidget::FinishFade()
{
	const EPlaybackState FinishedState = PlaybackState;
	BlackVisual->SetRenderOpacity(FadeEndOpacity);

	switch (FinishedState)
	{
	case EPlaybackState::FadingOutLevel:
		VideoImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		PlaybackState = EPlaybackState::Ready;
		StartReadyMedia(EPlaybackState::StartingBehindBlack);
		break;

	case EPlaybackState::FadingInVideo:
		PlaybackState = EPlaybackState::Playing;
		break;

	case EPlaybackState::FadingOutVideoAtEnd:
		PlaybackState = EPlaybackState::WaitingForEndBehindBlack;
		VideoImage->SetVisibility(ESlateVisibility::Hidden);
		break;

	case EPlaybackState::FadingInLevel:
		PlaybackState = EPlaybackState::Closed;
		if (Options.bRemoveOnCompletion)
		{
			RemoveFromParent();
		}
		break;

	default:
		break;
	}
}

void UBertaVideoPlayerWidget::ResetVisuals()
{
	if (BlackVisual)
	{
		BlackVisual->SetRenderOpacity(0.0f);
	}
	if (VideoImage)
	{
		VideoImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

bool UBertaVideoPlayerWidget::CanReleaseOwnedPause() const
{
	return bPauseReleaseRequested;
}

void UBertaVideoPlayerWidget::ReleaseOwnedPause()
{
	if (!bOwnsPause)
	{
		return;
	}

	bPauseReleaseRequested = true;
	if (AGameModeBase* GameMode = PauseGameMode.Get())
	{
		GameMode->ClearPause();
	}

	bOwnsPause = false;
	PauseGameMode.Reset();
}

void UBertaVideoPlayerWidget::CleanupMediaResources()
{
	ReleaseOwnedPause();
	bPlayRequested = false;
	bRestartingLoop = false;
	bLoopSeekPending = false;

	if (InternalMediaPlayer)
	{
		InternalMediaPlayer->OnMediaOpened.RemoveDynamic(this, &ThisClass::HandleMediaOpened);
		InternalMediaPlayer->OnMediaOpenFailed.RemoveDynamic(this, &ThisClass::HandleMediaOpenFailed);
		InternalMediaPlayer->OnPlaybackResumed.RemoveDynamic(this, &ThisClass::HandlePlaybackResumed);
		InternalMediaPlayer->OnEndReached.RemoveDynamic(this, &ThisClass::HandleEndReached);
		InternalMediaPlayer->OnSeekCompleted.RemoveDynamic(this, &ThisClass::HandleSeekCompleted);
		InternalMediaPlayer->Close();
	}

	if (InternalExternalAudio)
	{
		InternalExternalAudio->Stop();
		InternalExternalAudio->SetSound(nullptr);
		if (InternalExternalAudio->IsRegistered())
		{
			InternalExternalAudio->UnregisterComponent();
		}
		InternalExternalAudio->DestroyComponent();
		InternalExternalAudio = nullptr;
	}

	if (InternalMediaSound)
	{
		InternalMediaSound->RemoveClockSink();
		InternalMediaSound->SetMediaPlayer(nullptr);
		InternalMediaSound->UpdatePlayer();
		InternalMediaSound->Stop();
		if (InternalMediaSound->IsRegistered())
		{
			InternalMediaSound->UnregisterComponent();
		}
		InternalMediaSound->DestroyComponent();
		InternalMediaSound = nullptr;
	}

	if (InternalMediaTexture)
	{
		InternalMediaTexture->SetMediaPlayer(nullptr);
		InternalMediaTexture->UpdateResource();
		InternalMediaTexture = nullptr;
	}

	if (VideoImage)
	{
		VideoImage->SetBrushResourceObject(nullptr);
	}
	InternalMediaPlayer = nullptr;

	if (!bTerminal)
	{
		PlaybackState = EPlaybackState::Inactive;
	}
}

void UBertaVideoPlayerWidget::FailPlayback(const FString& ErrorMessage)
{
	if (bTerminal)
	{
		return;
	}

	bTerminal = true;
	PlaybackState = EPlaybackState::Closed;
	UE_LOG(
		LogBertaDevKit,
		Warning,
		TEXT("[BertaVideoPlayerWidget] %s"),
		*ErrorMessage);
	OnPlaybackFailed.Broadcast(this, ErrorMessage);
	CleanupMediaResources();
	ResetVisuals();
	RemoveFromParent();
}

void UBertaVideoPlayerWidget::HandleMediaOpened(FString OpenedUrl)
{
	static_cast<void>(OpenedUrl);

	if (bTerminal || PlaybackState != EPlaybackState::Opening)
	{
		return;
	}

	PlaybackState = EPlaybackState::Ready;
	if (bPlayRequested)
	{
		StartRequestedPlayback();
	}
}

void UBertaVideoPlayerWidget::HandleMediaOpenFailed(FString FailedUrl)
{
	if (!bTerminal)
	{
		FailPlayback(FString::Printf(
			TEXT("Media Source '%s' failed to open (%s)."),
			*GetNameSafe(MediaSource),
			*FailedUrl));
	}
}

void UBertaVideoPlayerWidget::HandlePlaybackResumed()
{
	if (bTerminal || bLoopSeekPending || (PlaybackState != EPlaybackState::Starting && PlaybackState != EPlaybackState::StartingBehindBlack))
	{
		return;
	}

	const bool bRevealVideo = PlaybackState == EPlaybackState::StartingBehindBlack;
	PlaybackState = EPlaybackState::Playing;
	const bool bWasRestartingLoop = bRestartingLoop;
	bRestartingLoop = false;
	if (InternalExternalAudio && (!bWasRestartingLoop || Options.bRestartExternalAudioOnLoop))
	{
		InternalExternalAudio->Stop();
		InternalExternalAudio->Play(0.0f);
	}

	if (!bWasRestartingLoop)
	{
		OnPlaybackStarted.Broadcast(this);
		if (!bTerminal && PlaybackState == EPlaybackState::Playing && bRevealVideo)
		{
			BeginFade(EPlaybackState::FadingInVideo, TransitionHalfDuration);
		}
	}
}

void UBertaVideoPlayerWidget::HandleEndReached()
{
	if (bTerminal || bLoopSeekPending || (PlaybackState != EPlaybackState::Starting && PlaybackState != EPlaybackState::StartingBehindBlack && PlaybackState != EPlaybackState::FadingInVideo && PlaybackState != EPlaybackState::Playing && PlaybackState != EPlaybackState::FadingOutVideoAtEnd && PlaybackState != EPlaybackState::WaitingForEndBehindBlack))
	{
		return;
	}

	if (Options.bLoop)
	{
		if (BlackVisual)
		{
			BlackVisual->SetRenderOpacity(0.0f);
		}
		bRestartingLoop = true;
		bLoopSeekPending = true;
		PlaybackState = EPlaybackState::Starting;
		if (!InternalMediaPlayer || !InternalMediaPlayer->Seek(FTimespan::Zero()))
		{
			FailPlayback(TEXT("Failed to seek to the beginning for video looping."));
		}
		return;
	}

	if (Options.bUseEndFade)
	{
		TransitionHalfDuration = FMath::Max(0.0f, Options.EndFadeDuration);
		// Cover any final sample or backend clear before hiding the video.
		BlackVisual->SetRenderOpacity(1.0f);
		VideoImage->SetVisibility(ESlateVisibility::Hidden);
		bTerminal = true;
		PlaybackState = EPlaybackState::FadingInLevel;
		CleanupMediaResources();
		OnPlaybackCompleted.Broadcast(this);
		if (PlaybackState == EPlaybackState::FadingInLevel)
		{
			BeginFade(EPlaybackState::FadingInLevel, TransitionHalfDuration);
		}
		return;
	}

	bTerminal = true;
	PlaybackState = EPlaybackState::Closed;
	CleanupMediaResources();
	ResetVisuals();
	OnPlaybackCompleted.Broadcast(this);
	if (Options.bRemoveOnCompletion)
	{
		RemoveFromParent();
	}
}

void UBertaVideoPlayerWidget::HandleSeekCompleted()
{
	if (bTerminal || !bLoopSeekPending || PlaybackState != EPlaybackState::Starting || !InternalMediaPlayer)
	{
		return;
	}

	bLoopSeekPending = false;
	// WMF may resume as part of seeking and emit PlaybackResumed before SeekCompleted.
	// Confirm that resumed state here rather than waiting for another resume event.
	if (InternalMediaPlayer->IsPlaying())
	{
		HandlePlaybackResumed();
	}
	else
	{
		PlaybackState = EPlaybackState::Ready;
		StartReadyMedia();
	}
}
