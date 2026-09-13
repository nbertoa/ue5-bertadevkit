#pragma once

#include "BertaDesktopCaptureTypes.h"
#include "CoreMinimal.h"

class FBertaDesktopCaptureDispatcher;

/** Win64-only Windows Graphics Capture backend. Never exposes native handles publicly. */
class FBertaWindowsCaptureBackend final
{
public:
	FBertaWindowsCaptureBackend();
	~FBertaWindowsCaptureBackend();

	FBertaWindowsCaptureBackend(const FBertaWindowsCaptureBackend&) = delete;
	FBertaWindowsCaptureBackend& operator=(const FBertaWindowsCaptureBackend&) = delete;

	static bool IsSupported();
	static bool EnumerateWindows(TArray<FBertaDesktopCaptureSource>& OutSources, FString& OutErrorMessage);

	bool Initialize(
		const FBertaDesktopCaptureSource& Source,
		int32 MaxFrameRate,
		const TSharedRef<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe>& Dispatcher,
		FIntPoint& OutInitialSize,
		EBertaDesktopCaptureError& OutError,
		FString& OutErrorMessage);
	void Stop();

private:
	class FImplementation;
	TUniquePtr<FImplementation> Implementation;
};
