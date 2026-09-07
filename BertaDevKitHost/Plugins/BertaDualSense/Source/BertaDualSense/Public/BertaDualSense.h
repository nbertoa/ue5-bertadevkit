#pragma once

#include "IInputDeviceModule.h"

class FBertaDualSenseModule final : public IInputDeviceModule
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual TSharedPtr<IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;
	virtual bool SupportsDynamicReloading() override { return false; }

private:
	bool bSDLGamepadSubsystemInitialized = false;
	bool bInputDeviceModularFeatureRegistered = false;
};
