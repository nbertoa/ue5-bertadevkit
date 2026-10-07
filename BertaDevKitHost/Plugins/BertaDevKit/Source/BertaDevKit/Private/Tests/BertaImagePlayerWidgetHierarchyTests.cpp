#include "UI/BertaImagePlayerWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Engine/Texture2D.h"
#include "Layout/Children.h"
#include "Misc/AutomationTest.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/SWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaImagePlayerWidgetHierarchyTest,
	"BertaDevKit.UI.ImagePlayer.ConstructedHierarchy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaImagePlayerWidgetHierarchyTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UTexture2D> Texture(NewObject<UTexture2D>());
	for (int32 TemplateCase = 0; TemplateCase < 3; ++TemplateCase)
	{
		for (int32 AutoplayCase = 0; AutoplayCase < 2; ++AutoplayCase)
		{
			AddInfo(FString::Printf(TEXT("Template case %d, autoplay %d"), TemplateCase, AutoplayCase));
			TStrongObjectPtr<UBertaImagePlayerWidget> Widget(NewObject<UBertaImagePlayerWidget>());
			Widget->Texture = Texture.Get();
			Widget->Options.bAutoPlay = AutoplayCase != 0;
			Widget->Options.bRemoveOnCompletion = false;
			Widget->Options.FadeInDuration = 1.0f;
			Widget->Options.DisplayDuration = 3.0f;
			Widget->Options.FadeOutDuration = 1.0f;
			if (!TestTrue(TEXT("Initialize creates the actual widget tree"), Widget->Initialize()))
			{
				return false;
			}

			// Seed the same tree shapes supplied by a Blueprint subclass. Both images
			// deliberately hold the supplied texture at full opacity before RebuildWidget.
			UImage* TemplateImage = nullptr;
			if (TemplateCase == 1)
			{
				TemplateImage = Widget->WidgetTree->ConstructWidget<UImage>(
					UImage::StaticClass(), TEXT("BlueprintImage"));
				Widget->WidgetTree->RootWidget = TemplateImage;
			}
			else if (TemplateCase == 2)
			{
				UOverlay* TemplateRoot = Widget->WidgetTree->ConstructWidget<UOverlay>(
					UOverlay::StaticClass(), TEXT("BertaImageRoot"));
				TemplateImage = Widget->WidgetTree->ConstructWidget<UImage>(
					UImage::StaticClass(), TEXT("BertaImageVisual"));
				TemplateRoot->AddChildToOverlay(TemplateImage);
				Widget->WidgetTree->RootWidget = TemplateRoot;
			}
			if (TemplateImage)
			{
				TemplateImage->SetBrushFromTexture(Texture.Get());
				TemplateImage->SetRenderOpacity(1.0f);
			}

			// Build real UMG and Slate children, first BEFORE the player's Construct.
			// No ImageVisual injection, paint, viewport, world, or renderer is involved.
			TSharedRef<SWidget> NativeSlateRoot = Widget->RebuildWidget();
			if (!TestNotNull(TEXT("RebuildWidget constructs the native image"), Widget->ImageVisual.Get())
				|| !TestNotNull(TEXT("RebuildWidget constructs the native root"), Widget->ImageRoot.Get()))
			{
				return false;
			}
			TestTrue(TEXT("The rendered tree uses the exclusively owned root"), Widget->GetRootWidget() == Widget->ImageRoot);
			TestEqual(TEXT("The UMG root has exactly one visual"), Widget->ImageRoot->GetChildrenCount(), 1);
			TestTrue(TEXT("The sole UMG child is ImageVisual"), Widget->ImageRoot->GetChildAt(0) == Widget->ImageVisual);
			TestTrue(TEXT("Designer name collisions never reuse a template image"), Widget->ImageVisual.Get() != TemplateImage);

			int32 ReachableImages = 0;
			Widget->WidgetTree->ForEachWidget([&](UWidget* Child)
			{
				ReachableImages += Child->IsA<UImage>() ? 1 : 0;
			});
			TestEqual(TEXT("No independent texture-bearing image survives in the rendered tree"), ReachableImages, 1);
			TestEqual(TEXT("The Slate root also has exactly one child"), NativeSlateRoot->GetChildren()->Num(), 1);

			TSharedPtr<SWidget> CachedImage = Widget->ImageVisual->GetCachedWidget();
			if (!TestTrue(TEXT("The real Slate image was built"), CachedImage.IsValid()))
			{
				return false;
			}
			TSharedRef<SWidget> SlateImage = CachedImage.ToSharedRef();
			TestTrue(TEXT("The sole Slate child is the controlled image"), NativeSlateRoot->GetChildren()->GetChildAt(0) == SlateImage);
			TestEqual(TEXT("Pre-Construct UMG opacity is exactly zero"), Widget->ImageVisual->GetRenderOpacity(), 0.0f);
			TestEqual(TEXT("Pre-Construct Slate opacity is exactly zero"), SlateImage->GetRenderOpacity(), 0.0f);
			TestTrue(TEXT("No brush texture is exposed before playback"), Widget->ImageVisual->GetBrush().GetResourceObject() == nullptr);
			if (TemplateImage)
			{
				TestFalse(TEXT("The discarded template image never acquires a Slate visual"), TemplateImage->GetCachedWidget().IsValid());
			}

			// TakeWidget exercises synchronization, PreConstruct, and NativeConstruct,
			// including autoplay, rather than calling lifecycle methods manually.
			TSharedRef<SWidget> ConstructedSlateWidget = Widget->TakeWidget();
			TestEqual(TEXT("The user widget wrapper has one root"), ConstructedSlateWidget->GetChildren()->Num(), 1);
			TestTrue(TEXT("The user widget wrapper renders the owned Slate root"), ConstructedSlateWidget->GetChildren()->GetChildAt(0) == NativeSlateRoot);
			TestEqual(TEXT("The first constructed frame's image opacity is zero"), SlateImage->GetRenderOpacity(), 0.0f);
			if (!Widget->Options.bAutoPlay)
			{
				TestTrue(TEXT("Manual playback stays inactive after construction"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Inactive);
				TestTrue(TEXT("Manual playback still has no texture before Play"), Widget->ImageVisual->GetBrush().GetResourceObject() == nullptr);
				TestTrue(TEXT("Constructed manual playback starts"), Widget->Play());
			}
			TestTrue(TEXT("Construction/play waits for the first Slate update"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::WaitingForFadeInTick);
			TestTrue(TEXT("The controlled image brush now references the supplied texture"), Widget->ImageVisual->GetBrush().GetResourceObject() == Texture.Get());
			TestEqual(TEXT("Assigning the brush cannot expose the image before fade progression"), SlateImage->GetRenderOpacity(), 0.0f);

			// Simulate a first Slate tick arriving well after the entire fade duration.
			// It must anchor the clock at opacity zero, not skip straight to display.
			const double StartTime = Widget->StateStartTime + 10.0;
			Widget->AdvancePlayback(StartTime);
			TestEqual(TEXT("A delayed first update still renders zero image opacity"), SlateImage->GetRenderOpacity(), 0.0f);
			TestTrue(TEXT("The first update enters fade-in instead of skipping it"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::FadingIn);
			TestEqual(TEXT("The fade clock starts at the first Slate update"), Widget->StateStartTime, StartTime);
			Widget->AdvancePlayback(StartTime + 0.25);
			TestEqual(TEXT("The first appearance comes from the controlled Slate opacity"), SlateImage->GetRenderOpacity(), 0.25f);
			Widget->AdvancePlayback(StartTime + 1.0);
			TestEqual(TEXT("The constructed image becomes fully visible"), SlateImage->GetRenderOpacity(), 1.0f);
			Widget->AdvancePlayback(StartTime + 3.999);
			TestEqual(TEXT("Display retains its full interval after fade-in"), SlateImage->GetRenderOpacity(), 1.0f);
			Widget->AdvancePlayback(StartTime + 4.0);
			Widget->AdvancePlayback(StartTime + 4.5);
			TestEqual(TEXT("Fade-out still drives the same Slate image linearly"), SlateImage->GetRenderOpacity(), 0.5f);
			Widget->AdvancePlayback(StartTime + 5.0);
			TestEqual(TEXT("Fade-out still ends fully transparent"), SlateImage->GetRenderOpacity(), 0.0f);
			TestTrue(TEXT("The constructed sequence still completes"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Completed);
			Widget->Close();
		}
	}

	{
		TStrongObjectPtr<UBertaImagePlayerWidget> Widget(NewObject<UBertaImagePlayerWidget>());
		Widget->Texture = Texture.Get();
		Widget->Options.bAutoPlay = false;
		TSharedPtr<SWidget> SlateWidget = Widget->TakeWidget();
		UOverlay* OriginalRoot = Widget->ImageRoot;
		UImage* OriginalImage = Widget->ImageVisual;
		SlateWidget.Reset(); // Routes real NativeDestruct and releases child Slate resources.
		TestTrue(TEXT("Slate destruction cancels the playback reference"), Widget->ImageVisual == nullptr);

		SlateWidget = Widget->TakeWidget();
		TestTrue(TEXT("Reconstruction reuses the owned root"), Widget->ImageRoot == OriginalRoot);
		TestTrue(TEXT("Reconstruction reuses the owned image"), Widget->ImageVisual == OriginalImage);
		TestEqual(TEXT("Reconstruction does not accumulate visuals"), Widget->ImageRoot->GetChildrenCount(), 1);
		TestEqual(TEXT("Reconstructed Slate image is transparent"), Widget->ImageVisual->GetCachedWidget()->GetRenderOpacity(), 0.0f);
		TestTrue(TEXT("Reconstructed playback can start"), Widget->Play());
		TestEqual(TEXT("Reconstructed texture assignment stays transparent"), Widget->ImageVisual->GetCachedWidget()->GetRenderOpacity(), 0.0f);
		Widget->Close();
	}

	{
		TStrongObjectPtr<UBertaImagePlayerWidget> Widget(NewObject<UBertaImagePlayerWidget>());
		Widget->Texture = Texture.Get();
		Widget->Options.bUseFadeIn = false;
		Widget->Options.bRemoveOnCompletion = false;
		TSharedRef<SWidget> SlateWidget = Widget->TakeWidget();
		TestEqual(TEXT("Disabling fade-in still displays immediately"), Widget->ImageVisual->GetCachedWidget()->GetRenderOpacity(), 1.0f);
		TestTrue(TEXT("Disabling fade-in still enters display"), Widget->PlaybackState == UBertaImagePlayerWidget::EPlaybackState::Displaying);
		Widget->Close();
	}

	return true;
}

#endif
