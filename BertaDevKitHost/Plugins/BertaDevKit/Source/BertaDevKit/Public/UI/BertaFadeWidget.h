#pragma once

#include "Blueprint/UserWidget.h"

#include "BertaFadeWidget.generated.h"

class UBorder;
class UBertaFadeWidget;

UENUM(BlueprintType)
enum class EBertaFadeType : uint8
{
	FadeIn,
	FadeOut
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBertaFadeFinishedEvent,
	UBertaFadeWidget*, Widget);

/** Fullscreen Runtime fade that animates a native solid-black layer using real elapsed time. */
UCLASS(Blueprintable)
class BERTADEVKIT_API UBertaFadeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Fade In is black to transparent; Fade Out is transparent to black. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade", meta = (ExposeOnSpawn = true))
	EBertaFadeType FadeType = EBertaFadeType::FadeOut;

	/** Real-time duration in seconds. Zero completes immediately. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade", meta = (ExposeOnSpawn = true, ClampMin = "0.0"))
	float Duration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade", meta = (ExposeOnSpawn = true))
	bool bRemoveOnFinished = true;

	/** Broadcast once after a fade reaches its final opacity and before optional removal. */
	UPROPERTY(BlueprintAssignable, Category = "BertaDevKit|UI|Fade")
	FBertaFadeFinishedEvent OnFadeFinished;

	/** Starts or restarts from the canonical opacity using the current settings. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|UI|Fade")
	void PlayFade();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void FinishFade();

	UPROPERTY(Transient)
	TObjectPtr<UBorder> BlackVisual;

	double FadeStartTime = 0.0;
	float ActiveDuration = 0.0f;
	float StartOpacity = 0.0f;
	float EndOpacity = 1.0f;
	bool bRemoveWhenFinished = true;
	bool bIsFading = false;
};
