#include "UI/BertaImagePlayerWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Engine/Texture2D.h"
#include "HAL/PlatformTime.h"
#include "Log/BertaDevKitLog.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/UObjectGlobals.h"

namespace BertaImagePlayerWidgetPrivate
{
const FName RootWidgetName(TEXT("BertaImageRoot"));
const FName ImageWidgetName(TEXT("BertaImageVisual"));
}

TSharedRef<SWidget> UBertaImagePlayerWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	ImageVisual = Cast<UImage>(WidgetTree->FindWidget(BertaImagePlayerWidgetPrivate::ImageWidgetName));
	if (!ImageVisual)
	{
		UWidget* ExistingRoot = WidgetTree->RootWidget;
		UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(),
			BertaImagePlayerWidgetPrivate::RootWidgetName);
		ImageVisual = WidgetTree->ConstructWidget<UImage>(
			UImage::StaticClass(),
			BertaImagePlayerWidgetPrivate::ImageWidgetName);
		ImageVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
		ImageVisual->SetRenderOpacity(0.0f);

		UOverlaySlot* ImageSlot = RootOverlay->AddChildToOverlay(ImageVisual);
		check(ImageSlot);
		ImageSlot->SetHorizontalAlignment(HAlign_Fill);
		ImageSlot->SetVerticalAlignment(VAlign_Fill);

		if (ExistingRoot)
		{
			UOverlaySlot* ContentSlot = RootOverlay->AddChildToOverlay(ExistingRoot);
			check(ContentSlot);
			ContentSlot->SetHorizontalAlignment(HAlign_Fill);
			ContentSlot->SetVerticalAlignment(VAlign_Fill);
		}

		WidgetTree->RootWidget = RootOverlay;
	}

	return Super::RebuildWidget();
}

void UBertaImagePlayerWidget::NativeConstruct()
{
	TGuardValue<bool> ConstructGuard(bIsConstructing, true);
	Super::NativeConstruct();

	if (!IsDesignTime() && PlaybackState == EPlaybackState::Inactive && Options.bAutoPlay)
	{
		Play();
	}
}

void UBertaImagePlayerWidget::NativeDestruct()
{
	// Like the video utility, external removal cancels a nonterminal sequence.
	// Re-adding that instance may start again; completion and Close remain terminal.
	if (PlaybackState != EPlaybackState::Completed && PlaybackState != EPlaybackState::Closed)
	{
		PlaybackState = EPlaybackState::Inactive;
	}
	ClearVisual();
	// Destruct listeners cannot start playback before the next visual construction.
	ImageVisual = nullptr;
	Super::NativeDestruct();
}

void UBertaImagePlayerWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (PlaybackState == EPlaybackState::Closed
		|| (PlaybackState == EPlaybackState::Completed && ActiveOptions.bRemoveOnCompletion))
	{
		RemoveWhenReady();
		return;
	}
	AdvancePlayback(FPlatformTime::Seconds());
}

bool UBertaImagePlayerWidget::Play()
{
	switch (PlaybackState)
	{
	case EPlaybackState::FadingIn:
	case EPlaybackState::Displaying:
	case EPlaybackState::FadingOut:
		return true;

	case EPlaybackState::Completed:
	case EPlaybackState::Closed:
		return false;

	default:
		break;
	}

	if (!IsValid(Texture))
	{
		UE_LOG(LogBertaDevKit, Warning, TEXT("[BertaImagePlayerWidget] '%s': No valid Texture was configured."), *GetName());
		Close();
		return false;
	}

	if (!ImageVisual)
	{
		UE_LOG(LogBertaDevKit, Warning, TEXT("[BertaImagePlayerWidget] '%s': Play requires a constructed native image visual; add the widget first."), *GetName());
		return false;
	}

	if (!FMath::IsFinite(Options.DisplayDuration)
		|| (Options.bUseFadeIn && !FMath::IsFinite(Options.FadeInDuration))
		|| (Options.bUseFadeOut && !FMath::IsFinite(Options.FadeOutDuration)))
	{
		UE_LOG(LogBertaDevKit, Warning, TEXT("[BertaImagePlayerWidget] '%s': Display and enabled fade durations must be finite."), *GetName());
		Close();
		return false;
	}

	ActiveOptions = Options;
	ActiveOptions.FadeInDuration = ActiveOptions.bUseFadeIn ? FMath::Max(0.0f, ActiveOptions.FadeInDuration) : 0.0f;
	ActiveOptions.DisplayDuration = FMath::Max(0.0f, ActiveOptions.DisplayDuration);
	ActiveOptions.FadeOutDuration = ActiveOptions.bUseFadeOut ? FMath::Max(0.0f, ActiveOptions.FadeOutDuration) : 0.0f;
	ImageVisual->SetBrushFromTexture(Texture);
	ImageVisual->SetRenderOpacity(ActiveOptions.bUseFadeIn ? 0.0f : 1.0f);
	PlaybackState = EPlaybackState::FadingIn;
	StateStartTime = FPlatformTime::Seconds();
	AdvancePlayback(StateStartTime);
	return true;
}

