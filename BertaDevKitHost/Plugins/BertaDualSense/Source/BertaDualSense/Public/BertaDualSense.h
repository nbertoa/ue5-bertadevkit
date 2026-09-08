#pragma once

#include "CoreMinimal.h"
#include "IInputDeviceModule.h"
#include "BertaDualSense.generated.h"

class FBertaDualSenseInputDevice;

UENUM(BlueprintType)
enum class EBertaDualSenseModel : uint8 { DualSense, DualSenseEdge };
UENUM(BlueprintType)
enum class EBertaDualSenseConnectionType : uint8 { Unknown, Wired, Wireless };
UENUM(BlueprintType)
enum class EBertaDualSensePowerState : uint8 { Unknown, OnBattery, NoBattery, Charging, Charged };

USTRUCT(BlueprintType)
struct BERTADUALSENSE_API FBertaDualSenseDeviceHandle
{
	GENERATED_BODY()
private:
	friend class FBertaDualSenseInputDevice;
	friend class FBertaDualSenseModule;
	int32 DeviceId = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct BERTADUALSENSE_API FBertaDualSenseDeviceInfo
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category="DualSense") FBertaDualSenseDeviceHandle Device;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") EBertaDualSenseModel Model = EBertaDualSenseModel::DualSense;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") EBertaDualSenseConnectionType ConnectionType = EBertaDualSenseConnectionType::Unknown;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") EBertaDualSensePowerState PowerState = EBertaDualSensePowerState::Unknown;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") int32 BatteryPercent = -1;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") int32 FirmwareVersion = 0;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") FString SerialNumber;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") FString Name;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") int32 ProductId = 0;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bSupportsRumble = false;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bSupportsRgbLed = false;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bSupportsPlayerLed = false;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bHasTouchpad = false;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bSupportsAccelerometer = false;
	UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bSupportsGyroscope = false;
};

class FBertaDualSenseModule final : public IInputDeviceModule
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual TSharedPtr<IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;
	virtual bool SupportsDynamicReloading() override { return false; }
	void SetMicrophoneLed(int32 ControllerId, bool bEnabled);
	void GetConnectedDevices(TArray<FBertaDualSenseDeviceInfo>& OutDevices) const;
	void GetDevicesForControllerId(int32 ControllerId, TArray<FBertaDualSenseDeviceInfo>& OutDevices) const;
	bool GetDeviceInfo(const FBertaDualSenseDeviceHandle& Device, FBertaDualSenseDeviceInfo& OutInfo) const;
	bool IsDeviceConnected(const FBertaDualSenseDeviceHandle& Device) const;
	bool SetLightColorForDevice(const FBertaDualSenseDeviceHandle& Device, FColor Color);
	bool ResetLightColorForDevice(const FBertaDualSenseDeviceHandle& Device);
	bool SetMicrophoneLedForDevice(const FBertaDualSenseDeviceHandle& Device, bool bEnabled);

private:
	void HandleEnginePreExit();
	void ShutdownInputDevices();

	TArray<TWeakPtr<FBertaDualSenseInputDevice>> CreatedInputDevices;
	FDelegateHandle EnginePreExitHandle;
	void* SDL3DllHandle = nullptr;
	bool bSDLGamepadSubsystemInitialized = false;
	bool bInputDeviceModularFeatureRegistered = false;
};
