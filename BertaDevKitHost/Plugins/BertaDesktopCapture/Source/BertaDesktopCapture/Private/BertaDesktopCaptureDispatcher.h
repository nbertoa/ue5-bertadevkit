#pragma once

#include "BertaDesktopCaptureTypes.h"
#include "CoreMinimal.h"

class UBertaDesktopCaptureSession;

/** Bounded cross-thread mailbox: resize plus only the newest complete frame. */
class FBertaDesktopCaptureDispatcher final
	: public TSharedFromThis<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe>
{
public:
	explicit FBertaDesktopCaptureDispatcher(UBertaDesktopCaptureSession* InSession);

	void PublishFrame(FIntPoint Size, TArray<uint8>&& Pixels);
	void PublishSizeChanged(FIntPoint Size);
	void PublishStopped(EBertaDesktopCaptureStopReason Reason, FString&& ErrorMessage);
	void SetUploadBlocked(bool bBlocked);
	void Suppress();

private:
	void ScheduleDrain();
	void DrainOnGameThread();

	FCriticalSection Mutex;
	TWeakObjectPtr<UBertaDesktopCaptureSession> Session;
	TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> PendingPixels;
	FIntPoint PendingFrameSize = FIntPoint::ZeroValue;
	FIntPoint PendingSizeChange = FIntPoint::ZeroValue;
	EBertaDesktopCaptureStopReason PendingStopReason = EBertaDesktopCaptureStopReason::CaptureFailed;
	FString PendingStopError;
	bool bHasSizeChange = false;
	bool bHasStop = false;
	bool bDrainScheduled = false;
	bool bUploadBlocked = false;
	bool bSuppressed = false;
};
