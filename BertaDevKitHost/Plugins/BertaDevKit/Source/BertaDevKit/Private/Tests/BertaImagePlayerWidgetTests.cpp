#include "UI/BertaImagePlayerWidget.h"

#include "Tests/BertaImagePlayerWidgetTestReceiver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Engine/Texture2D.h"
#include "Misc/AutomationTest.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaImagePlayerWidgetPlaybackTest,
	"BertaDevKit.UI.ImagePlayer.PlaybackContracts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaImagePlayerWidgetPlaybackTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UTexture2D> Texture(NewObject<UTexture2D>());
	TStrongObjectPtr<UBertaImagePlayerWidgetTestReceiver> Receiver(NewObject<UBertaImagePlayerWidgetTestReceiver>());
	auto MakeWidget = [&]()
	{
		TStrongObjectPtr<UBertaImagePlayerWidget> Widget(NewObject<UBertaImagePlayerWidget>());
		// Exercise the real state machine without creating Slate resources or a world.
		Widget->ImageVisual = NewObject<UImage>(Widget.Get());
		Widget->Texture = Texture.Get();
		Widget->Options.bAutoPlay = false;
		Widget->Options.bRemoveOnCompletion = false;
		Receiver->CompletionCount = 0;
		Receiver->bPlayAcceptedDuringCompletion = false;
		Receiver->bCloseOnCompletion = false;
		Widget->OnDisplayCompleted.AddDynamic(Receiver.Get(), &UBertaImagePlayerWidgetTestReceiver::RecordCompletion);
		return Widget;
	};

	{
		auto Widget = MakeWidget();
		Widget->Options.FadeInDuration = 1.0f;
		Widget->Options.DisplayDuration = 4.0f;
		Widget->Options.FadeOutDuration = 1.0f;
		TestTrue(TEXT("Inactive playback can start"), Widget->Play());
		Widget->AdvancePlayback(1000.0); // First Slate update anchors the fade clock.
		TestEqual(TEXT("Fade-in starts transparent"), Widget->ImageVisual->GetRenderOpacity(), 0.0f);
		Widget->AdvancePlayback(1000.5);
		TestEqual(TEXT("Fade-in is linear"), Widget->ImageVisual->GetRenderOpacity(), 0.5f);
		TestTrue(TEXT("Play during playback is idempotent"), Widget->Play());
		TestEqual(TEXT("Repeated Play preserves the stage clock"), Widget->StateStartTime, 1000.0);
		Widget->AdvancePlayback(1001.0);
		TestTrue(TEXT("Fade-in transitions to display"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Displaying);
		TestEqual(TEXT("Display starts exactly fully visible"), Widget->ImageVisual->GetRenderOpacity(), 1.0f);

		Widget->Options.DisplayDuration = 0.0f;
		Widget->Options.bUseFadeOut = false;
		Widget->Options.bRemoveOnCompletion = true;
		Widget->Texture = nullptr;
		Widget->AdvancePlayback(1004.999);
		TestEqual(TEXT("The full four seconds exclude fade-in"), Widget->ImageVisual->GetRenderOpacity(), 1.0f);
		TestEqual(TEXT("No completion during display"), Receiver->CompletionCount, 0);
		TestTrue(TEXT("The brush retains the originally sampled texture"), Widget->ImageVisual->GetBrush().GetResourceObject() == Texture.Get());
		Widget->AdvancePlayback(1005.0);
		TestTrue(TEXT("Display transitions to fade-out"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::FadingOut);
		TestEqual(TEXT("Fade-out starts fully visible"), Widget->ImageVisual->GetRenderOpacity(), 1.0f);
		Widget->AdvancePlayback(1005.5);
		TestEqual(TEXT("Fade-out is linear and uses captured options"), Widget->ImageVisual->GetRenderOpacity(), 0.5f);
		Widget->AdvancePlayback(1006.0);
		TestTrue(TEXT("Completion is terminal"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Completed);
		TestEqual(TEXT("Fade-out ends exactly transparent"), Widget->ImageVisual->GetRenderOpacity(), 0.0f);
		TestEqual(TEXT("Completion is emitted once"), Receiver->CompletionCount, 1);
		TestFalse(TEXT("Play cannot restart completed playback"), Widget->Play());
		TestFalse(TEXT("The completion listener sees terminal state"), Receiver->bPlayAcceptedDuringCompletion);
		Widget->AdvancePlayback(1010.0);
		Widget->CompletePlayback();
		Widget->Close();
		Widget->Close();
		Widget->NativeDestruct();
		TestEqual(TEXT("Later updates, Close, and destruction cannot duplicate completion"), Receiver->CompletionCount, 1);
		TestFalse(TEXT("Close prevents future playback"), Widget->Play());
	}

	{
		auto Widget = MakeWidget();
		Widget->Options.FadeInDuration = 1.0f;
		Widget->Options.DisplayDuration = 3.0f;
		Widget->Options.FadeOutDuration = 1.0f;
		TestTrue(TEXT("Late-tick scenario starts"), Widget->Play());
		Widget->AdvancePlayback(1000.0);
		Widget->AdvancePlayback(1010.0);
		TestTrue(TEXT("A late fade-in tick starts the fully visible hold"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Displaying);
		TestEqual(TEXT("The hold clock starts when opacity reaches one"), Widget->StateStartTime, 1010.0);
		Widget->AdvancePlayback(1012.999);
		TestEqual(TEXT("A late fade tick cannot shorten the hold"), Widget->ImageVisual->GetRenderOpacity(), 1.0f);
		TestEqual(TEXT("Late fade-in does not skip completion stages"), Receiver->CompletionCount, 0);
		Widget->AdvancePlayback(1013.0);
		TestTrue(TEXT("The full display interval precedes fade-out"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::FadingOut);
		Widget->AdvancePlayback(1014.0);
		TestEqual(TEXT("Late-tick scenario completes once"), Receiver->CompletionCount, 1);
	}

	{
		auto Widget = MakeWidget();
		Widget->Options.bUseFadeIn = false;
		Widget->Options.bUseFadeOut = false;
		Widget->Options.FadeInDuration = 10.0f;
		Widget->Options.FadeOutDuration = 10.0f;
		Widget->Options.DisplayDuration = 3.0f;
		TestTrue(TEXT("Disabled fades start immediately"), Widget->Play());
		Widget->StateStartTime = 1000.0;
		TestTrue(TEXT("Disabled fade-in goes directly to display"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Displaying);
		TestEqual(TEXT("No fade-in means full opacity immediately"), Widget->ImageVisual->GetRenderOpacity(), 1.0f);
		Widget->AdvancePlayback(1002.999);
		TestEqual(TEXT("Disabled fades do not reduce the hold"), Receiver->CompletionCount, 0);
		Widget->AdvancePlayback(1003.0);
		TestEqual(TEXT("No fade-out completes at the end of display"), Receiver->CompletionCount, 1);
		TestEqual(TEXT("Without removal or fade-out the completed image remains visible"), Widget->ImageVisual->GetRenderOpacity(), 1.0f);
	}

	{
		auto Widget = MakeWidget();
		Widget->Options.FadeInDuration = 0.0f;
		Widget->Options.DisplayDuration = 0.0f;
		Widget->Options.FadeOutDuration = 0.0f;
		TestTrue(TEXT("All-zero playback completes synchronously"), Widget->Play());
		TestEqual(TEXT("All-zero playback emits exactly one completion"), Receiver->CompletionCount, 1);
		TestEqual(TEXT("All-zero fade-out reaches zero opacity"), Widget->ImageVisual->GetRenderOpacity(), 0.0f);
		Widget->AdvancePlayback(1000.0);
		TestFalse(TEXT("All-zero playback cannot restart"), Widget->Play());
		TestEqual(TEXT("All-zero playback has no duplicate completion"), Receiver->CompletionCount, 1);
	}

	{
		auto Widget = MakeWidget();
		Widget->Options.FadeInDuration = 0.0f;
		Widget->Options.DisplayDuration = 0.0f;
		Widget->Options.FadeOutDuration = 1.0f;
		TestTrue(TEXT("Zero fade-in and display still allow fade-out"), Widget->Play());
		Widget->StateStartTime = 1000.0;
		TestTrue(TEXT("Zero display immediately enters fade-out"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::FadingOut);
		Widget->AdvancePlayback(1000.5);
		TestEqual(TEXT("Fade-out after zero display remains linear"), Widget->ImageVisual->GetRenderOpacity(), 0.5f);
		Widget->AdvancePlayback(1001.0);
		TestEqual(TEXT("Mixed zero-duration playback completes once"), Receiver->CompletionCount, 1);
	}

	{
		auto Widget = MakeWidget();
		Widget->Options.FadeInDuration = -1.0f;
		Widget->Options.DisplayDuration = -1.0f;
		Widget->Options.FadeOutDuration = -1.0f;
		TestTrue(TEXT("Negative durations clamp to immediate"), Widget->Play());
		TestEqual(TEXT("Clamped durations complete once"), Receiver->CompletionCount, 1);
	}

	{
		auto Widget = MakeWidget();
		Widget->Texture = nullptr;
		AddExpectedMessage(TEXT("No valid Texture was configured."), EAutomationExpectedMessageFlags::Contains, 1);
		TestFalse(TEXT("Missing texture explicitly fails"), Widget->Play());
		TestTrue(TEXT("Missing texture closes the widget"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Closed);
		TestEqual(TEXT("Failure does not emit natural completion"), Receiver->CompletionCount, 0);
		TestEqual(TEXT("Failure leaves the visual transparent"), Widget->ImageVisual->GetRenderOpacity(), 0.0f);
		Widget->Texture = Texture.Get();
		TestFalse(TEXT("Failure remains terminal after changing input"), Widget->Play());
	}

	{
		auto Widget = MakeWidget();
		TestTrue(TEXT("Manual cancellation scenario starts"), Widget->Play());
		Widget->Close();
		Widget->Close();
		Widget->AdvancePlayback(100000.0);
		TestEqual(TEXT("Close makes the image transparent"), Widget->ImageVisual->GetRenderOpacity(), 0.0f);
		TestTrue(TEXT("Close releases the brush resource"), Widget->ImageVisual->GetBrush().GetResourceObject() == nullptr);
		Widget->NativeDestruct();
		TestEqual(TEXT("Manual Close is idempotent and emits no completion"), Receiver->CompletionCount, 0);
		TestFalse(TEXT("Closed playback cannot restart"), Widget->Play());
	}

	{
		auto Widget = MakeWidget();
		Widget->Options.FadeInDuration = 0.0f;
		Widget->Options.DisplayDuration = 0.0f;
		Widget->Options.FadeOutDuration = 0.0f;
		Widget->Options.bRemoveOnCompletion = true;
		Receiver->bCloseOnCompletion = true;
		TestTrue(TEXT("A completion callback may close immediately"), Widget->Play());
		TestEqual(TEXT("Reentrant Close cannot duplicate completion"), Receiver->CompletionCount, 1);
		TestFalse(TEXT("Reentrant Play is rejected during completion"), Receiver->bPlayAcceptedDuringCompletion);
		TestTrue(TEXT("Reentrant Close leaves a closed terminal state"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Closed);
		Widget->CompletePlayback();
		Widget->Close();
		TestEqual(TEXT("Repeated cleanup remains silent"), Receiver->CompletionCount, 1);
	}

	{
		auto Widget = MakeWidget();
		TestTrue(TEXT("External removal scenario starts"), Widget->Play());
		Widget->NativeDestruct();
		TestTrue(TEXT("External destruction cancels nonterminal playback"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Inactive);
		Widget->AdvancePlayback(100000.0);
		TestEqual(TEXT("External destruction does not emit completion"), Receiver->CompletionCount, 0);
		AddExpectedMessage(TEXT("Play requires a constructed native image visual"), EAutomationExpectedMessageFlags::Contains, 1);
		TestFalse(TEXT("Destructed playback must wait for visual reconstruction"), Widget->Play());
		Widget->Close();
	}

	{
		auto Widget = MakeWidget();
		Widget->Options.DisplayDuration = std::numeric_limits<float>::infinity();
		AddExpectedMessage(TEXT("Display and enabled fade durations must be finite."), EAutomationExpectedMessageFlags::Contains, 1);
		TestFalse(TEXT("Non-finite display duration fails safely"), Widget->Play());
		TestTrue(TEXT("Non-finite duration is terminal"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Closed);
		TestEqual(TEXT("Non-finite duration emits no completion"), Receiver->CompletionCount, 0);
	}

	{
		auto Widget = MakeWidget();
		Widget->Options.bUseFadeIn = false;
		Widget->Options.bUseFadeOut = false;
		Widget->Options.FadeInDuration = std::numeric_limits<float>::quiet_NaN();
		Widget->Options.FadeOutDuration = std::numeric_limits<float>::infinity();
		Widget->Options.DisplayDuration = 0.0f;
		TestTrue(TEXT("Disabled fade durations are ignored even when non-finite"), Widget->Play());
		TestEqual(TEXT("Disabled non-finite fades complete once"), Receiver->CompletionCount, 1);
	}

	{
		auto Widget = MakeWidget();
		Widget->ImageVisual = nullptr;
		AddExpectedMessage(TEXT("Play requires a constructed native image visual"), EAutomationExpectedMessageFlags::Contains, 1);
		TestFalse(TEXT("Play before visual construction fails explicitly"), Widget->Play());
		TestTrue(TEXT("Missing visual leaves playback inactive"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Inactive);
		Widget->ImageVisual = NewObject<UImage>(Widget.Get());
		TestTrue(TEXT("Play can be retried after visual construction"), Widget->Play());
		Widget->Close();
	}

	{
		auto Widget = MakeWidget();
		TStrongObjectPtr<UOverlay> Parent(NewObject<UOverlay>());
		Parent->AddChildToOverlay(Widget.Get());
		Widget->Options.FadeInDuration = 0.0f;
		Widget->Options.DisplayDuration = 0.0f;
		Widget->Options.FadeOutDuration = 0.0f;
		Widget->Options.bRemoveOnCompletion = true;
		Widget->bIsConstructing = true;
		TestTrue(TEXT("All-zero playback can complete during construction"), Widget->Play());
		TestEqual(TEXT("Construction completion still broadcasts immediately once"), Receiver->CompletionCount, 1);
		TestTrue(TEXT("Removal waits for construction to finish"), Widget->GetParent() == Parent.Get());
		Widget->bIsConstructing = false;
		Widget->RemoveWhenReady();
		TestTrue(TEXT("Deferred completion removal detaches from its parent"), Widget->GetParent() == nullptr);
		TestEqual(TEXT("Deferred removal does not rebroadcast completion"), Receiver->CompletionCount, 1);
	}

	{
		auto Widget = MakeWidget();
		TStrongObjectPtr<UOverlay> Parent(NewObject<UOverlay>());
		Parent->AddChildToOverlay(Widget.Get());
		Widget->bIsConstructing = true;
		Widget->Close();
		Widget->Close();
		TestTrue(TEXT("Close during construction defers only removal"), Widget->GetParent() == Parent.Get());
		TestTrue(TEXT("Close is already terminal during construction"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Closed);
		Widget->bIsConstructing = false;
		Widget->RemoveWhenReady();
		TestTrue(TEXT("Deferred Close detaches from its parent"), Widget->GetParent() == nullptr);
		TestEqual(TEXT("Deferred Close emits no natural completion"), Receiver->CompletionCount, 0);
	}

	return true;
}

#endif

void UBertaImagePlayerWidgetTestReceiver::RecordCompletion(UBertaImagePlayerWidget* Widget)
{
#if WITH_DEV_AUTOMATION_TESTS
	++CompletionCount;
	bPlayAcceptedDuringCompletion = Widget->Play();
	if (bReplaceOnCompletion)
	{
		bReplaceOnCompletion = false;
		Widget->Options.bRemoveOnCompletion = false;
		Widget->SetTexture(ReplacementTexture);
	}
	if (bCloseOnCompletion)
	{
		Widget->Close();
	}
#endif
}
