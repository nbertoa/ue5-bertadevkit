#include "BertaDesktopCaptureDispatcher.h"

#include "Async/Async.h"
#include "BertaDesktopCaptureSession.h"

FBertaDesktopCaptureDispatcher::FBertaDesktopCaptureDispatcher(UBertaDesktopCaptureSession* InSession)
	: Session(InSession)
{
}

void FBertaDesktopCaptureDispatcher::PublishFrame(const FIntPoint Size, TArray<uint8>&& Pixels)
{
	bool bShouldSchedule = false;
	{
		FScopeLock Lock(&Mutex);
		if (bSuppressed || bHasStop)
		{
			return;
		}

		PendingPixels = MakeShared<TArray<uint8>, ESPMode::ThreadSafe>(MoveTemp(Pixels));
		PendingFrameSize = Size;
		bShouldSchedule = !bUploadBlocked && !bDrainScheduled;
		bDrainScheduled |= bShouldSchedule;
	}

	if (bShouldSchedule)
	{
		ScheduleDrain();
	}
}

void FBertaDesktopCaptureDispatcher::PublishSizeChanged(const FIntPoint Size)
{
	bool bShouldSchedule = false;
	{
		FScopeLock Lock(&Mutex);
		if (bSuppressed || bHasStop)
		{
			return;
		}

		PendingSizeChange = Size;
		bHasSizeChange = true;
		PendingPixels.Reset();
		PendingFrameSize = FIntPoint::ZeroValue;
		bShouldSchedule = !bDrainScheduled;
		bDrainScheduled = true;
	}

	if (bShouldSchedule)
	{
		ScheduleDrain();
	}
}

void FBertaDesktopCaptureDispatcher::PublishStopped(
	const EBertaDesktopCaptureStopReason Reason,
	FString&& ErrorMessage)
{
	bool bShouldSchedule = false;
	{
		FScopeLock Lock(&Mutex);
		if (bSuppressed || bHasStop)
		{
			return;
		}

		PendingPixels.Reset();
		bHasSizeChange = false;
		bHasStop = true;
		PendingStopReason = Reason;
		PendingStopError = MoveTemp(ErrorMessage);
		bShouldSchedule = !bDrainScheduled;
		bDrainScheduled = true;
	}

	if (bShouldSchedule)
	{
		ScheduleDrain();
	}
}

void FBertaDesktopCaptureDispatcher::Suppress()
{
	FScopeLock Lock(&Mutex);
	bSuppressed = true;
	PendingPixels.Reset();
	bHasSizeChange = false;
	bHasStop = false;
	Session.Reset();
}

void FBertaDesktopCaptureDispatcher::SetUploadBlocked(const bool bBlocked)
{
	bool bShouldSchedule = false;
	{
		FScopeLock Lock(&Mutex);
		bUploadBlocked = bBlocked;
		bShouldSchedule = !bBlocked && !bSuppressed && !bDrainScheduled && PendingPixels.IsValid();
		bDrainScheduled |= bShouldSchedule;
	}

	if (bShouldSchedule)
	{
		ScheduleDrain();
	}
}

void FBertaDesktopCaptureDispatcher::ScheduleDrain()
{
	TSharedRef<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe> Self = AsShared();
	AsyncTask(ENamedThreads::GameThread, [Self]()
	{
		Self->DrainOnGameThread();
	});
}

void FBertaDesktopCaptureDispatcher::DrainOnGameThread()
{
	check(IsInGameThread());

	TWeakObjectPtr<UBertaDesktopCaptureSession> TargetSession;
	TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Pixels;
	FIntPoint FrameSize = FIntPoint::ZeroValue;
	FIntPoint SizeChange = FIntPoint::ZeroValue;
	EBertaDesktopCaptureStopReason StopReason = EBertaDesktopCaptureStopReason::CaptureFailed;
	FString StopError;
	bool bDispatchSizeChange = false;
	bool bDispatchStop = false;

	{
		FScopeLock Lock(&Mutex);
		bDrainScheduled = false;
		if (bSuppressed)
		{
			return;
		}

		TargetSession = Session;
		Pixels = MoveTemp(PendingPixels);
		FrameSize = PendingFrameSize;
		PendingFrameSize = FIntPoint::ZeroValue;
		bDispatchSizeChange = bHasSizeChange;
		SizeChange = PendingSizeChange;
		bHasSizeChange = false;
		bDispatchStop = bHasStop;
		StopReason = PendingStopReason;
		StopError = MoveTemp(PendingStopError);
		bHasStop = false;
	}

	UBertaDesktopCaptureSession* Target = TargetSession.Get();
	if (Target == nullptr)
	{
		return;
	}

	if (bDispatchSizeChange)
	{
		Target->HandleCaptureSizeChanged(SizeChange);
	}
	if (Pixels.IsValid())
	{
		bool bBlocked = false;
		{
			FScopeLock Lock(&Mutex);
			bBlocked = bUploadBlocked;
			if (bBlocked && !bSuppressed && !bHasStop && !PendingPixels.IsValid())
			{
				PendingPixels = MoveTemp(Pixels);
				PendingFrameSize = FrameSize;
			}
		}
		if (!bBlocked)
		{
			Target->HandleCapturedFrame(FrameSize, MoveTemp(Pixels));
		}
	}
	if (bDispatchStop)
	{
		Target->HandleNativeStopped(StopReason, StopError);
	}
}
