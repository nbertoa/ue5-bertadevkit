#include "BertaDualSense.h"

#include "Features/IModularFeatures.h"
#include "GenericPlatform/GenericInputDeviceMap.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "GenericPlatform/InputDeviceRegistry.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/Paths.h"
#include "SDL3/SDL.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaDualSense, Log, All);

namespace
{
	constexpr Uint16 SonyVendorId = 0x054C;
	constexpr Uint16 DualSenseProductId = 0x0CE6;
	constexpr Uint16 DualSenseEdgeProductId = 0x0DF2;

	FString ToUnrealString(const char* String)
	{
		return String ? UTF8_TO_TCHAR(String) : FString();
	}

	bool IsSupportedDualSense(const Uint16 VendorId, const Uint16 ProductId)
	{
		return VendorId == SonyVendorId && (ProductId == DualSenseProductId || ProductId == DualSenseEdgeProductId);
	}

	FName GetHardwareDeviceIdentifier(const Uint16 ProductId)
	{
		return ProductId == DualSenseEdgeProductId ? FName(TEXT("DualSenseEdge")) : FName(TEXT("DualSense"));
	}

	struct FConnectedDualSense
	{
		SDL_Gamepad* Gamepad = nullptr;
		FInputDeviceId InputDeviceId = INPUTDEVICEID_NONE;
		FPlatformUserId PlatformUserId = PLATFORMUSERID_NONE;
	};

	class FBertaDualSenseInputDevice final : public IInputDevice
	{
	public:
		explicit FBertaDualSenseInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
			: MessageHandler(InMessageHandler)
		{
		}

		virtual ~FBertaDualSenseInputDevice() override
		{
			DisconnectAllDevices();
		}

