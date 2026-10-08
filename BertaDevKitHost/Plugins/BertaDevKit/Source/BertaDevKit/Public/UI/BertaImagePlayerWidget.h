#pragma once

#include "Blueprint/UserWidget.h"

#include "BertaImagePlayerWidget.generated.h"

class UImage;
class UOverlay;
class UTexture2D;
class UBertaImagePlayerWidget;

USTRUCT(BlueprintType)
struct BERTADEVKIT_API FBertaImagePlaybackOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image")
	bool bAutoPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image")
	bool bRemoveOnCompletion = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image|Transitions")
	bool bUseFadeIn = true;

	/** Real seconds from transparent to fully visible. Zero is immediate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image|Transitions", meta = (ClampMin = "0.0", EditCondition = "bUseFadeIn"))
	float FadeInDuration = 0.5f;

	/** Real seconds at RenderOpacity 1.0, excluding both fades. Zero is immediate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image", meta = (ClampMin = "0.0"))
	float DisplayDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image|Transitions")
	bool bUseFadeOut = true;

	/** Real seconds from fully visible to transparent. Zero is immediate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image|Transitions", meta = (ClampMin = "0.0", EditCondition = "bUseFadeOut"))
	float FadeOutDuration = 0.5f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBertaImageDisplayCompletedEvent,
	UBertaImagePlayerWidget*, Widget);

/** Fullscreen Runtime image display. Owns its visual tree; Blueprint designer content is not rendered. */
UCLASS(Blueprintable)
class BERTADEVKIT_API UBertaImagePlayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set Texture restarts using current Options; null clears without closing. C++ callers should use SetTexture. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetTexture, Category = "Image", meta = (ExposeOnSpawn = true))
	TObjectPtr<UTexture2D> Texture;

	/** Options are captured by Play; edits do not change an active sequence. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image", meta = (ExposeOnSpawn = true))
	FBertaImagePlaybackOptions Options;

	/** Broadcast once after the sequence completes, before optional removal. Never emitted by Close. */
	UPROPERTY(BlueprintAssignable, Category = "BertaDevKit|UI|Image")
	FBertaImageDisplayCompletedEvent OnDisplayCompleted;

	/** Same-source assignment restarts; before construction this only configures the source. Close stays terminal. */
	UFUNCTION(BlueprintSetter, Category = "BertaDevKit|UI|Image")
	void SetTexture(UTexture2D* NewTexture);

	/**
	 * Starts once the native visual has been constructed (including construction as an embedded child).
	 * A positive fade-in anchors its clock on the first Slate update at zero opacity.
	 * Returns true while already playing, false after completion or Close. Does not restart.
	 * Invalid texture or non-finite active durations log a warning and close the widget.
	 */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|UI|Image")
	bool Play();

	/** Terminal, idempotent cancellation and removal. Does not broadcast natural completion. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|UI|Image")
	void Close();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	friend class FBertaImagePlayerWidgetPlaybackTest;
	friend class FBertaImagePlayerWidgetHierarchyTest;
	friend class FBertaImagePlayerWidgetReplacementTest;

	enum class EPlaybackState : uint8
	{
		Inactive,
		WaitingForFadeInTick,
		FadingIn,
		Displaying,
		FadingOut,
		Completed,
		Closed
	};

	void ResetPlayback();
	void AdvancePlayback(double Now);
	void CompletePlayback();
	void ClearVisual();
	void RemoveWhenReady();

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> ImageRoot;

	UPROPERTY(Transient)
	TObjectPtr<UImage> ImageVisual;

	FBertaImagePlaybackOptions ActiveOptions;
	EPlaybackState PlaybackState = EPlaybackState::Inactive;
	double StateStartTime = 0.0;
	uint64 PlaybackGeneration = 0;
	bool bNativeConstructed = false;
	bool bSourceCleared = false;
	bool bIsConstructing = false;
};
