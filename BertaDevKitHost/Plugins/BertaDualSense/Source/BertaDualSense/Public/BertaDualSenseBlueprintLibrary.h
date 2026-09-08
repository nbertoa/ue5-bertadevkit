#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaDualSense.h"
#include "BertaDualSenseBlueprintLibrary.generated.h"

UCLASS()
class BERTADUALSENSE_API UBertaDualSenseBlueprintLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BertaDualSense|Output")
	static bool SetMicrophoneLed(int32 ControllerId, bool bEnabled);
	UFUNCTION(BlueprintPure, Category="BertaDualSense|Devices") static TArray<FBertaDualSenseDeviceInfo> GetConnectedDualSenseDevices();
	UFUNCTION(BlueprintPure, Category="BertaDualSense|Devices", meta=(ReturnDisplayName="Found")) static bool GetDualSenseDeviceInfo(const FBertaDualSenseDeviceHandle& Device, FBertaDualSenseDeviceInfo& DeviceInfo);
	UFUNCTION(BlueprintPure, Category="BertaDualSense|Devices") static bool IsDualSenseDeviceConnected(const FBertaDualSenseDeviceHandle& Device);
	UFUNCTION(BlueprintCallable, Category="BertaDualSense|Output") static bool SetDualSenseLightColorForDevice(const FBertaDualSenseDeviceHandle& Device, FColor Color);
	UFUNCTION(BlueprintCallable, Category="BertaDualSense|Output") static bool ResetDualSenseLightColorForDevice(const FBertaDualSenseDeviceHandle& Device);
	UFUNCTION(BlueprintCallable, Category="BertaDualSense|Output") static bool SetDualSenseMicrophoneLedForDevice(const FBertaDualSenseDeviceHandle& Device, bool bEnabled);
};