		virtual void Tick(float DeltaTime) override
		{
			SDL_UpdateGamepads();

			int GamepadCount = 0;
			SDL_JoystickID* GamepadIds = SDL_GetGamepads(&GamepadCount);
			if (!GamepadIds)
			{
				if (!bGamepadEnumerationFailureLogged)
				{
					bGamepadEnumerationFailureLogged = true;
					UE_LOG(LogBertaDualSense, Error, TEXT("SDL_GetGamepads failed; preserving current DualSense connections: %s"), UTF8_TO_TCHAR(SDL_GetError()));
				}
				return;
			}

			bGamepadEnumerationFailureLogged = false;
			TSet<SDL_JoystickID> PresentGamepads;

			for (int Index = 0; Index < GamepadCount; ++Index)
			{
				PresentGamepads.Add(GamepadIds[Index]);
			}

			for (auto It = ConnectedDevices.CreateIterator(); It; ++It)
			{
				if (!PresentGamepads.Contains(It.Key()))
				{
					DisconnectDevice(It.Key(), It.Value());
					It.RemoveCurrent();
				}
			}

			for (const SDL_JoystickID InstanceId : PresentGamepads)
			{
				if (ConnectedDevices.Contains(InstanceId))
				{
					continue;
				}

				const Uint16 VendorId = SDL_GetGamepadVendorForID(InstanceId);
				const Uint16 ProductId = SDL_GetGamepadProductForID(InstanceId);
				if (VendorId == 0 || ProductId == 0)
				{
					if (!UnavailableIdentityLogged.Contains(InstanceId))
					{
						UnavailableIdentityLogged.Add(InstanceId);
						UE_LOG(LogBertaDualSense, Warning, TEXT("Ignoring SDL gamepad %u because VID/PID is unavailable (VID=%04X PID=%04X, Name='%s')."), InstanceId, VendorId, ProductId, *ToUnrealString(SDL_GetGamepadNameForID(InstanceId)));
					}
					continue;
				}

				if (IsSupportedDualSense(VendorId, ProductId) && !OpenFailures.Contains(InstanceId))
				{
					ConnectDevice(InstanceId, VendorId, ProductId);
				}
			}

			SDL_free(GamepadIds);

			RemoveNoLongerPresent(OpenFailures, PresentGamepads);
			RemoveNoLongerPresent(UnavailableIdentityLogged, PresentGamepads);
		}
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
		void ConnectDevice(const SDL_JoystickID InstanceId, const Uint16 VendorId, const Uint16 ProductId)
		{
			const double OpenStartTime = FPlatformTime::Seconds();
			SDL_Gamepad* Gamepad = SDL_OpenGamepad(InstanceId);
			const double OpenDurationMilliseconds = (FPlatformTime::Seconds() - OpenStartTime) * 1000.0;
			if (!Gamepad)
			{
				OpenFailures.Add(InstanceId);
				UE_LOG(LogBertaDualSense, Error, TEXT("Failed to open DualSense SDL instance %u after %.3f ms: %s"), InstanceId, OpenDurationMilliseconds, UTF8_TO_TCHAR(SDL_GetError()));
				return;
			}

			const SDL_JoystickID OpenedInstanceId = SDL_GetGamepadID(Gamepad);
			if (OpenedInstanceId != InstanceId)
			{
				OpenFailures.Add(InstanceId);
				SDL_CloseGamepad(Gamepad);
				UE_LOG(LogBertaDualSense, Error, TEXT("SDL opened DualSense instance %u as instance %u; ignoring the inconsistent handle."), InstanceId, OpenedInstanceId);
				return;
			}

			const FString Serial = ToUnrealString(SDL_GetGamepadSerial(Gamepad));
			const FInputDeviceId InputDeviceId = Serial.IsEmpty()
				? IPlatformInputDeviceMapper::Get().AllocateNewInputDeviceId()
				: PersistentDeviceIds.GetOrCreateDeviceId(Serial);
			IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
			const FPlatformUserId PlatformUserId = DeviceMapper.GetPlatformUserForNewlyConnectedDevice();

			FInputDeviceDescriptor Descriptor;
			Descriptor.HardwareDeviceHandle = InputDeviceId;
			Descriptor.InputDeviceName = FName(TEXT("BertaDualSense"));
			Descriptor.HardwareDeviceIdentifier = GetHardwareDeviceIdentifier(ProductId);
			FInputDeviceRegistry::RegisterDevice(Descriptor);

			if (!ensure(DeviceMapper.Internal_MapInputDeviceToUser(InputDeviceId, PlatformUserId, EInputDeviceConnectionState::Connected)))
			{
				SDL_CloseGamepad(Gamepad);
				return;
			}

			FConnectedDualSense& ConnectedDevice = ConnectedDevices.Add(InstanceId);
			ConnectedDevice.Gamepad = Gamepad;
			ConnectedDevice.InputDeviceId = InputDeviceId;
			ConnectedDevice.PlatformUserId = PlatformUserId;
			UE_LOG(LogBertaDualSense, Log, TEXT("Connected %s (SDL instance %u, VID=%04X PID=%04X, Serial='%s', Path='%s') as InputDeviceId %d for PlatformUserId %d; SDL_OpenGamepad took %.3f ms."), *ToUnrealString(SDL_GetGamepadName(Gamepad)), InstanceId, VendorId, ProductId, *Serial, *ToUnrealString(SDL_GetGamepadPath(Gamepad)), InputDeviceId.GetId(), PlatformUserId.GetInternalId(), OpenDurationMilliseconds);
		}

		void DisconnectDevice(const SDL_JoystickID InstanceId, FConnectedDualSense& ConnectedDevice)
		{
			IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
			DeviceMapper.Internal_MapInputDeviceToUser(ConnectedDevice.InputDeviceId, DeviceMapper.GetUserForUnpairedInputDevices(), EInputDeviceConnectionState::Disconnected);
			SDL_CloseGamepad(ConnectedDevice.Gamepad);
			UE_LOG(LogBertaDualSense, Log, TEXT("Disconnected DualSense SDL instance %u from InputDeviceId %d."), InstanceId, ConnectedDevice.InputDeviceId.GetId());
		}

		void DisconnectAllDevices()
		{
			for (TPair<SDL_JoystickID, FConnectedDualSense>& Pair : ConnectedDevices)
			{
				DisconnectDevice(Pair.Key, Pair.Value);
			}
			ConnectedDevices.Empty();
		}

