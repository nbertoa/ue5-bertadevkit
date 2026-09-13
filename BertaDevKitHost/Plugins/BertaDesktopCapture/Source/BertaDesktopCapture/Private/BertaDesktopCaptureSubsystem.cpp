#include "BertaDesktopCaptureSubsystem.h"

#include "BertaDesktopCapture.h"
#include "BertaDesktopCaptureSession.h"

void UBertaDesktopCaptureSubsystem::Deinitialize()
{
	bIsShuttingDown = true;

	TArray<TObjectPtr<UBertaDesktopCaptureSession>> SessionsToStop = MoveTemp(ActiveSessions);
	ActiveSessions.Reset();
	for (UBertaDesktopCaptureSession* Session : SessionsToStop)
	{
		if (Session != nullptr)
		{
			Session->ShutdownForOwner();
		}
	}

	Super::Deinitialize();
}

bool UBertaDesktopCaptureSubsystem::StartCapture(
	const FBertaDesktopCaptureSource& Source,
	const FBertaDesktopCaptureOptions& Options,
	UBertaDesktopCaptureSession*& OutSession,
	EBertaDesktopCaptureError& OutError,
	FString& OutErrorMessage)
{
	OutSession = nullptr;
	OutError = EBertaDesktopCaptureError::None;
	OutErrorMessage.Reset();

	if (!IsInGameThread() || bIsShuttingDown)
	{
		OutError = EBertaDesktopCaptureError::SubsystemUnavailable;
		OutErrorMessage = TEXT("The BertaDesktopCapture GameInstance subsystem is unavailable or shutting down.");
		return false;
	}
	if (Options.MaxFrameRate < 1 || Options.MaxFrameRate > 60)
	{
		OutError = EBertaDesktopCaptureError::InvalidOptions;
		OutErrorMessage = TEXT("MaxFrameRate must be between 1 and 60 inclusive.");
		return false;
	}
	if (Source.Id.IsEmpty())
	{
		OutError = EBertaDesktopCaptureError::InvalidSource;
		OutErrorMessage = TEXT("Desktop capture source Id must not be empty.");
		return false;
	}
	UBertaDesktopCaptureSession* Session = NewObject<UBertaDesktopCaptureSession>(this);
	if (!Session->InitializeInternal(Source, Options, this, OutError, OutErrorMessage))
	{
		UE_LOG(LogBertaDesktopCapture, Warning, TEXT("Desktop capture launch failed: %s"), *OutErrorMessage);
		return false;
	}

	ActiveSessions.Add(Session);
	OutSession = Session;
	return true;
}

void UBertaDesktopCaptureSubsystem::NotifySessionStopped(UBertaDesktopCaptureSession* Session)
{
	check(IsInGameThread());
	ActiveSessions.RemoveSingleSwap(Session);
}