void UBertaImagePlayerWidget::Close()
{
	PlaybackState = EPlaybackState::Closed;
	ClearVisual();
	RemoveWhenReady();
}

void UBertaImagePlayerWidget::AdvancePlayback(const double Now)
{
	// At most three forward transitions. Zero-duration stages finish in this call.
	for (;;)
	{
		const double ElapsedSeconds = FMath::Max(0.0, Now - StateStartTime);
		switch (PlaybackState)
		{
		case EPlaybackState::FadingIn:
			check(ImageVisual);
			if (ElapsedSeconds < ActiveOptions.FadeInDuration)
			{
				ImageVisual->SetRenderOpacity(static_cast<float>(ElapsedSeconds / ActiveOptions.FadeInDuration));
				return;
			}

			ImageVisual->SetRenderOpacity(1.0f);
			PlaybackState = EPlaybackState::Displaying;
			// Start the hold when the image actually reaches 1.0. A late fade tick
			// must not consume any of the fully visible display duration.
			StateStartTime = Now;
			break;

		case EPlaybackState::Displaying:
			check(ImageVisual);
			if (ElapsedSeconds < ActiveOptions.DisplayDuration)
			{
				return;
			}

			PlaybackState = EPlaybackState::FadingOut;
			StateStartTime = Now;
			break;

		case EPlaybackState::FadingOut:
			check(ImageVisual);
			if (ElapsedSeconds < ActiveOptions.FadeOutDuration)
			{
				ImageVisual->SetRenderOpacity(1.0f - static_cast<float>(ElapsedSeconds / ActiveOptions.FadeOutDuration));
				return;
			}

			ImageVisual->SetRenderOpacity(ActiveOptions.bUseFadeOut ? 0.0f : 1.0f);
			CompletePlayback();
			return;

		default:
			return;
		}
	}
}

void UBertaImagePlayerWidget::CompletePlayback()
{
	if (PlaybackState != EPlaybackState::FadingOut)
	{
		return;
	}

	// Establish terminal state before calling listeners, including reentrant Play/Close.
	PlaybackState = EPlaybackState::Completed;
	const bool bShouldRemove = ActiveOptions.bRemoveOnCompletion;
	OnDisplayCompleted.Broadcast(this);
	if (bShouldRemove && PlaybackState == EPlaybackState::Completed)
	{
		RemoveWhenReady();
	}
}

void UBertaImagePlayerWidget::ClearVisual()
{
	if (ImageVisual)
	{
		ImageVisual->SetRenderOpacity(0.0f);
		ImageVisual->SetBrushResourceObject(nullptr);
	}
}

void UBertaImagePlayerWidget::RemoveWhenReady()
{
	// UE 5.8 attaches the viewport container AFTER TakeWidget calls NativeConstruct.
	// Removing during Construct would unregister it before that attachment, leaving
	// an unmanaged container. Terminal playback removes on the first Slate tick instead.
	if (!bIsConstructing)
	{
		RemoveFromParent();
	}
}
