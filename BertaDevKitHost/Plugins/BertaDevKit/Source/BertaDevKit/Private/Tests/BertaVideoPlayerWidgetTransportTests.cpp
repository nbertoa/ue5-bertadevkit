#include "UI/BertaVideoPlayerWidget.h"
#include "Tests/BertaVideoPlayerWidgetTestReceiver.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Components/AudioComponent.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "FileMediaSource.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "Misc/AutomationTest.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaVideoPlayerWidgetTransportTest,
	"BertaDevKit.UI.VideoPlayer.TransportContracts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaVideoPlayerWidgetTransportTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UBertaVideoPlayerWidget> Widget(NewObject<UBertaVideoPlayerWidget>());
	TStrongObjectPtr<UFileMediaSource> Source(NewObject<UFileMediaSource>());
	TStrongObjectPtr<UOverlay> Parent(NewObject<UOverlay>());
	TStrongObjectPtr<UBertaVideoPlayerWidgetTestReceiver> Receiver(NewObject<UBertaVideoPlayerWidgetTestReceiver>());
	Parent->AddChildToOverlay(Widget.Get());
	Widget->SetMediaSource(Source.Get());
	Widget->Options.bAutoPlay = false;
	Widget->Options.bPauseGame = false;
	Widget->Options.bPlayAudio = false;
	Widget->VideoImage = NewObject<UImage>(Widget.Get());
	Widget->BlackVisual = NewObject<UBorder>(Widget.Get());
	Widget->ActiveOptions = Widget->Options;
	Widget->OnPlaybackStarted.AddDynamic(Receiver.Get(), &UBertaVideoPlayerWidgetTestReceiver::Started);
	Widget->OnPlaybackCompleted.AddDynamic(Receiver.Get(), &UBertaVideoPlayerWidgetTestReceiver::Completed);
	Widget->OnPlaybackFailed.AddDynamic(Receiver.Get(), &UBertaVideoPlayerWidgetTestReceiver::Failed);
	FString Error;
	TestTrue(TEXT("Create resources without opening a backend"), Widget->CreateMediaResources(Error));
	TStrongObjectPtr<UMediaPlayer> Player(Widget->InternalMediaPlayer);
	TStrongObjectPtr<UMediaTexture> Texture(Widget->InternalMediaTexture);
	Widget->InternalExternalAudio = NewObject<UAudioComponent>(Widget.Get());
	TStrongObjectPtr<UAudioComponent> Audio(Widget->InternalExternalAudio);
	Widget->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::Playing;
	Widget->bOwnsPause = true;
	const uint64 Run = Widget->PlaybackGeneration;

	// The unopened native player rejects transport, without failing or changing widget state.
	TestFalse(TEXT("A backend-rejected Pause is harmless"), Widget->Pause());
	TestFalse(TEXT("Resume without transport Pause is harmless"), Widget->Resume());
	TestFalse(TEXT("Rejected Pause does not mark transport paused"), Widget->bTransportPaused);

	// Exercise exactly the bookkeeping used after native Pause/Play accept the request.
	// Backend acceptance and actual video position preservation remain manual checks.
	Widget->ApplyTransportPause(true, 100.0);
	TestTrue(TEXT("Accepted Pause marks transport paused and preserves playing phase"), Widget->bTransportPaused && Widget->PlaybackState == UBertaVideoPlayerWidget::EPlaybackState::Playing);
	TestTrue(TEXT("Pause uses SetPaused on the same audio component"), Audio->bIsPaused && Widget->InternalExternalAudio == Audio.Get());
	TestTrue(TEXT("Pause preserves media resources"), Widget->InternalMediaPlayer == Player.Get() && Widget->InternalMediaTexture == Texture.Get());
	TestTrue(TEXT("Pause preserves gameplay pause ownership"), Widget->bOwnsPause);
	TestEqual(TEXT("Pause does not invalidate the run"), Widget->PlaybackGeneration, Run);
	TestFalse(TEXT("Repeated Pause returns false without mutation"), Widget->Pause());
	TestFalse(TEXT("Play while paused takes the backend Resume path, not the playing no-op"), Widget->Play());
	TestTrue(TEXT("Rejected Resume keeps the current Pause"), Widget->bTransportPaused);
	Widget->HandlePlaybackResumed();
	TestEqual(TEXT("Resume callback never repeats Started"), Receiver->StartedCount, 0);
	Widget->ApplyTransportPause(false, 110.0);
	Widget->HandlePlaybackResumed();
	TestEqual(TEXT("Accepted Resume also never repeats Started"), Receiver->StartedCount, 0);
	Widget->ApplyTransportPause(false, 120.0);
	TestFalse(TEXT("Accepted Resume unpauses the same audio"), Audio->bIsPaused || Widget->bTransportPaused);
	TestTrue(TEXT("Resume still retains gameplay pause ownership"), Widget->bOwnsPause);
	TestFalse(TEXT("Repeated Resume is a harmless no-op"), Widget->Resume());

	const UBertaVideoPlayerWidget::EPlaybackState Fades[] = {
		UBertaVideoPlayerWidget::EPlaybackState::FadingInVideo,
		UBertaVideoPlayerWidget::EPlaybackState::FadingOutVideoAtEnd
	};
	for (auto State : Fades)
	{
		Widget->PlaybackState = State;
		Widget->ActiveFadeDuration = 20.0f;
		Widget->FadeStartTime = 100.0;
		Widget->FadeStartOpacity = State == UBertaVideoPlayerWidget::EPlaybackState::FadingInVideo ? 1.0f : 0.0f;
		Widget->FadeEndOpacity = 1.0f - Widget->FadeStartOpacity;
		Widget->AdvanceFade(105.0);
		const float FrozenOpacity = Widget->BlackVisual->GetRenderOpacity();
		Widget->ApplyTransportPause(true, 105.0);
		Widget->ApplyTransportPause(true, 106.0);
		Widget->AdvanceFade(1000.0);
		Widget->TryStartEndFade();
		TestEqual(TEXT("Paused fade cannot progress across a long real-time interval"), Widget->BlackVisual->GetRenderOpacity(), FrozenOpacity);
		TestTrue(TEXT("Paused end-fade detection cannot advance its phase"), Widget->PlaybackState == State);
		Widget->ApplyTransportPause(false, 115.0);
		TestEqual(TEXT("Resume shifts the clock exactly once by paused duration"), Widget->FadeStartTime, 110.0);
		Widget->AdvanceFade(115.0);
		TestEqual(TEXT("Resume retains previous visible progress"), Widget->BlackVisual->GetRenderOpacity(), FrozenOpacity);
		Widget->AdvanceFade(120.0);
		TestEqual(TEXT("Fade continues after Resume"), Widget->BlackVisual->GetRenderOpacity(), 0.5f);
	}
	TestEqual(TEXT("Transport emits no completion"), Receiver->CompletedCount, 0);
	TestEqual(TEXT("Transport misuse emits no failure"), Receiver->FailedCount, 0);

	Widget->ApplyTransportPause(true, 130.0);
	Widget->bLoopSeekPending = true;
	Widget->bRestartingLoop = true;
	Widget->bPlayRequested = true;
	Widget->Stop();
	TestTrue(TEXT("Pause then Stop is nonterminal/stopped"), !Widget->bTerminal && Widget->PlaybackState == UBertaVideoPlayerWidget::EPlaybackState::Stopped);
	TestFalse(TEXT("Stop clears transport/loop ownership and releases gameplay pause"), Widget->bTransportPaused || Widget->bEndReachedWhilePaused || Widget->bLoopSeekPending || Widget->bRestartingLoop || Widget->bPlayRequested || Widget->bOwnsPause);
	TestEqual(TEXT("Stop clears pause clock"), Widget->TransportPauseStartTime, 0.0);
	TestEqual(TEXT("Stop cancels transition"), Widget->ActiveFadeDuration, 0.0f);
	TestNull(TEXT("Stop releases player"), Widget->InternalMediaPlayer.Get());
	TestNull(TEXT("Stop releases texture"), Widget->InternalMediaTexture.Get());
	TestNull(TEXT("Stop releases audio rather than retaining a paused component"), Widget->InternalExternalAudio.Get());
	TestNull(TEXT("Old texture is detached"), Texture->GetMediaPlayer());
	TestNull(TEXT("Old audio sound is cleared"), Audio->Sound.Get());
	TestFalse(TEXT("All old media delegates are removed"), Player->OnMediaOpened.IsBound() || Player->OnMediaOpenFailed.IsBound() || Player->OnPlaybackResumed.IsBound() || Player->OnEndReached.IsBound() || Player->OnSeekCompleted.IsBound());
	TestTrue(TEXT("Stop preserves parent/source/configuration/bindings"), Widget->GetParent() == Parent.Get() && Widget->MediaSource == Source.Get() && !Widget->Options.bAutoPlay && Widget->OnPlaybackStarted.IsBound());
	Widget->Stop();
	Widget->HandleMediaOpened(TEXT("stale"));
	Widget->HandleMediaOpenFailed(TEXT("stale"));
	Widget->HandlePlaybackResumed();
	Widget->HandleSeekCompleted();
	Widget->HandleEndReached();
	TestTrue(TEXT("Stop twice and late callbacks stay stopped"), Widget->PlaybackState == UBertaVideoPlayerWidget::EPlaybackState::Stopped);
	Widget->NativeDestruct();
	TestTrue(TEXT("Destruction of a stopped visual cannot re-enable autoplay"), Widget->PlaybackState == UBertaVideoPlayerWidget::EPlaybackState::Stopped);
	TestFalse(TEXT("Pause after Stop is harmless"), Widget->Pause());
	TestFalse(TEXT("Resume after Stop is harmless"), Widget->Resume());
	TestEqual(TEXT("Stop never completes"), Receiver->CompletedCount, 0);
	TestEqual(TEXT("Stop/late callbacks never fail"), Receiver->FailedCount, 0);

	// Explicit Play reaches fresh activation even with autoplay false. There is no World/backend here.
	Widget->bNativeConstructed = true;
	const uint64 StoppedRun = Widget->PlaybackGeneration;
	AddExpectedMessage(TEXT("no valid World context"), EAutomationExpectedMessageFlags::Contains, 1);
	TestFalse(TEXT("A new activation needs a World"), Widget->Play());
	TestTrue(TEXT("Play after Stop starts a new generation"), Widget->PlaybackGeneration != StoppedRun);
	TestFalse(TEXT("Fresh activation retains configured autoplay false"), Widget->ActiveOptions.bAutoPlay);
	TestEqual(TEXT("Only the real activation error reports failure"), Receiver->FailedCount, 1);
	Parent->AddChildToOverlay(Widget.Get());
	Widget->SetMediaSource(Source.Get());
	TestFalse(TEXT("Stop then source replacement clears terminal failure"), Widget->bTerminal);

	// Reentrant Stop cannot be followed by the interrupted Started handler's reveal fade.
	Widget->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::StartingBehindBlack;
	Widget->TransitionHalfDuration = 2.0f;
	Receiver->bStopOnStarted = true;
	Widget->HandlePlaybackResumed();
	TestTrue(TEXT("Started listener Stop remains stopped"), Widget->PlaybackState == UBertaVideoPlayerWidget::EPlaybackState::Stopped);
	TestEqual(TEXT("Started Stop cancels the pre-established reveal"), Widget->ActiveFadeDuration, 0.0f);
	Receiver->bStopOnStarted = false;

	Widget->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::Playing;
	Widget->ApplyTransportPause(true, 150.0);
	Widget->HandleEndReached();
	Widget->SetMediaSource(Source.Get());
	TestFalse(TEXT("Pause then source replacement discards old transport state"), Widget->bTransportPaused || Widget->bEndReachedWhilePaused);
	TestEqual(TEXT("Replacement clears the paused clock"), Widget->TransportPauseStartTime, 0.0);
	TestTrue(TEXT("Replacement preserves the same child"), Widget->GetParent() == Parent.Get());
	Widget->Stop();
	Widget->SetMediaSource(nullptr);
	TestTrue(TEXT("Stop then intentional clear follows source replacement semantics"), Widget->PlaybackState == UBertaVideoPlayerWidget::EPlaybackState::Inactive && !Widget->bTerminal);

	TStrongObjectPtr<UBertaVideoPlayerWidget> AtEnd(NewObject<UBertaVideoPlayerWidget>());
	AtEnd->InternalMediaPlayer = NewObject<UMediaPlayer>(AtEnd.Get());
	AtEnd->ActiveOptions.bRemoveOnCompletion = false;
	AtEnd->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::Playing;
	AtEnd->OnPlaybackCompleted.AddDynamic(Receiver.Get(), &UBertaVideoPlayerWidgetTestReceiver::Completed);
	const int32 PreviousCompletions = Receiver->CompletedCount;
	AtEnd->ApplyTransportPause(true, 200.0);
	AtEnd->HandleEndReached();
	TestTrue(TEXT("An end notification arriving while paused is retained"), AtEnd->bEndReachedWhilePaused);
	TestEqual(TEXT("Queued natural end cannot complete while paused"), Receiver->CompletedCount, PreviousCompletions);
	AtEnd->Stop();
	TestFalse(TEXT("Stop cancels a still-pending natural end"), AtEnd->bEndReachedWhilePaused);
	TestEqual(TEXT("Stop cannot broadcast the pending completion"), Receiver->CompletedCount, PreviousCompletions);
	AtEnd->InternalMediaPlayer = NewObject<UMediaPlayer>(AtEnd.Get());
	AtEnd->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::Playing;
	AtEnd->ActiveOptions.bRemoveOnCompletion = false;
	AtEnd->ApplyTransportPause(true, 210.0);
	AtEnd->HandleEndReached();
	TestTrue(TEXT("Resume processes a retained natural boundary"), AtEnd->Resume());
	TestEqual(TEXT("Retained natural end completes exactly once on Resume"), Receiver->CompletedCount, PreviousCompletions + 1);
	AtEnd->HandleEndReached();
	TestEqual(TEXT("Duplicate end callback is harmless"), Receiver->CompletedCount, PreviousCompletions + 1);
	AtEnd->Stop();
	TestFalse(TEXT("Stop discards deferred natural end state"), AtEnd->bEndReachedWhilePaused);

	Widget->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::Playing;
	Widget->ApplyTransportPause(true, 160.0);
	Widget->Close();
	Widget->Stop();
	Widget->SetMediaSource(Source.Get());
	TestFalse(TEXT("Close discards transport Pause"), Widget->bTransportPaused);
	TestFalse(TEXT("Close then Play cannot revive"), Widget->Play());
	TestTrue(TEXT("Close remains terminal even after Stop/source assignment"), Widget->bTerminal && Widget->bClosedExplicitly);
	TestNull(TEXT("Only Close removes the widget"), Widget->GetParent());
	return true;
}
#endif
