#pragma once

#include "BertaDesktopCaptureTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaDesktopCaptureBlueprintLibrary.generated.h"

class UBertaDesktopCaptureSession;

UCLASS()
class BERTADESKTOPCAPTURE_API UBertaDesktopCaptureBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "BertaDesktopCapture", meta = (DisplayName = "Is Desktop Capture Supported"))
	static bool IsDesktopCaptureSupported();

	UFUNCTION(BlueprintCallable, Category = "BertaDesktopCapture|Sources", meta = (ReturnDisplayName = "Success"))
	static bool GetDisplayCaptureSources(
		TArray<FBertaDesktopCaptureSource>& OutSources,
		EBertaDesktopCaptureError& OutError,
		FString& OutErrorMessage);

	UFUNCTION(BlueprintCallable, Category = "BertaDesktopCapture|Sources", meta = (ReturnDisplayName = "Success"))
	static bool GetWindowCaptureSources(
		TArray<FBertaDesktopCaptureSource>& OutSources,
		EBertaDesktopCaptureError& OutError,
		FString& OutErrorMessage);

	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDesktopCapture|Capture",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool StartDesktopCapture(
		const UObject* WorldContextObject,
		const FBertaDesktopCaptureSource& Source,
		const FBertaDesktopCaptureOptions& Options,
		UBertaDesktopCaptureSession*& OutSession,
		EBertaDesktopCaptureError& OutError,
		FString& OutErrorMessage);
};