		static void RemoveNoLongerPresent(TSet<SDL_JoystickID>& InstanceIds, const TSet<SDL_JoystickID>& PresentGamepads)
		{
			for (auto It = InstanceIds.CreateIterator(); It; ++It)
			{
				if (!PresentGamepads.Contains(*It))
				{
					It.RemoveCurrent();
				}
			}
		}

		TSharedRef<FGenericApplicationMessageHandler> MessageHandler;
		TInputDeviceMap<FString> PersistentDeviceIds;
		TMap<SDL_JoystickID, FConnectedDualSense> ConnectedDevices;
		TSet<SDL_JoystickID> OpenFailures;
		TSet<SDL_JoystickID> UnavailableIdentityLogged;
		bool bGamepadEnumerationFailureLogged = false;
	};
}

void FBertaDualSenseModule::StartupModule()
{
	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense StartupModule begin"));

	const FString SDL3DllPath = FPaths::ConvertRelativePathToFull(FPlatformProcess::BaseDir(), TEXT("SDL3.dll"));
	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense SDL3 DLL path: %s"), *SDL3DllPath);
	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense before GetDllHandle"));
	SDL3DllHandle = FPlatformProcess::GetDllHandle(*SDL3DllPath);
	if (!SDL3DllHandle)
	{
		UE_LOG(LogBertaDualSense, Error, TEXT("BertaDualSense failed to load SDL3 DLL from %s"), *SDL3DllPath);
		return;
	}
	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense after successful GetDllHandle"));

	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense before SDL_GetVersion"));
	const int LinkedSDLVersion = SDL_GetVersion();
	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense linked SDL version: %d.%d.%d"), SDL_VERSIONNUM_MAJOR(LinkedSDLVersion), SDL_VERSIONNUM_MINOR(LinkedSDLVersion), SDL_VERSIONNUM_MICRO(LinkedSDLVersion));
	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense compile-time SDL version: %d.%d.%d"), SDL_VERSIONNUM_MAJOR(SDL_VERSION), SDL_VERSIONNUM_MINOR(SDL_VERSION), SDL_VERSIONNUM_MICRO(SDL_VERSION));

	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense before SDL_InitSubSystem(SDL_INIT_GAMEPAD)"));
	bSDLGamepadSubsystemInitialized = SDL_InitSubSystem(SDL_INIT_GAMEPAD);
	if (!bSDLGamepadSubsystemInitialized)
	{
		UE_LOG(LogBertaDualSense, Error, TEXT("Failed to initialize SDL gamepad subsystem: %s"), UTF8_TO_TCHAR(SDL_GetError()));
		return;
	}
	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense after successful SDL_InitSubSystem"));

	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense before SDL_SetGamepadEventsEnabled(false)"));
	SDL_SetGamepadEventsEnabled(false);
	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense after SDL_SetGamepadEventsEnabled(false)"));

	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense before IInputDeviceModule::StartupModule()"));
	IInputDeviceModule::StartupModule();
	bInputDeviceModularFeatureRegistered = true;
	UE_LOG(LogBertaDualSense, Log, TEXT("BertaDualSense after IInputDeviceModule::StartupModule()"));

	UE_LOG(LogBertaDualSense, Log, TEXT("Initialized SDL gamepad subsystem with gamepad events disabled."));
}

void FBertaDualSenseModule::ShutdownModule()
{
	if (bInputDeviceModularFeatureRegistered)
	{
		IModularFeatures::Get().UnregisterModularFeature(GetModularFeatureName(), this);
		bInputDeviceModularFeatureRegistered = false;
	}

	if (bSDLGamepadSubsystemInitialized)
	{
		SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
		bSDLGamepadSubsystemInitialized = false;
		UE_LOG(LogBertaDualSense, Log, TEXT("Shut down SDL gamepad subsystem."));
	}

	if (SDL3DllHandle)
	{
		FPlatformProcess::FreeDllHandle(SDL3DllHandle);
		SDL3DllHandle = nullptr;
	}
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
