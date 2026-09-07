#pragma once

#include "IInputDeviceModule.h"

class FBertaDualSenseInputDevice;

class FBertaDualSenseModule final : public IInputDeviceModule
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual TSharedPtr<IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;
	virtual bool SupportsDynamicReloading() override { return false; }
	void SetMicrophoneLed(int32 ControllerId, bool bEnabled);

private:
	void HandleEnginePreExit();
	void ShutdownInputDevices();

	TArray<TWeakPtr<FBertaDualSenseInputDevice>> CreatedInputDevices;
	FDelegateHandle EnginePreExitHandle;
	void* SDL3DllHandle = nullptr;
	bool bSDLGamepadSubsystemInitialized = false;
	bool bInputDeviceModularFeatureRegistered = false;
};
