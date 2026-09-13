#pragma once

#include "BertaDesktopCaptureTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "BertaDesktopCaptureSubsystem.generated.h"

class UBertaDesktopCaptureSession;

/** Owns active desktop captures for exactly one GameInstance lifetime. */
UCLASS()
class BERTADESKTOPCAPTURE_API UBertaDesktopCaptureSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	bool StartCapture(
		const FBertaDesktopCaptureSource& Source,
		const FBertaDesktopCaptureOptions& Options,
		UBertaDesktopCaptureSession*& OutSession,
		EBertaDesktopCaptureError& OutError,
		FString& OutErrorMessage);

private:
	friend class UBertaDesktopCaptureSession;

	void NotifySessionStopped(UBertaDesktopCaptureSession* Session);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBertaDesktopCaptureSession>> ActiveSessions;

	bool bIsShuttingDown = false;
};
