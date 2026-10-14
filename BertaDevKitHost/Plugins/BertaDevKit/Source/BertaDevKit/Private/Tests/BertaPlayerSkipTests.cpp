#include "UI/BertaImagePlayerWidget.h"
#include "UI/BertaVideoPlayerWidget.h"
#include "Tests/BertaImagePlayerWidgetTestReceiver.h"
#include "Tests/BertaVideoPlayerWidgetTestReceiver.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Components/AudioComponent.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Engine/Texture2D.h"
#include "FileMediaSource.h"
#include "HAL/PlatformTime.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaTexture.h"
#include "Misc/AutomationTest.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/SWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaImagePlayerWidgetSkipTest,
	"BertaDevKit.UI.ImagePlayer.SkipContracts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaImagePlayerWidgetSkipTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UTexture2D> A(NewObject<UTexture2D>());
	TStrongObjectPtr<UTexture2D> B(NewObject<UTexture2D>());
	TStrongObjectPtr<UOverlay> Parent(NewObject<UOverlay>());
	TStrongObjectPtr<UBertaImagePlayerWidgetTestReceiver> Receiver(NewObject<UBertaImagePlayerWidgetTestReceiver>());
	TSharedPtr<SWidget> Slate;
	auto MakeWidget = [&]()
	{
		Slate.Reset();
		Receiver->CompletionCount = 0;
		Receiver->bReplaceOnCompletion = false;
		TStrongObjectPtr<UBertaImagePlayerWidget> Widget(NewObject<UBertaImagePlayerWidget>());
		Widget->Options.bAutoPlay = false;
		Widget->Options.bRemoveOnCompletion = false;
		Widget->Options.FadeInDuration = 1.0f;
		Widget->Options.DisplayDuration = 10.0f;
		Widget->Options.FadeOutDuration = 1.0f;
		Widget->SetTexture(A.Get());
		Parent->AddChildToOverlay(Widget.Get());
		Slate = Widget->TakeWidget(); // Real native UMG/Slate hierarchy, no visual injection.
		Widget->OnDisplayCompleted.AddDynamic(Receiver.Get(), &UBertaImagePlayerWidgetTestReceiver::RecordCompletion);
		return Widget;
	};
	{
		auto Widget = MakeWidget();
		TestFalse(TEXT("Inactive Skip is silent"), Widget->SkipToFadeOut());
		Widget->Play();
		Widget->AdvancePlayback(100.0);
		Widget->AdvancePlayback(101.0);
		Widget->Options.bUseFadeOut = false; // The exit must use captured options.
		TestTrue(TEXT("Displaying Skip abandons the hold immediately"), Widget->SkipToFadeOut());
		TestTrue(TEXT("Skip enters FadingOut"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::FadingOut);
		TestEqual(TEXT("Full-opacity exit keeps captured duration"), Widget->ActiveFadeOutDuration, 1.0f);
		const double ExitStart = Widget->StateStartTime;
		Widget->SkipToFadeOut();
		TestEqual(TEXT("Repeated Skip cannot reset the exit clock"), Widget->StateStartTime, ExitStart);
		Widget->AdvancePlayback(ExitStart + 0.5);
		TestEqual(TEXT("Exit progresses without waiting ten display seconds"), Widget->ImageVisual->GetRenderOpacity(), 0.5f);
		Widget->AdvancePlayback(ExitStart + 1.0);
		Widget->AdvancePlayback(ExitStart + 20.0);
		TestEqual(TEXT("Skipped completion is exactly once"), Receiver->CompletionCount, 1);
		TestTrue(TEXT("Removal false preserves the parent"), Widget->GetParent() == Parent.Get());
		TestFalse(TEXT("Completed Skip returns false"), Widget->SkipToFadeOut());
	}
	{
		auto Widget = MakeWidget();
		Widget->Play();
		Widget->AdvancePlayback(100.0);
		// Use the public UMG API that synchronizes the constructed Slate visual.
		Widget->ImageVisual->SetRenderOpacity(0.3f);
		TestEqual(TEXT("Partial opacity is synchronized into real Slate"), Widget->ImageVisual->GetCachedWidget()->GetRenderOpacity(), 0.3f);
		Widget->SkipToFadeOut();
		TestEqual(TEXT("Fade-in skip never jumps to full brightness"), Widget->ImageVisual->GetRenderOpacity(), 0.3f);
		TestEqual(TEXT("Fade speed gives opacity-proportional duration"), Widget->ActiveFadeOutDuration, 0.3f);
		const double ExitStart = Widget->StateStartTime;
		Widget->AdvancePlayback(ExitStart + 0.15);
		TestTrue(TEXT("Partial-alpha exit is linear"), FMath::IsNearlyEqual(Widget->ImageVisual->GetRenderOpacity(), 0.15f));
		Widget->AdvancePlayback(ExitStart + 0.301);
		TestEqual(TEXT("Partial-alpha exit completes without a full second delay"), Receiver->CompletionCount, 1);
	}
	{
		auto Widget = MakeWidget();
		Widget->Play();
		TestTrue(TEXT("Positive fade-in retains the transparent first-frame state"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::WaitingForFadeInTick);
		Widget->SkipToFadeOut();
		TestEqual(TEXT("Transparent waiting state completes immediately"), Receiver->CompletionCount, 1);
		TestEqual(TEXT("A transparent skip has no useless duration"), Widget->ActiveFadeOutDuration, 0.0f);
		TestEqual(TEXT("The constructed Slate image stays transparent"), Widget->ImageVisual->GetCachedWidget()->GetRenderOpacity(), 0.0f);
	}
	{
		auto Widget = MakeWidget();
		Widget->Options.bUseFadeIn = false;
		Widget->Play();
		Widget->AdvancePlayback(Widget->StateStartTime + 10.0);
		const double NaturalExitStart = Widget->StateStartTime;
		Widget->SkipToFadeOut();
		TestEqual(TEXT("Already-natural fade-out is not restarted"), Widget->StateStartTime, NaturalExitStart);
		TestEqual(TEXT("Idempotent Skip cannot complete early"), Receiver->CompletionCount, 0);
		Widget->AdvancePlayback(NaturalExitStart + 1.0);
	}
	for (int32 Case = 0; Case < 2; ++Case)
	{
		auto Widget = MakeWidget();
		Widget->Options.bUseFadeIn = false;
		Widget->Options.bUseFadeOut = Case != 0;
		Widget->Options.FadeOutDuration = Case == 0 ? 1.0f : 0.0f;
		Widget->Options.bRemoveOnCompletion = true;
		Widget->Play();
		Widget->SkipToFadeOut();
		TestEqual(TEXT("Disabled/zero exit completes synchronously"), Receiver->CompletionCount, 1);
		TestEqual(TEXT("Disabled exit preserves the normal fully-visible final semantics"), Widget->ImageVisual->GetRenderOpacity(), Case == 0 ? 1.0f : 0.0f);
		TestNull(TEXT("Captured removal is honored"), Widget->GetParent());
	}
	{
		auto Widget = MakeWidget();
		Widget->Options.bUseFadeIn = false;
		Widget->Play();
		Widget->SkipToFadeOut();
		Widget->Options.bAutoPlay = true;
		Widget->Options.bUseFadeIn = true;
		Widget->SetTexture(B.Get());
		Widget->AdvancePlayback(1000.0);
		TestTrue(TEXT("Replacement uses its own transparent startup"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::FadingIn);
		TestEqual(TEXT("Replacement first Slate update stays transparent"), Widget->ImageVisual->GetRenderOpacity(), 0.0f);
		TestEqual(TEXT("Interrupted exit cannot complete the replacement"), Receiver->CompletionCount, 0);
		TestTrue(TEXT("Replacement keeps its brush and parent"), Widget->ImageVisual->GetBrush().GetResourceObject() == B.Get() && Widget->GetParent() == Parent.Get());
	}
	{
		auto Widget = MakeWidget();
		Widget->Options.bRemoveOnCompletion = true;
		Widget->Play();
		Receiver->ReplacementTexture = B.Get();
		Receiver->bReplaceOnCompletion = true;
		Widget->Options.bAutoPlay = true;
		Widget->SkipToFadeOut(); // Immediate transparent exit calls the listener.
		TestEqual(TEXT("Reentrant replacement receives only the old completion"), Receiver->CompletionCount, 1);
		TestTrue(TEXT("Old removal cannot detach the new run"), Widget->GetParent() == Parent.Get() && Widget->Texture == B.Get());
	}
	{
		auto Widget = MakeWidget();
		Widget->Options.bUseFadeIn = false;
		Widget->Play();
		Widget->SkipToFadeOut();
		Widget->Close();
		Widget->AdvancePlayback(100000.0);
		TestEqual(TEXT("Close cancels a pending skipped completion"), Receiver->CompletionCount, 0);
		TestFalse(TEXT("Closed Skip returns false"), Widget->SkipToFadeOut());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaVideoPlayerWidgetSkipTest,
	"BertaDevKit.UI.VideoPlayer.SkipContracts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaVideoPlayerWidgetSkipTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UFileMediaSource> A(NewObject<UFileMediaSource>());
	TStrongObjectPtr<UFileMediaSource> B(NewObject<UFileMediaSource>());
	TStrongObjectPtr<UOverlay> Parent(NewObject<UOverlay>());
	TStrongObjectPtr<UBertaVideoPlayerWidgetTestReceiver> Receiver(NewObject<UBertaVideoPlayerWidgetTestReceiver>());
	auto MakeWidget = [&]()
	{
		Receiver->CompletedCount = Receiver->StartedCount = Receiver->FailedCount = 0;
		Receiver->bReplaceOnCompletion = false;
		TStrongObjectPtr<UBertaVideoPlayerWidget> Widget(NewObject<UBertaVideoPlayerWidget>());
		Widget->SetMediaSource(A.Get());
		Widget->Options.bAutoPlay = false;
		Widget->Options.bPlayAudio = false;
		Widget->Options.bPauseGame = false;
		Widget->Options.bUseEndFade = true;
		Widget->Options.EndFadeDuration = 1.0f;
		Widget->Options.bRemoveOnCompletion = false;
		Widget->ActiveOptions = Widget->Options;
		Widget->VideoImage = NewObject<UImage>(Widget.Get());
		Widget->BlackVisual = NewObject<UBorder>(Widget.Get());
		Widget->BlackVisual->SetRenderOpacity(0.0f);
		FString Error;
		TestTrue(TEXT("Create real resources without opening a backend"), Widget->CreateMediaResources(Error));
		Widget->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::Playing;
		Parent->AddChildToOverlay(Widget.Get());
		Widget->OnPlaybackStarted.AddDynamic(Receiver.Get(), &UBertaVideoPlayerWidgetTestReceiver::Started);
		Widget->OnPlaybackCompleted.AddDynamic(Receiver.Get(), &UBertaVideoPlayerWidgetTestReceiver::Completed);
		Widget->OnPlaybackFailed.AddDynamic(Receiver.Get(), &UBertaVideoPlayerWidgetTestReceiver::Failed);
		return Widget;
	};
	{
		auto Widget = MakeWidget();
		TStrongObjectPtr<UMediaPlayer> OldPlayer(Widget->InternalMediaPlayer);
		TStrongObjectPtr<UMediaTexture> OldTexture(Widget->InternalMediaTexture);
		Widget->InternalExternalAudio = NewObject<UAudioComponent>(Widget.Get());
		Widget->InternalExternalAudio->SetPaused(true);
		Widget->InternalMediaSound = NewObject<UMediaSoundComponent>(Widget.Get());
		Widget->bOwnsPause = true;
		Widget->ActiveOptions.bLoop = true;
		Widget->bRestartingLoop = Widget->bLoopSeekPending = true;
		Widget->ApplyTransportPause(true, 100.0);
		Widget->bEndReachedWhilePaused = true;
		Widget->Options.bUseEndFade = false;
		Widget->Options.bRemoveOnCompletion = true;
		TestTrue(TEXT("Paused loop can enter its exit directly"), Widget->SkipToFadeOut());
		TestTrue(TEXT("Skip follows an explicit fade independent of natural EndReached"), Widget->PlaybackState == UBertaVideoPlayerWidget::EPlaybackState::FadingOutSkippedVideo);
		TestFalse(TEXT("Exit clears all transport pause and loop continuations"), Widget->bTransportPaused || Widget->bEndReachedWhilePaused || Widget->bLoopSeekPending || Widget->bRestartingLoop);
		TestEqual(TEXT("Exit resets the pause clock"), Widget->TransportPauseStartTime, 0.0);
		TestFalse(TEXT("External audio cannot stay active/paused"), Widget->InternalExternalAudio->IsActive() || Widget->InternalExternalAudio->bIsPaused);
		TestFalse(TEXT("Closed media cannot advance"), OldPlayer->IsPlaying());
		TestFalse(TEXT("Texture auto-clear is disabled for last-frame retention"), OldTexture->AutoClear);
		TestTrue(TEXT("Same visual texture survives until fully covered"), Widget->VideoImage->GetBrush().GetResourceObject() == OldTexture.Get());
		TestFalse(TEXT("Old media delegates are detached immediately"), OldPlayer->OnMediaOpened.IsBound() || OldPlayer->OnMediaOpenFailed.IsBound() || OldPlayer->OnPlaybackResumed.IsBound() || OldPlayer->OnEndReached.IsBound() || OldPlayer->OnSeekCompleted.IsBound());
		TestTrue(TEXT("Gameplay pause stays owned until terminal cleanup"), Widget->bOwnsPause);
		TestFalse(TEXT("Transport Pause cannot block an exit"), Widget->Pause());
		TestFalse(TEXT("Transport Resume cannot restart skipped media"), Widget->Resume());
		const double ExitStart = Widget->FadeStartTime;
		Widget->SkipToFadeOut();
		TestEqual(TEXT("Repeated Skip does not reset its fade timer"), Widget->FadeStartTime, ExitStart);
		Widget->HandlePlaybackResumed();
		Widget->HandleMediaOpened(TEXT("old"));
		Widget->HandleMediaOpenFailed(TEXT("old"));
		Widget->HandleSeekCompleted();
		Widget->HandleEndReached();
		TestEqual(TEXT("Old callbacks cannot complete or start during the cover"), Receiver->CompletedCount + Receiver->StartedCount, 0);
		Widget->AdvanceFade(ExitStart + 1.0);
		TestEqual(TEXT("Cover finish emits completion exactly once"), Receiver->CompletedCount, 1);
		TestFalse(TEXT("Terminal cleanup releases owned pause"), Widget->bOwnsPause);
		TestNull(TEXT("Terminal cleanup releases retained media"), Widget->InternalMediaPlayer.Get());
		TestTrue(TEXT("Level reveal follows normal completion"), Widget->PlaybackState == UBertaVideoPlayerWidget::EPlaybackState::FadingInLevel);
		const double RevealStart = Widget->FadeStartTime;
		Widget->SkipToFadeOut();
		TestEqual(TEXT("Skip during level reveal cannot restart it"), Widget->FadeStartTime, RevealStart);
		Widget->AdvanceFade(RevealStart + 1.0);
		TestTrue(TEXT("Captured removal false retains parent"), Widget->GetParent() == Parent.Get());
		TestFalse(TEXT("Already-completed Skip returns false"), Widget->SkipToFadeOut());
		TestEqual(TEXT("No duplicate completion"), Receiver->CompletedCount, 1);
	}
	{
		auto Widget = MakeWidget();
		Widget->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::FadingInVideo;
		Widget->BlackVisual->SetRenderOpacity(0.6f);
		Widget->SkipToFadeOut();
		TestEqual(TEXT("Reveal redirect never flashes transparent"), Widget->BlackVisual->GetRenderOpacity(), 0.6f);
		TestTrue(TEXT("Remaining alpha determines cover duration"), FMath::IsNearlyEqual(Widget->ActiveFadeDuration, 0.4f));
		Widget->AdvanceFade(Widget->FadeStartTime + 0.2);
		TestTrue(TEXT("Redirect remains linear from actual opacity"), FMath::IsNearlyEqual(Widget->BlackVisual->GetRenderOpacity(), 0.8f));
		Widget->Stop();
		Widget->AdvanceFade(100000.0);
		TestEqual(TEXT("Stop aborts skipped completion"), Receiver->CompletedCount, 0);
		TestFalse(TEXT("Stopped Skip returns false"), Widget->SkipToFadeOut());
	}
	{
		auto Widget = MakeWidget();
		Widget->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::FadingOutVideoAtEnd;
		Widget->ActiveFadeDuration = 1.0f;
		Widget->FadeStartOpacity = 0.0f;
		Widget->FadeEndOpacity = 1.0f;
		Widget->FadeStartTime = FPlatformTime::Seconds() - 0.25;
		Widget->BlackVisual->SetRenderOpacity(0.25f);
		const double NaturalExitStart = Widget->FadeStartTime;
		Widget->SkipToFadeOut();
		TestEqual(TEXT("An existing natural end fade keeps its clock"), Widget->FadeStartTime, NaturalExitStart);
		TestEqual(TEXT("Existing end fade keeps its current alpha"), Widget->BlackVisual->GetRenderOpacity(), 0.25f);
		Widget->AdvanceFade(NaturalExitStart + 1.0);
		TestEqual(TEXT("Adopted end fade completes without another native end event"), Receiver->CompletedCount, 1);
	}
	{
		auto Widget = MakeWidget();
		Widget->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::FadingOutVideoAtEnd;
		Widget->ActiveFadeDuration = 1.0f;
		Widget->FadeStartOpacity = 0.0f;
		Widget->FadeEndOpacity = 1.0f;
		const double PauseTime = FPlatformTime::Seconds() - 5.0;
		Widget->FadeStartTime = PauseTime - 0.25;
		Widget->BlackVisual->SetRenderOpacity(0.25f);
		Widget->ApplyTransportPause(true, PauseTime);
		const double OldClock = Widget->FadeStartTime;
		const double BeforeSkip = FPlatformTime::Seconds();
		Widget->SkipToFadeOut();
		const double AfterSkip = FPlatformTime::Seconds();
		const double Shift = Widget->FadeStartTime - OldClock;
		TestTrue(TEXT("Paused natural exit excludes exactly the elapsed pause interval"), Shift >= BeforeSkip - PauseTime - 0.00001 && Shift <= AfterSkip - PauseTime + 0.00001);
		TestEqual(TEXT("Paused natural exit retains its duration"), Widget->ActiveFadeDuration, 1.0f);
		TestEqual(TEXT("Paused natural exit retains its visible progress"), Widget->BlackVisual->GetRenderOpacity(), 0.25f);
		Widget->AdvanceFade(Widget->FadeStartTime + 1.0);
		TestEqual(TEXT("Paused natural exit completes without Resume"), Receiver->CompletedCount, 1);
	}
	{
		auto Widget = MakeWidget();
		Widget->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::WaitingForEndBehindBlack;
		Widget->BlackVisual->SetRenderOpacity(1.0f);
		Widget->SkipToFadeOut();
		TestEqual(TEXT("Already-covered video no longer waits for EndReached"), Receiver->CompletedCount, 1);
		TestTrue(TEXT("Covered Skip goes directly to the level reveal"), Widget->PlaybackState == UBertaVideoPlayerWidget::EPlaybackState::FadingInLevel);
	}
	for (int32 Case = 0; Case < 2; ++Case)
	{
		auto Widget = MakeWidget();
		Widget->ActiveOptions.bUseEndFade = Case != 0;
		Widget->ActiveOptions.EndFadeDuration = Case == 0 ? 1.0f : 0.0f;
		Widget->ActiveOptions.bRemoveOnCompletion = true;
		Widget->SkipToFadeOut();
		TestEqual(TEXT("Disabled/zero video exit completes once immediately"), Receiver->CompletedCount, 1);
		TestNull(TEXT("Immediate exit honors removal"), Widget->GetParent());
		TestEqual(TEXT("Immediate exit does not leave black"), Widget->BlackVisual->GetRenderOpacity(), 0.0f);
		Widget->HandleEndReached();
		TestEqual(TEXT("Late end callback cannot duplicate immediate completion"), Receiver->CompletedCount, 1);
	}
	const UBertaVideoPlayerWidget::EPlaybackState Preplay[] = {
		UBertaVideoPlayerWidget::EPlaybackState::Opening,
		UBertaVideoPlayerWidget::EPlaybackState::Ready,
		UBertaVideoPlayerWidget::EPlaybackState::FadingOutLevel,
		UBertaVideoPlayerWidget::EPlaybackState::Starting,
		UBertaVideoPlayerWidget::EPlaybackState::StartingBehindBlack
	};
	for (auto State : Preplay)
	{
		auto Widget = MakeWidget();
		Widget->PlaybackState = State;
		Widget->BlackVisual->SetRenderOpacity(0.4f);
		Widget->SkipToFadeOut();
		TestEqual(TEXT("Pre-play exit completes intentionally without starting media"), Receiver->CompletedCount, 1);
		TestEqual(TEXT("Pre-play exit preserves partial black rather than forcing full black"), Widget->BlackVisual->GetRenderOpacity(), 0.4f);
		TestEqual(TEXT("Pre-play reveal uses proportional duration"), Widget->ActiveFadeDuration, 0.4f);
		Widget->HandleMediaOpened(TEXT("late"));
		Widget->HandlePlaybackResumed();
		TestEqual(TEXT("Pre-play Skip never emits Started"), Receiver->StartedCount, 0);
		Widget->AdvanceFade(Widget->FadeStartTime + 0.401);
		TestEqual(TEXT("Pre-play exit restores the level"), Widget->BlackVisual->GetRenderOpacity(), 0.0f);
	}
	{
		auto Widget = MakeWidget();
		Widget->SkipToFadeOut();
		Widget->SetMediaSource(B.Get()); // Preconstruction test context avoids opening a backend.
		Widget->AdvanceFade(100000.0);
		TestEqual(TEXT("Replacement cancels the prior skipped completion"), Receiver->CompletedCount, 0);
		TestTrue(TEXT("Replacement keeps its source/parent"), Widget->MediaSource == B.Get() && Widget->GetParent() == Parent.Get());
	}
	{
		auto Widget = MakeWidget();
		Widget->ActiveOptions.bRemoveOnCompletion = true;
		Widget->Options.bRemoveOnCompletion = false;
		Receiver->ReplacementSource = B.Get();
		Receiver->bReplaceOnCompletion = true;
		Widget->SkipToFadeOut();
		Widget->AdvanceFade(Widget->FadeStartTime + 1.0);
		TestEqual(TEXT("Reentrant replacement sees one completion"), Receiver->CompletedCount, 1);
		TestTrue(TEXT("Old exit cannot reveal/remove the replacement"), Widget->PlaybackState == UBertaVideoPlayerWidget::EPlaybackState::Inactive && Widget->GetParent() == Parent.Get() && Widget->MediaSource == B.Get());
	}
	{
		auto Widget = MakeWidget();
		Widget->SkipToFadeOut();
		Widget->Close();
		Widget->AdvanceFade(100000.0);
		TestEqual(TEXT("Close cancels skipped completion"), Receiver->CompletedCount, 0);
		TestFalse(TEXT("Closed Skip returns false"), Widget->SkipToFadeOut());
		TestEqual(TEXT("Expected state calls never fail playback"), Receiver->FailedCount, 0);
	}
	return true;
}
#endif
