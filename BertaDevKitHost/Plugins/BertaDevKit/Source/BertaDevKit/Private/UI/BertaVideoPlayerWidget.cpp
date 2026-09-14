#include "UI/BertaVideoPlayerWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Log/BertaDevKitLog.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaSource.h"
#include "MediaTexture.h"

namespace BertaVideoPlayerWidgetPrivate
{
const FName RootWidgetName(TEXT("BertaVideoRoot"));
const FName ImageWidgetName(TEXT("BertaVideoImage"));
}

TSharedRef<SWidget> UBertaVideoPlayerWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	VideoImage = Cast<UImage>(WidgetTree->FindWidget(BertaVideoPlayerWidgetPrivate::ImageWidgetName));
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
	Super::NativeDestruct();
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
		return StartReadyMedia();

	case EPlaybackState::Starting:
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
	if (!VideoImage)
	{
		OutErrorMessage = TEXT("Failed to create the native video image.");
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
	InternalMediaPlayer->SetLooping(false);
	InternalMediaPlayer->OnMediaOpened.AddDynamic(this, &ThisClass::HandleMediaOpened);
	InternalMediaPlayer->OnMediaOpenFailed.AddDynamic(this, &ThisClass::HandleMediaOpenFailed);
	InternalMediaPlayer->OnPlaybackResumed.AddDynamic(this, &ThisClass::HandlePlaybackResumed);
	InternalMediaPlayer->OnEndReached.AddDynamic(this, &ThisClass::HandleEndReached);

	InternalMediaTexture->AutoClear = true;
	InternalMediaTexture->ClearColor = FLinearColor::Black;
	InternalMediaTexture->SetMediaPlayer(InternalMediaPlayer);
	InternalMediaTexture->UpdateResource();
	VideoImage->SetBrushResourceObject(InternalMediaTexture);

	if (Options.bPlayAudio)
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

bool UBertaVideoPlayerWidget::StartReadyMedia()
{
	if (bTerminal || PlaybackState != EPlaybackState::Ready || !InternalMediaPlayer)
	{
		return false;
	}

	PlaybackState = EPlaybackState::Starting;
	if (!InternalMediaPlayer->Play())
	{
		FailPlayback(FString::Printf(
			TEXT("Failed to start playback for Media Source '%s'."),
			*GetNameSafe(MediaSource)));
		return false;
	}

	return !bTerminal;
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

	if (InternalMediaPlayer)
	{
		InternalMediaPlayer->OnMediaOpened.RemoveDynamic(this, &ThisClass::HandleMediaOpened);
		InternalMediaPlayer->OnMediaOpenFailed.RemoveDynamic(this, &ThisClass::HandleMediaOpenFailed);
		InternalMediaPlayer->OnPlaybackResumed.RemoveDynamic(this, &ThisClass::HandlePlaybackResumed);
		InternalMediaPlayer->OnEndReached.RemoveDynamic(this, &ThisClass::HandleEndReached);
		InternalMediaPlayer->Close();
	}

	if (InternalMediaSound)
	{
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
		StartReadyMedia();
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
	if (bTerminal || PlaybackState != EPlaybackState::Starting)
	{
		return;
	}

	PlaybackState = EPlaybackState::Playing;
	OnPlaybackStarted.Broadcast(this);
}

void UBertaVideoPlayerWidget::HandleEndReached()
{
	if (bTerminal || (PlaybackState != EPlaybackState::Starting && PlaybackState != EPlaybackState::Playing))
	{
		return;
	}

	bTerminal = true;
	PlaybackState = EPlaybackState::Closed;
	OnPlaybackCompleted.Broadcast(this);
	CleanupMediaResources();
	if (Options.bRemoveOnCompletion)
	{
		RemoveFromParent();
	}
}
