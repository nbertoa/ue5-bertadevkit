#include "BertaDualSense.h"

#include "Features/IModularFeatures.h"
#include "SDL3/SDL.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaDualSense, Log, All);

namespace
{
	class FBertaDualSenseInputDevice final : public IInputDevice
	{
	public:
		explicit FBertaDualSenseInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
			: MessageHandler(InMessageHandler)
		{
		}

		virtual void Tick(float DeltaTime) override {}
		virtual void SendControllerEvents() override {}

		virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override
		{
			MessageHandler = InMessageHandler;
		}

		virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override { return false; }
		virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override {}
		virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) override {}
		virtual bool SupportsForceFeedback(int32 ControllerId) override { return false; }

	private:
		TSharedRef<FGenericApplicationMessageHandler> MessageHandler;
	};
}

void FBertaDualSenseModule::StartupModule()
{
	IInputDeviceModule::StartupModule();

	bSDLGamepadSubsystemInitialized = SDL_InitSubSystem(SDL_INIT_GAMEPAD);
	if (!bSDLGamepadSubsystemInitialized)
	{
		UE_LOG(LogBertaDualSense, Error, TEXT("Failed to initialize SDL gamepad subsystem: %s"), UTF8_TO_TCHAR(SDL_GetError()));
		return;
	}

	SDL_SetGamepadEventsEnabled(false);
	UE_LOG(LogBertaDualSense, Log, TEXT("Initialized SDL gamepad subsystem with gamepad events disabled."));
}

void FBertaDualSenseModule::ShutdownModule()
{
	if (bSDLGamepadSubsystemInitialized)
	{
		SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
		bSDLGamepadSubsystemInitialized = false;
		UE_LOG(LogBertaDualSense, Log, TEXT("Shut down SDL gamepad subsystem."));
	}

	IModularFeatures::Get().UnregisterModularFeature(GetModularFeatureName(), this);
}

TSharedPtr<IInputDevice> FBertaDualSenseModule::CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	if (!bSDLGamepadSubsystemInitialized)
	{
		return nullptr;
	}

	return MakeShared<FBertaDualSenseInputDevice>(InMessageHandler);
}

IMPLEMENT_MODULE(FBertaDualSenseModule, BertaDualSense)