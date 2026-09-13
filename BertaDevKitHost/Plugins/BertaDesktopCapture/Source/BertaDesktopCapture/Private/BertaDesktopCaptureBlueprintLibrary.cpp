#include "BertaDesktopCaptureBlueprintLibrary.h"

#include "BertaDesktopCaptureSession.h"
#include "BertaDesktopCaptureSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericApplication.h"
#include "Windows/BertaWindowsCaptureBackend.h"

bool UBertaDesktopCaptureBlueprintLibrary::IsDesktopCaptureSupported()
{
	return FBertaWindowsCaptureBackend::IsSupported();
}

bool UBertaDesktopCaptureBlueprintLibrary::GetDisplayCaptureSources(
	TArray<FBertaDesktopCaptureSource>& OutSources,
	EBertaDesktopCaptureError& OutError,
	FString& OutErrorMessage)
{
	OutSources.Reset();
	OutError = EBertaDesktopCaptureError::None;
	OutErrorMessage.Reset();
	if (!FBertaWindowsCaptureBackend::IsSupported())
	{
		OutError = EBertaDesktopCaptureError::UnsupportedOperatingSystem;
		OutErrorMessage = TEXT("Windows Graphics Capture programmatic interop is unavailable; Windows 10 version 1903 or later is required.");
		return false;
	}

	FDisplayMetrics Metrics;
	FDisplayMetrics::RebuildDisplayMetrics(Metrics);
	for (const FMonitorInfo& Monitor : Metrics.MonitorInfo)
	{
		const int32 Width = Monitor.DisplayRect.Right - Monitor.DisplayRect.Left;
		const int32 Height = Monitor.DisplayRect.Bottom - Monitor.DisplayRect.Top;
		if (Monitor.ID.IsEmpty() || Monitor.NativeHandle == nullptr || Width <= 0 || Height <= 0)
		{
			continue;
		}

		FBertaDesktopCaptureSource& Source = OutSources.AddDefaulted_GetRef();
		Source.Type = EBertaDesktopCaptureSourceType::Display;
		Source.Id = Monitor.ID;
		Source.Name = Monitor.FriendlyName.IsEmpty() ? Monitor.Name : Monitor.FriendlyName;
		Source.Size = FIntPoint(Width, Height);
		Source.bIsPrimaryDisplay = Monitor.bIsPrimary;
	}
	return true;
}

bool UBertaDesktopCaptureBlueprintLibrary::GetWindowCaptureSources(
	TArray<FBertaDesktopCaptureSource>& OutSources,
	EBertaDesktopCaptureError& OutError,
	FString& OutErrorMessage)
{
	OutSources.Reset();
	OutError = EBertaDesktopCaptureError::None;
	OutErrorMessage.Reset();
	if (!FBertaWindowsCaptureBackend::IsSupported())
	{
		OutError = EBertaDesktopCaptureError::UnsupportedOperatingSystem;
		OutErrorMessage = TEXT("Windows Graphics Capture programmatic interop is unavailable; Windows 10 version 1903 or later is required.");
		return false;
	}
	if (!FBertaWindowsCaptureBackend::EnumerateWindows(OutSources, OutErrorMessage))
	{
		OutError = EBertaDesktopCaptureError::CaptureFailed;
		return false;
	}
	return true;
}

bool UBertaDesktopCaptureBlueprintLibrary::StartDesktopCapture(
	const UObject* WorldContextObject,
	const FBertaDesktopCaptureSource& Source,
	const FBertaDesktopCaptureOptions& Options,
	UBertaDesktopCaptureSession*& OutSession,
	EBertaDesktopCaptureError& OutError,
	FString& OutErrorMessage)
{
	OutSession = nullptr;
	OutError = EBertaDesktopCaptureError::None;
	OutErrorMessage.Reset();

	if (!FBertaWindowsCaptureBackend::IsSupported())
	{
		OutError = EBertaDesktopCaptureError::UnsupportedOperatingSystem;
		OutErrorMessage = TEXT("Windows Graphics Capture programmatic interop is unavailable; Windows 10 version 1903 or later is required.");
		return false;
	}
	if (Options.MaxFrameRate < 1 || Options.MaxFrameRate > 60)
	{
		OutError = EBertaDesktopCaptureError::InvalidOptions;
		OutErrorMessage = TEXT("MaxFrameRate must be between 1 and 60 inclusive.");
		return false;
	}

	if (!IsInGameThread() || GEngine == nullptr || WorldContextObject == nullptr)
	{
		OutError = EBertaDesktopCaptureError::InvalidWorldContext;
		OutErrorMessage = TEXT("A valid WorldContextObject on the Game Thread is required.");
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
	if (GameInstance == nullptr)
	{
		OutError = EBertaDesktopCaptureError::InvalidWorldContext;
		OutErrorMessage = TEXT("No GameInstance is available for the supplied world context.");
		return false;
	}

	UBertaDesktopCaptureSubsystem* Subsystem = GameInstance->GetSubsystem<UBertaDesktopCaptureSubsystem>();
	if (Subsystem == nullptr)
	{
		OutError = EBertaDesktopCaptureError::SubsystemUnavailable;
		OutErrorMessage = TEXT("No BertaDesktopCapture subsystem is available for the supplied GameInstance.");
		return false;
	}

	return Subsystem->StartCapture(Source, Options, OutSession, OutError, OutErrorMessage);
}
