#pragma once

#include "CoreMinimal.h"

#include "BertaWindowTypes.generated.h"

UENUM(BlueprintType)
enum class EBertaWindowMode : uint8
{
	Unknown,
	Windowed,
	WindowedFullscreen,
	Fullscreen
};

UENUM(BlueprintType)
enum class EBertaWindowError : uint8
{
	None,
	InvalidWorldContext,
	NoGameInstance,
	NoGameViewport,
	NoWindow,
	EmbeddedViewportUnsupported,
	InvalidSize,
	DisplayNotFound,
	WindowNotWindowed,
	WindowNotRestored,
	OperationUnsupported
};

USTRUCT(BlueprintType)
struct BERTAWINDOWTOOLS_API FBertaWindowInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Window")
	FString Title;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Window")
	FIntPoint DesktopPosition = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Window")
	FIntPoint WindowSize = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Window")
	FIntPoint ClientSize = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Window")
	float DPIScale = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Window")
	EBertaWindowMode WindowMode = EBertaWindowMode::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Window")
	bool bIsVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Window")
	bool bIsActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Window")
	bool bIsForeground = false;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Window")
	bool bIsMinimized = false;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Window")
	bool bIsMaximized = false;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Displays")
	bool bHasDisplay = false;

	UPROPERTY(BlueprintReadOnly, Category = "BertaWindowTools|Displays")
	FString DisplayId;
};
