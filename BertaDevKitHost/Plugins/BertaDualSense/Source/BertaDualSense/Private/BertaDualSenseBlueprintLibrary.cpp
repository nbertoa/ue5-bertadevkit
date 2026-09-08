#include "BertaDualSenseBlueprintLibrary.h"

#include "BertaDualSense.h"
#include "Modules/ModuleManager.h"

bool UBertaDualSenseBlueprintLibrary::SetMicrophoneLed(int32 ControllerId, bool bEnabled)
{
	if (FBertaDualSenseModule* Module = FModuleManager::GetModulePtr<FBertaDualSenseModule>(TEXT("BertaDualSense")))
	{
		Module->SetMicrophoneLed(ControllerId, bEnabled);
		return true;
	}

	return false;
}

TArray<FBertaDualSenseDeviceInfo> UBertaDualSenseBlueprintLibrary::GetConnectedDualSenseDevices() { TArray<FBertaDualSenseDeviceInfo> Devices; if (const FBertaDualSenseModule* Module=FModuleManager::GetModulePtr<FBertaDualSenseModule>(TEXT("BertaDualSense"))) Module->GetConnectedDevices(Devices); return Devices; }
bool UBertaDualSenseBlueprintLibrary::GetDualSenseDeviceInfo(const FBertaDualSenseDeviceHandle& Device, FBertaDualSenseDeviceInfo& DeviceInfo) { if(const FBertaDualSenseModule* Module=FModuleManager::GetModulePtr<FBertaDualSenseModule>(TEXT("BertaDualSense"))) return Module->GetDeviceInfo(Device,DeviceInfo); DeviceInfo={};return false; }
bool UBertaDualSenseBlueprintLibrary::IsDualSenseDeviceConnected(const FBertaDualSenseDeviceHandle& Device) { if(const FBertaDualSenseModule* Module=FModuleManager::GetModulePtr<FBertaDualSenseModule>(TEXT("BertaDualSense"))) return Module->IsDeviceConnected(Device); return false; }
TArray<FBertaDualSenseDeviceInfo> UBertaDualSenseBlueprintLibrary::GetDualSenseDevicesForControllerId(int32 ControllerId) { TArray<FBertaDualSenseDeviceInfo> Devices; if(const FBertaDualSenseModule* Module=FModuleManager::GetModulePtr<FBertaDualSenseModule>(TEXT("BertaDualSense")))Module->GetDevicesForControllerId(ControllerId,Devices);return Devices; }
bool UBertaDualSenseBlueprintLibrary::SetDualSenseLightColorForDevice(const FBertaDualSenseDeviceHandle& D,FColor C) { if(FBertaDualSenseModule* M=FModuleManager::GetModulePtr<FBertaDualSenseModule>(TEXT("BertaDualSense")))return M->SetLightColorForDevice(D,C);return false; }
bool UBertaDualSenseBlueprintLibrary::ResetDualSenseLightColorForDevice(const FBertaDualSenseDeviceHandle& D) { if(FBertaDualSenseModule* M=FModuleManager::GetModulePtr<FBertaDualSenseModule>(TEXT("BertaDualSense")))return M->ResetLightColorForDevice(D);return false; }
bool UBertaDualSenseBlueprintLibrary::SetDualSenseMicrophoneLedForDevice(const FBertaDualSenseDeviceHandle& D,bool B) { if(FBertaDualSenseModule* M=FModuleManager::GetModulePtr<FBertaDualSenseModule>(TEXT("BertaDualSense")))return M->SetMicrophoneLedForDevice(D,B);return false; }
