#pragma once

#include "BertaSystemInfoTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaSystemInfoBlueprintLibrary.generated.h"

UCLASS()
class BERTASYSTEMINFO_API UBertaSystemInfoBlueprintLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "BertaSystemInfo|System")
	static FBertaOperatingSystemInfo GetOperatingSystemInfo();

	UFUNCTION(BlueprintPure, Category = "BertaSystemInfo|System")
	static FBertaCPUInfo GetCPUInfo();

	UFUNCTION(BlueprintPure, Category = "BertaSystemInfo|System")
	static FBertaGPUInfo GetGPUInfo();

	UFUNCTION(BlueprintCallable, Category = "BertaSystemInfo|System")
	static FBertaMemoryInfo GetMemoryInfo();

	UFUNCTION(BlueprintCallable, Category = "BertaSystemInfo|Displays")
	static TArray<FBertaDisplayInfo> GetDisplays();

	UFUNCTION(BlueprintCallable, Category = "BertaSystemInfo|Displays", meta = (ReturnDisplayName = "Found"))
	static bool GetPrimaryDisplay(FBertaDisplayInfo& OutDisplay);

	UFUNCTION(BlueprintCallable, Category = "BertaSystemInfo|Displays", meta = (ReturnDisplayName = "Success"))
	static bool GetAvailableDisplayModes(const FString& DisplayId, TArray<FBertaDisplayMode>& OutModes);

	UFUNCTION(BlueprintCallable, Category = "BertaSystemInfo|Audio")
	static TArray<FBertaAudioInputDeviceInfo> GetAudioInputDevices();

	UFUNCTION(BlueprintCallable, Category = "BertaSystemInfo|Audio", meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool GetAudioOutputDevices(const UObject* WorldContextObject, TArray<FBertaAudioOutputDeviceInfo>& OutDevices);
};
