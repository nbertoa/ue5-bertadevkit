#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaWindowTypes.h"

#include "BertaWindowBlueprintLibrary.generated.h"

UCLASS()
class BERTAWINDOWTOOLS_API UBertaWindowBlueprintLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BertaWindowTools|Window",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool GetGameWindowInfo(
		const UObject* WorldContextObject,
		FBertaWindowInfo& OutInfo,
		EBertaWindowError& OutError);

	UFUNCTION(BlueprintCallable, Category = "BertaWindowTools|Window",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool SetWindowPosition(
		const UObject* WorldContextObject,
		FIntPoint DesktopPosition,
		EBertaWindowError& OutError);

	UFUNCTION(BlueprintCallable, Category = "BertaWindowTools|Window",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool SetWindowClientSize(
		const UObject* WorldContextObject,
		FIntPoint ClientSize,
		EBertaWindowError& OutError);

	UFUNCTION(BlueprintCallable, Category = "BertaWindowTools|Displays",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success", CPP_Default_bUseWorkArea = "true"))
	static bool CenterWindowOnDisplay(
		const UObject* WorldContextObject,
		const FString& DisplayId,
		bool bUseWorkArea,
		EBertaWindowError& OutError);

	UFUNCTION(BlueprintCallable, Category = "BertaWindowTools|Displays",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success", CPP_Default_bUseWorkArea = "true"))
	static bool CenterWindowOnPrimaryDisplay(
		const UObject* WorldContextObject,
		bool bUseWorkArea,
		EBertaWindowError& OutError);

	UFUNCTION(BlueprintCallable, Category = "BertaWindowTools|Window",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool MaximizeWindow(
		const UObject* WorldContextObject,
		EBertaWindowError& OutError);

	UFUNCTION(BlueprintCallable, Category = "BertaWindowTools|Window",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool MinimizeWindow(
		const UObject* WorldContextObject,
		EBertaWindowError& OutError);

	UFUNCTION(BlueprintCallable, Category = "BertaWindowTools|Window",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool RestoreWindow(
		const UObject* WorldContextObject,
		EBertaWindowError& OutError);

	UFUNCTION(BlueprintCallable, Category = "BertaWindowTools|Window",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool BringWindowToFront(
		const UObject* WorldContextObject,
		EBertaWindowError& OutError);

	UFUNCTION(BlueprintCallable, Category = "BertaWindowTools|Window",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool SetWindowTitle(
		const UObject* WorldContextObject,
		const FString& Title,
		EBertaWindowError& OutError);
};
