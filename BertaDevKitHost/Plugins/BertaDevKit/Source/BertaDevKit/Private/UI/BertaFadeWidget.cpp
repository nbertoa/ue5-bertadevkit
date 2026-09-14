#include "UI/BertaFadeWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "HAL/PlatformTime.h"

namespace BertaFadeWidgetPrivate
{
const FName RootWidgetName(TEXT("BertaFadeRoot"));
const FName BlackVisualName(TEXT("BertaFadeBlackVisual"));
}

TSharedRef<SWidget> UBertaFadeWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	BlackVisual = Cast<UBorder>(WidgetTree->FindWidget(BertaFadeWidgetPrivate::BlackVisualName));
	if (!BlackVisual)
	{
		UWidget* ExistingRoot = WidgetTree->RootWidget;
		UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(),
			BertaFadeWidgetPrivate::RootWidgetName);

		if (ExistingRoot)
		{
			if (UOverlaySlot* ContentSlot = RootOverlay->AddChildToOverlay(ExistingRoot))
			{
				ContentSlot->SetHorizontalAlignment(HAlign_Fill);
				ContentSlot->SetVerticalAlignment(VAlign_Fill);
			}
		}

		BlackVisual = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			BertaFadeWidgetPrivate::BlackVisualName);
		BlackVisual->SetBrushColor(FLinearColor::Black);
		BlackVisual->SetPadding(FMargin(0.0f));
		BlackVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
		BlackVisual->SetRenderOpacity(FadeType == EBertaFadeType::FadeOut ? 0.0f : 1.0f);

		if (UOverlaySlot* FadeSlot = RootOverlay->AddChildToOverlay(BlackVisual))
		{
			FadeSlot->SetHorizontalAlignment(HAlign_Fill);
			FadeSlot->SetVerticalAlignment(VAlign_Fill);
		}

		WidgetTree->RootWidget = RootOverlay;
	}

	return Super::RebuildWidget();
}

void UBertaFadeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsDesignTime())
	{
		PlayFade();
	}
}

void UBertaFadeWidget::NativeDestruct()
{
	bIsFading = false;
	Super::NativeDestruct();
}

void UBertaFadeWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsFading || !BlackVisual)
	{
		return;
	}

	const double ElapsedSeconds = FMath::Max(0.0, FPlatformTime::Seconds() - FadeStartTime);
	const float Alpha = FMath::Clamp(static_cast<float>(ElapsedSeconds / ActiveDuration), 0.0f, 1.0f);
	BlackVisual->SetRenderOpacity(FMath::Lerp(StartOpacity, EndOpacity, Alpha));

	if (Alpha >= 1.0f)
	{
		FinishFade();
	}
}

void UBertaFadeWidget::PlayFade()
{
	StartOpacity = FadeType == EBertaFadeType::FadeOut ? 0.0f : 1.0f;
	EndOpacity = FadeType == EBertaFadeType::FadeOut ? 1.0f : 0.0f;
	ActiveDuration = FMath::Max(0.0f, Duration);
	bRemoveWhenFinished = bRemoveOnFinished;
	bIsFading = true;

	if (BlackVisual)
	{
		BlackVisual->SetRenderOpacity(StartOpacity);
	}

	if (ActiveDuration <= 0.0f)
	{
		FinishFade();
		return;
	}

	FadeStartTime = FPlatformTime::Seconds();
}

void UBertaFadeWidget::FinishFade()
{
	if (!bIsFading)
	{
		return;
	}

	bIsFading = false;
	if (BlackVisual)
	{
		BlackVisual->SetRenderOpacity(EndOpacity);
	}

	const bool bShouldRemove = bRemoveWhenFinished;
	OnFadeFinished.Broadcast(this);
	if (bShouldRemove)
	{
		RemoveFromParent();
	}
}
