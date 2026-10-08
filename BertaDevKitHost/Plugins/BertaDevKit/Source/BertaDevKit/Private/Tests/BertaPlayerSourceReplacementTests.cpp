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
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaTexture.h"
#include "Misc/AutomationTest.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Widgets/SWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaImagePlayerWidgetReplacementTest,
	"BertaDevKit.UI.ImagePlayer.SourceReplacement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaImagePlayerWidgetReplacementTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UTexture2D> A(NewObject<UTexture2D>());
	TStrongObjectPtr<UTexture2D> B(NewObject<UTexture2D>());
	TStrongObjectPtr<UBertaImagePlayerWidgetTestReceiver> Receiver(NewObject<UBertaImagePlayerWidgetTestReceiver>());
	TStrongObjectPtr<UBertaImagePlayerWidget> Widget(NewObject<UBertaImagePlayerWidget>());
	TStrongObjectPtr<UOverlay> Parent(NewObject<UOverlay>());
	Parent->AddChildToOverlay(Widget.Get());
	Widget->Options.bRemoveOnCompletion = false;
	Widget->SetTexture(A.Get());
	TestNull(TEXT("Preconstruction setter does not create a visual"), Widget->ImageVisual.Get());
	TSharedRef<SWidget> Slate = Widget->TakeWidget();
	UImage* OriginalVisual = Widget->ImageVisual;
	Widget->OnDisplayCompleted.AddDynamic(Receiver.Get(), &UBertaImagePlayerWidgetTestReceiver::RecordCompletion);

	const UBertaImagePlayerWidget::EPlaybackState States[] = {
		UBertaImagePlayerWidget::EPlaybackState::WaitingForFadeInTick,
		UBertaImagePlayerWidget::EPlaybackState::FadingIn,
		UBertaImagePlayerWidget::EPlaybackState::Displaying,
		UBertaImagePlayerWidget::EPlaybackState::FadingOut,
		UBertaImagePlayerWidget::EPlaybackState::Completed
	};
	for (auto State : States)
	{
		Widget->PlaybackState = State;
		Widget->ImageVisual->SetRenderOpacity(1.0f);
		Widget->Options.FadeInDuration = 2.0f;
		Widget->Options.DisplayDuration = 7.0f;
		const uint64 PreviousRun = Widget->PlaybackGeneration;
		Widget->SetTexture(B.Get());
		TestTrue(TEXT("Every stage restarts through the transparent first-tick state"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::WaitingForFadeInTick);
		TestTrue(TEXT("A restart invalidates the previous execution"), Widget->PlaybackGeneration != PreviousRun);
		TestEqual(TEXT("Replacement captures current Options"), Widget->ActiveOptions.DisplayDuration, 7.0f);
		TestEqual(TEXT("The new brush is invisible in actual Slate"), OriginalVisual->GetCachedWidget()->GetRenderOpacity(), 0.0f);
		TestTrue(TEXT("Replacement keeps the native hierarchy and parent"), Widget->ImageVisual == OriginalVisual && Widget->GetParent() == Parent.Get());
		TestTrue(TEXT("New source is on the controlled image"), OriginalVisual->GetBrush().GetResourceObject() == B.Get());
		Widget->AdvancePlayback(1000.0);
		TestEqual(TEXT("Late first tick still remains transparent"), OriginalVisual->GetCachedWidget()->GetRenderOpacity(), 0.0f);
		Widget->AdvancePlayback(1001.0);
		TestEqual(TEXT("Restart fade is linear from its own beginning"), OriginalVisual->GetRenderOpacity(), 0.5f);
	}
	TestEqual(TEXT("Interrupted runs never complete"), Receiver->CompletionCount, 0);
	Widget->SetTexture(nullptr);
	TestTrue(TEXT("Null clears and leaves inactive reusable state"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Inactive);
	TestNull(TEXT("Null clears the brush"), OriginalVisual->GetBrush().GetResourceObject());
	Widget->Options.bAutoPlay = false;
	Widget->SetTexture(A.Get());
	TestTrue(TEXT("Autoplay false waits for explicit Play"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Inactive);
	TestTrue(TEXT("Valid source after null can play"), Widget->Play());

	// Both old and replacement runs complete synchronously; state checks alone cannot distinguish them.
	Widget->Options.bAutoPlay = true;
	Widget->Options.FadeInDuration = 0;
	Widget->Options.DisplayDuration = 0;
	Widget->Options.FadeOutDuration = 0;
	Widget->Options.bRemoveOnCompletion = true;
	Receiver->ReplacementTexture = B.Get();
	Receiver->bReplaceOnCompletion = true;
	Widget->SetTexture(A.Get());
	TestEqual(TEXT("Each completed source broadcasts exactly once"), Receiver->CompletionCount, 2);
	TestTrue(TEXT("Old completion cannot remove an immediately completed replacement"), Widget->GetParent() == Parent.Get());
	TestTrue(TEXT("The completion listener's replacement persists"), Widget->Texture == B.Get());
	Widget->Close();
	Widget->SetTexture(A.Get());
	TestFalse(TEXT("Setter cannot revive explicit Close"), Widget->Play());
	TestNull(TEXT("Close removes the widget"), Widget->GetParent());

	TStrongObjectPtr<UBertaImagePlayerWidget> Removed(NewObject<UBertaImagePlayerWidget>());
	Parent->AddChildToOverlay(Removed.Get());
	Removed->Options.bAutoPlay = false;
	Removed->Options.bUseFadeIn = false;
	Removed->Options.bUseFadeOut = false;
	Removed->Options.DisplayDuration = 0;
	Removed->SetTexture(A.Get());
	TSharedRef<SWidget> RemovedSlate = Removed->TakeWidget();
	Removed->Play();
	Removed->SetTexture(B.Get());
	TestNull(TEXT("A setter never reattaches a naturally removed widget"), Removed->GetParent());
	TestTrue(TEXT("Removed widgets configure the next construction without playback"), Removed->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Inactive);
	TStrongObjectPtr<UBertaImagePlayerWidget> Constructed(NewObject<UBertaImagePlayerWidget>());
	Parent->AddChildToOverlay(Constructed.Get());
	Constructed->Options.FadeInDuration = 0;
	Constructed->Options.DisplayDuration = 0;
	Constructed->Options.FadeOutDuration = 0;
	Constructed->SetTexture(A.Get());
	Receiver->CompletionCount = 0;
	Receiver->bReplaceOnCompletion = true;
	Constructed->OnDisplayCompleted.AddDynamic(Receiver.Get(), &UBertaImagePlayerWidgetTestReceiver::RecordCompletion);
	TSharedRef<SWidget> ConstructedSlate = Constructed->TakeWidget();
	TestEqual(TEXT("Replacement from initial autoplay completion also runs immediately"), Receiver->CompletionCount, 2);
	TestTrue(TEXT("Construct-time replacement prevents old pending removal"), Constructed->GetParent() == Parent.Get());
	Constructed->Close();

	TStrongObjectPtr<UBertaImagePlayerWidget> Empty(NewObject<UBertaImagePlayerWidget>());
	Empty->SetTexture(nullptr);
	TSharedRef<SWidget> EmptySlate = Empty->TakeWidget();
	TestTrue(TEXT("An intentional clear before construction remains inactive"), Empty->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Inactive);
	Empty->SetTexture(A.Get());
	TestTrue(TEXT("A constructed cleared widget can restart later"), Empty->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::WaitingForFadeInTick);
	Empty->Close();
#if WITH_EDITOR
	const FProperty* TextureProperty = UBertaImagePlayerWidget::StaticClass()->FindPropertyByName(TEXT("Texture"));
	TestEqual(TEXT("Blueprint assignment is wired to SetTexture"), TextureProperty->GetMetaData(TEXT("BlueprintSetter")), FString(TEXT("SetTexture")));
	TestTrue(TEXT("Texture remains exposed on spawn"), TextureProperty->GetBoolMetaData(TEXT("ExposeOnSpawn")));
#endif
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaVideoPlayerWidgetReplacementTest,
	"BertaDevKit.UI.VideoPlayer.SourceReplacement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaVideoPlayerWidgetReplacementTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UFileMediaSource> Source(NewObject<UFileMediaSource>());
	TStrongObjectPtr<UBertaVideoPlayerWidget> Widget(NewObject<UBertaVideoPlayerWidget>());
	TStrongObjectPtr<UOverlay> Parent(NewObject<UOverlay>());
	TStrongObjectPtr<UBertaVideoPlayerWidgetTestReceiver> Receiver(NewObject<UBertaVideoPlayerWidgetTestReceiver>());
	Parent->AddChildToOverlay(Widget.Get());
	Widget->SetMediaSource(Source.Get());
	TestNull(TEXT("Preconstruction setter never opens media"), Widget->InternalMediaPlayer.Get());
	Widget->VideoImage = NewObject<UImage>(Widget.Get());
	Widget->BlackVisual = NewObject<UBorder>(Widget.Get());
	Widget->OnPlaybackStarted.AddDynamic(Receiver.Get(), &UBertaVideoPlayerWidgetTestReceiver::Started);
	Widget->OnPlaybackCompleted.AddDynamic(Receiver.Get(), &UBertaVideoPlayerWidgetTestReceiver::Completed);
	Widget->OnPlaybackFailed.AddDynamic(Receiver.Get(), &UBertaVideoPlayerWidgetTestReceiver::Failed);

	// Construct real media resources but never open a backend or play audible output.
	Widget->ActiveOptions.bPlayAudio = false;
	FString Error;
	TestTrue(TEXT("Native resource creation succeeds without opening a source"), Widget->CreateMediaResources(Error));
	TStrongObjectPtr<UMediaPlayer> OldPlayer(Widget->InternalMediaPlayer);
	TStrongObjectPtr<UMediaTexture> OldTexture(Widget->InternalMediaTexture);
	Widget->InternalExternalAudio = NewObject<UAudioComponent>(Widget.Get());
	Widget->InternalMediaSound = NewObject<UMediaSoundComponent>(Widget.Get());
	Widget->bTerminal = true;
	Widget->bOwnsPause = true;
	Widget->bRestartingLoop = true;
	Widget->bLoopSeekPending = true;
	Widget->ActiveFadeDuration = 3.0f;
	Widget->BlackVisual->SetRenderOpacity(1.0f);
	// Replacement reset is exercised without invoking a media backend, between native constructions.
	Widget->bNativeConstructed = false;
	Widget->SetMediaSource(Source.Get());
	TestFalse(TEXT("Source replacement resets natural terminal state"), Widget->bTerminal);
	TestFalse(TEXT("Old pause ownership is released/reset"), Widget->bOwnsPause || Widget->bPauseReleaseRequested);
	TestFalse(TEXT("Loop ownership is reset"), Widget->bRestartingLoop || Widget->bLoopSeekPending || Widget->bPlayRequested);
	TestNull(TEXT("Old player is released"), Widget->InternalMediaPlayer.Get());
	TestNull(TEXT("Old texture is released"), Widget->InternalMediaTexture.Get());
	TestNull(TEXT("Old external audio is released"), Widget->InternalExternalAudio.Get());
	TestNull(TEXT("Old media audio is released"), Widget->InternalMediaSound.Get());
	TestNull(TEXT("Texture no longer refers to the old player"), OldTexture->GetMediaPlayer());
	TestFalse(TEXT("Old media delegates are unbound"), OldPlayer->OnMediaOpened.IsBound() || OldPlayer->OnMediaOpenFailed.IsBound() || OldPlayer->OnPlaybackResumed.IsBound() || OldPlayer->OnEndReached.IsBound() || OldPlayer->OnSeekCompleted.IsBound());
	TestEqual(TEXT("Black transition resets"), Widget->BlackVisual->GetRenderOpacity(), 0.0f);
	TestTrue(TEXT("Replacement retains the same attached widget"), Widget->GetParent() == Parent.Get());
	TestEqual(TEXT("Interruption does not complete"), Receiver->CompletedCount, 0);
	const uint64 Run = Widget->PlaybackGeneration;
	Widget->SetMediaSource(Source.Get());
	TestTrue(TEXT("Same-source assignment resets again"), Widget->PlaybackGeneration != Run);
	Widget->SetMediaSource(nullptr);
	TestFalse(TEXT("Intentional null is reusable and nonterminal"), Widget->bTerminal);
	TestEqual(TEXT("Intentional null emits no failure"), Receiver->FailedCount, 0);
	Widget->SetMediaSource(Source.Get());

	// Validate option capture before activation requires World/backend access.
	Widget->Options.bAutoPlay = false;
	Widget->Options.bLoop = true;
	AddExpectedMessage(TEXT("no valid World context"), EAutomationExpectedMessageFlags::Contains, 2);
	TestFalse(TEXT("Activation without a World cannot open a backend"), Widget->ActivatePlayback());
	TestFalse(TEXT("A new activation captures current autoplay false"), Widget->ActiveOptions.bAutoPlay);
	TestTrue(TEXT("A new activation captures current loop options"), Widget->ActiveOptions.bLoop);
	Parent->AddChildToOverlay(Widget.Get());
	Widget->SetMediaSource(Source.Get());
	Widget->bNativeConstructed = true;
	Receiver->bClearOnEvent = true;
	TestFalse(TEXT("No-World activation fails explicitly"), Widget->ActivatePlayback());
	TestFalse(TEXT("A failure listener may clear the source without being removed"), Widget->bTerminal);
	TestTrue(TEXT("Failure listener keeps its child attached"), Widget->GetParent() == Parent.Get());
	Receiver->bClearOnEvent = false;

	Widget->bNativeConstructed = true;
	Widget->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::Playing;
	Widget->ActiveOptions.bRemoveOnCompletion = true;
	Receiver->bClearOnEvent = true;
	Widget->HandleEndReached();
	TestEqual(TEXT("Natural completion broadcasts once"), Receiver->CompletedCount, 1);
	TestTrue(TEXT("Completion replacement prevents old removal"), Widget->GetParent() == Parent.Get());
	TestFalse(TEXT("Completion replacement is inactive/reusable"), Widget->bTerminal);

	Widget->PlaybackState = UBertaVideoPlayerWidget::EPlaybackState::StartingBehindBlack;
	Widget->HandlePlaybackResumed();
	TestEqual(TEXT("Started listener ran"), Receiver->StartedCount, 1);
	TestTrue(TEXT("Started replacement prevents the old reveal fade"), Widget->PlaybackState == UBertaVideoPlayerWidget::EPlaybackState::Inactive);
	Widget->Close();
	Widget->SetMediaSource(Source.Get());
	TestTrue(TEXT("Close remains explicitly terminal"), Widget->bTerminal && Widget->bClosedExplicitly);
	TestFalse(TEXT("Source assignment cannot undo Close"), Widget->Play());
	TestNull(TEXT("Close removes the parent attachment"), Widget->GetParent());
#if WITH_EDITOR
	const FProperty* SourceProperty = UBertaVideoPlayerWidget::StaticClass()->FindPropertyByName(TEXT("MediaSource"));
	TestEqual(TEXT("Blueprint assignment is wired to SetMediaSource"), SourceProperty->GetMetaData(TEXT("BlueprintSetter")), FString(TEXT("SetMediaSource")));
	TestTrue(TEXT("MediaSource remains exposed on spawn"), SourceProperty->GetBoolMetaData(TEXT("ExposeOnSpawn")));
#endif
	return true;
}
#endif

void UBertaVideoPlayerWidgetTestReceiver::Started(UBertaVideoPlayerWidget* Widget)
{
#if WITH_DEV_AUTOMATION_TESTS
	++StartedCount;
	if (bClearOnEvent) Widget->SetMediaSource(nullptr);
#endif
}
void UBertaVideoPlayerWidgetTestReceiver::Completed(UBertaVideoPlayerWidget* Widget)
{
#if WITH_DEV_AUTOMATION_TESTS
	++CompletedCount;
	if (bClearOnEvent) Widget->SetMediaSource(nullptr);
#endif
}
void UBertaVideoPlayerWidgetTestReceiver::Failed(UBertaVideoPlayerWidget* Widget, FString Error)
{
#if WITH_DEV_AUTOMATION_TESTS
	++FailedCount;
	if (bClearOnEvent) Widget->SetMediaSource(nullptr);
#endif
}
