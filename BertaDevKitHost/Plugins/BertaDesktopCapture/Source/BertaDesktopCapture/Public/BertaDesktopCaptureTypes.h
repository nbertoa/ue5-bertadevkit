#pragma once

#include "CoreMinimal.h"

#include "BertaDesktopCaptureTypes.generated.h"

UENUM(BlueprintType)
enum class EBertaDesktopCaptureSourceType : uint8
{
	Display,
	Window
};

UENUM(BlueprintType)
enum class EBertaDesktopCaptureError : uint8
{
	None,
	UnsupportedPlatform,
	UnsupportedOperatingSystem,
	InvalidWorldContext,
	SubsystemUnavailable,
	InvalidSource,
	SourceUnavailable,
	InvalidOptions,
	CaptureInitializationFailed,
	TextureCreationFailed,
	CaptureFailed
};

UENUM(BlueprintType)
enum class EBertaDesktopCaptureStopReason : uint8
{
	UserStopped,
	SourceClosed,
	CaptureFailed
};

USTRUCT(BlueprintType)
struct BERTADESKTOPCAPTURE_API FBertaDesktopCaptureSource
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Desktop Capture")
	EBertaDesktopCaptureSourceType Type = EBertaDesktopCaptureSourceType::Display;

	UPROPERTY(BlueprintReadOnly, Category = "Desktop Capture")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Desktop Capture")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Desktop Capture")
	FIntPoint Size = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "Desktop Capture")
	bool bIsPrimaryDisplay = false;
};

USTRUCT(BlueprintType)
struct BERTADESKTOPCAPTURE_API FBertaDesktopCaptureOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desktop Capture", meta = (ClampMin = "1", ClampMax = "60"))
	int32 MaxFrameRate = 30;
};
