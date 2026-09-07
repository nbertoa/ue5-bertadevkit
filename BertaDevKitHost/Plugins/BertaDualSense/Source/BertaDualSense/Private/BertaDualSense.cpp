#include "BertaDualSense.h"

#include "Features/IModularFeatures.h"
#include "GenericPlatform/GenericInputDeviceMap.h"
#include "GenericPlatform/IInputInterface.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "GenericPlatform/InputDeviceRegistry.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Interfaces/IPluginManager.h"
#include "InputCoreTypes.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Paths.h"
#include "SDL3/SDL.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaDualSense, Log, All);

namespace
{
	constexpr Uint16 SonyVendorId = 0x054C;
	constexpr Uint16 DualSenseProductId = 0x0CE6;
	constexpr Uint16 DualSenseEdgeProductId = 0x0DF2;
	constexpr int32 NumGamepadButtons = 24;
	constexpr int32 LeftStickDeadZone = 7849;
	constexpr int32 RightStickDeadZone = 8689;
	constexpr float TriggerThreshold = 30.0f / 255.0f;
	constexpr Uint32 RumbleDurationMilliseconds = 1000;
	constexpr int32 NumEdgeButtons = 4;
	constexpr int32 TriggerEffectSize = 11;
	constexpr Uint8 EnableRightTrigger = 0x04;
	constexpr Uint8 EnableLeftTrigger = 0x08;
	constexpr Uint8 EnableMicLight = 0x01;
	const FName EdgeButtonKeys[NumEdgeButtons] = { FName(TEXT("BertaDualSense_Edge_RightPaddle")), FName(TEXT("BertaDualSense_Edge_LeftPaddle")), FName(TEXT("BertaDualSense_Edge_RightFn")), FName(TEXT("BertaDualSense_Edge_LeftFn")) };
	struct FPS5EffectsState { Uint8 EnableBits1=0, EnableBits2=0, RumbleRight=0, RumbleLeft=0, HeadphoneVolume=0, SpeakerVolume=0, MicrophoneVolume=0, AudioEnableBits=0, MicLightMode=0, AudioMuteBits=0; Uint8 RightTrigger[TriggerEffectSize]={}; Uint8 LeftTrigger[TriggerEffectSize]={}; Uint8 Reserved1[6]={}; Uint8 EnableBits3=0; Uint8 Reserved2[2]={}; Uint8 LedAnimation=0, LedBrightness=0, PadLights=0, LedRed=0, LedGreen=0, LedBlue=0; };
	static_assert(sizeof(FPS5EffectsState) == 47);

	const FGamepadKeyNames::Type GamepadButtonKeys[NumGamepadButtons] =
	{
		FGamepadKeyNames::FaceButtonBottom,
		FGamepadKeyNames::FaceButtonRight,
		FGamepadKeyNames::FaceButtonLeft,
		FGamepadKeyNames::FaceButtonTop,
		FGamepadKeyNames::LeftShoulder,
		FGamepadKeyNames::RightShoulder,
		FGamepadKeyNames::SpecialRight,
		FGamepadKeyNames::SpecialLeft,
		FGamepadKeyNames::LeftThumb,
		FGamepadKeyNames::RightThumb,
		FGamepadKeyNames::LeftTriggerThreshold,
		FGamepadKeyNames::RightTriggerThreshold,
		FGamepadKeyNames::DPadUp,
		FGamepadKeyNames::DPadDown,
		FGamepadKeyNames::DPadLeft,
		FGamepadKeyNames::DPadRight,
		FGamepadKeyNames::LeftStickUp,
		FGamepadKeyNames::LeftStickDown,
		FGamepadKeyNames::LeftStickLeft,
		FGamepadKeyNames::LeftStickRight,
		FGamepadKeyNames::RightStickUp,
		FGamepadKeyNames::RightStickDown,
		FGamepadKeyNames::RightStickLeft,
		FGamepadKeyNames::RightStickRight,
	};

	float NormalizeSignedAxis(const Sint16 AxisValue)
	{
		return static_cast<float>(AxisValue) / (AxisValue <= 0 ? 32768.0f : 32767.0f);
	}

	float NormalizeTriggerAxis(const Sint16 AxisValue)
	{
		return static_cast<float>(AxisValue) / 32767.0f;
	}

	Uint16 ToRumbleMagnitude(const float Value)
	{
		return static_cast<Uint16>(FMath::RoundToInt(FMath::Clamp(Value, 0.0f, 1.0f) * 65535.0f));
	}

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
		bool ButtonStates[NumGamepadButtons] = { false };
		double NextRepeatTime[NumGamepadButtons] = { 0.0 };
		Sint16 LeftXAnalog = 0;
		Sint16 LeftYAnalog = 0;
		Sint16 RightXAnalog = 0;
		Sint16 RightYAnalog = 0;
		Sint16 LeftTriggerAnalog = 0;
		Sint16 RightTriggerAnalog = 0;
		FForceFeedbackValues ForceFeedback;
		bool bSupportsRumble = false;
		bool bRumbleFailureLogged = false;
		bool bHasTouchpad = false;
		int32 TouchpadFingerCount = 0;
		int32 PrimaryTouchpadFinger = INDEX_NONE;
		bool bTouchpadTouched = false;
		float TouchpadX = 0.0f;
		float TouchpadY = 0.0f;
		bool bTouchpadQueryFailureLogged = false;
		Uint16 ProductId = 0;
		bool EdgeButtonStates[NumEdgeButtons] = {};
		double EdgeNextRepeatTime[NumEdgeButtons] = {};
		bool bSupportsRgbLed = false;
		bool bSupportsPlayerLed = false;
		bool bAccelerometerEnabled = false;
		bool bGyroscopeEnabled = false;
		bool bSensorFailureLogged = false;
		bool bAdvancedOutputFailureLogged = false;
		bool bMicrophoneLedOn = false;
		Uint8 LeftTriggerEffect[TriggerEffectSize] = {};
		Uint8 RightTriggerEffect[TriggerEffectSize] = {};
		bool bLeftTriggerEffectActive = false;
		bool bRightTriggerEffectActive = false;
	};
}

class FBertaDualSenseInputDevice final : public IInputDevice
{
	public:
		explicit FBertaDualSenseInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
			: MessageHandler(InMessageHandler)
		{
			GConfig->GetFloat(TEXT("/Script/Engine.InputSettings"), TEXT("InitialButtonRepeatDelay"), InitialButtonRepeatDelay, GInputIni);
			GConfig->GetFloat(TEXT("/Script/Engine.InputSettings"), TEXT("ButtonRepeatDelay"), ButtonRepeatDelay, GInputIni);
		}

		virtual ~FBertaDualSenseInputDevice() override
		{
			Shutdown();
		}

		void Shutdown()
		{
			if (bShutdown)
			{
				return;
			}

			bShutdown = true;
			DisconnectAllDevices();
			OpenFailures.Empty();
			UnavailableIdentityLogged.Empty();
			bGamepadEnumerationFailureLogged = false;
		}

		virtual void Tick(float DeltaTime) override
		{
			if (bShutdown)
			{
				return;
			}

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
		virtual void SendControllerEvents() override
		{
			if (bShutdown)
			{
				return;
			}

			for (TPair<SDL_JoystickID, FConnectedDualSense>& Pair : ConnectedDevices)
			{
				SendControllerEvents(Pair.Value);
			}
		}

		virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override
		{
			MessageHandler = InMessageHandler;
		}

		virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override { return false; }
		virtual bool IsGamepadAttached() const override { return !ConnectedDevices.IsEmpty(); }
		virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override
		{
			const FPlatformUserId PlatformUserId = GetPlatformUserForControllerId(ControllerId);
			if (!PlatformUserId.IsValid())
			{
				return;
			}

			for (TPair<SDL_JoystickID, FConnectedDualSense>& Pair : ConnectedDevices)
			{
				FConnectedDualSense& ConnectedDevice = Pair.Value;
				if (ConnectedDevice.PlatformUserId == PlatformUserId && ConnectedDevice.bSupportsRumble)
				{
					SetForceFeedbackChannel(ConnectedDevice.ForceFeedback, ChannelType, Value);
					ApplyRumble(ConnectedDevice);
				}
			}
		}

		virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) override
		{
			const FPlatformUserId PlatformUserId = GetPlatformUserForControllerId(ControllerId);
			if (!PlatformUserId.IsValid())
			{
				return;
			}

			for (TPair<SDL_JoystickID, FConnectedDualSense>& Pair : ConnectedDevices)
			{
				FConnectedDualSense& ConnectedDevice = Pair.Value;
				if (ConnectedDevice.PlatformUserId == PlatformUserId && ConnectedDevice.bSupportsRumble)
				{
					ConnectedDevice.ForceFeedback.LeftLarge = FMath::Clamp(Values.LeftLarge, 0.0f, 1.0f);
					ConnectedDevice.ForceFeedback.LeftSmall = FMath::Clamp(Values.LeftSmall, 0.0f, 1.0f);
					ConnectedDevice.ForceFeedback.RightLarge = FMath::Clamp(Values.RightLarge, 0.0f, 1.0f);
					ConnectedDevice.ForceFeedback.RightSmall = FMath::Clamp(Values.RightSmall, 0.0f, 1.0f);
					ApplyRumble(ConnectedDevice);
				}
			}
		}

		virtual void SetLightColor(int32 ControllerId, FColor Color) override { ForEachControllerDevice(ControllerId, [&Color](FConnectedDualSense& Device){ SetLight(Device, Color); }); }
		virtual void ResetLightColor(int32 ControllerId) override { ForEachControllerDevice(ControllerId, [](FConnectedDualSense& Device){ SetLight(Device, FColor::Black); }); }
		virtual void SetDeviceProperty(int32 ControllerId, const FInputDeviceProperty* Property) override { if (Property) ForEachControllerDevice(ControllerId, [Property](FConnectedDualSense& Device){ SetTriggerProperty(Device, *Property); }); }
		void SetMicrophoneLed(int32 ControllerId, bool bOn) { ForEachControllerDevice(ControllerId, [bOn](FConnectedDualSense& Device){ Device.bMicrophoneLedOn=bOn; SendPs5Effects(Device,false,false,true); }); }
		virtual bool SupportsForceFeedback(int32 ControllerId) override
		{
			const FPlatformUserId PlatformUserId = GetPlatformUserForControllerId(ControllerId);
			if (!PlatformUserId.IsValid())
			{
				return false;
			}

			for (const TPair<SDL_JoystickID, FConnectedDualSense>& Pair : ConnectedDevices)
			{
				if (Pair.Value.PlatformUserId == PlatformUserId && Pair.Value.bSupportsRumble)
				{
					return true;
				}
			}

			return false;
		}

	private:
		FPlatformUserId GetPlatformUserForControllerId(const int32 ControllerId) const
		{
			IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
			FPlatformUserId PlatformUserId = PLATFORMUSERID_NONE;
			FInputDeviceId InputDeviceId = INPUTDEVICEID_NONE;
			return DeviceMapper.RemapControllerIdToPlatformUserAndDevice(ControllerId, PlatformUserId, InputDeviceId) ? PlatformUserId : PLATFORMUSERID_NONE;
		}

		static void SetForceFeedbackChannel(FForceFeedbackValues& ForceFeedback, const FForceFeedbackChannelType ChannelType, const float Value)
		{
			const float ClampedValue = FMath::Clamp(Value, 0.0f, 1.0f);
			switch (ChannelType)
			{
			case FForceFeedbackChannelType::LEFT_LARGE:
				ForceFeedback.LeftLarge = ClampedValue;
				break;
			case FForceFeedbackChannelType::LEFT_SMALL:
				ForceFeedback.LeftSmall = ClampedValue;
				break;
			case FForceFeedbackChannelType::RIGHT_LARGE:
				ForceFeedback.RightLarge = ClampedValue;
				break;
			case FForceFeedbackChannelType::RIGHT_SMALL:
				ForceFeedback.RightSmall = ClampedValue;
				break;
			}
		}

		void ApplyRumble(FConnectedDualSense& ConnectedDevice)
		{
			const Uint16 LowFrequencyMagnitude = ToRumbleMagnitude(FMath::Max(ConnectedDevice.ForceFeedback.LeftLarge, ConnectedDevice.ForceFeedback.RightLarge));
			const Uint16 HighFrequencyMagnitude = ToRumbleMagnitude(FMath::Max(ConnectedDevice.ForceFeedback.LeftSmall, ConnectedDevice.ForceFeedback.RightSmall));
			if (!SDL_RumbleGamepad(ConnectedDevice.Gamepad, LowFrequencyMagnitude, HighFrequencyMagnitude, RumbleDurationMilliseconds))
			{
				if (!ConnectedDevice.bRumbleFailureLogged)
				{
					ConnectedDevice.bRumbleFailureLogged = true;
					UE_LOG(LogBertaDualSense, Warning, TEXT("SDL_RumbleGamepad failed for InputDeviceId %d: %s"), ConnectedDevice.InputDeviceId.GetId(), UTF8_TO_TCHAR(SDL_GetError()));
				}
			}
			else
			{
				ConnectedDevice.bRumbleFailureLogged = false;
			}
		}
		template<typename TFunction> void ForEachControllerDevice(int32 ControllerId, TFunction&& Function) { const FPlatformUserId User=GetPlatformUserForControllerId(ControllerId); if(!User.IsValid())return; for(TPair<SDL_JoystickID,FConnectedDualSense>& Pair:ConnectedDevices)if(Pair.Value.PlatformUserId==User)Function(Pair.Value); }
		static void SetLight(FConnectedDualSense& Device,FColor Color) { if(Device.bSupportsRgbLed&&!SDL_SetGamepadLED(Device.Gamepad,Color.R,Color.G,Color.B)) UE_LOG(LogBertaDualSense,Warning,TEXT("SDL_SetGamepadLED failed for InputDeviceId %d: %s"),Device.InputDeviceId.GetId(),UTF8_TO_TCHAR(SDL_GetError())); }
		static void ClearTrigger(Uint8 (&E)[TriggerEffectSize]) { FMemory::Memzero(E);E[0]=0x05; }
		static void FeedbackTrigger(Uint8 (&E)[TriggerEffectSize],int32 Position,int32 Strength) { FMemory::Memzero(E);E[0]=0x01;E[1]=static_cast<Uint8>(FMath::Clamp(Position,0,9)*255/9);E[2]=static_cast<Uint8>(FMath::Clamp(Strength,0,8)*255/8); }
		static void VibrationTrigger(Uint8 (&E)[TriggerEffectSize],int32 Position,int32 Frequency,int32 Strength) { FMemory::Memzero(E);E[0]=0x06;E[1]=static_cast<Uint8>(FMath::Clamp(Position,0,9)*255/9);E[2]=static_cast<Uint8>(FMath::Clamp(Strength,0,8)*255/8);E[3]=static_cast<Uint8>(FMath::Clamp(Frequency,0,255)); }
		static void SendPs5Effects(FConnectedDualSense& D,bool L,bool R,bool M) { FPS5EffectsState E; if(R){E.EnableBits1|=EnableRightTrigger;FMemory::Memcpy(E.RightTrigger,D.RightTriggerEffect,TriggerEffectSize);}if(L){E.EnableBits1|=EnableLeftTrigger;FMemory::Memcpy(E.LeftTrigger,D.LeftTriggerEffect,TriggerEffectSize);}if(M){E.EnableBits2|=EnableMicLight;E.MicLightMode=D.bMicrophoneLedOn?1:0;}if(!SDL_SendGamepadEffect(D.Gamepad,&E,sizeof(E))&&!D.bAdvancedOutputFailureLogged){D.bAdvancedOutputFailureLogged=true;UE_LOG(LogBertaDualSense,Warning,TEXT("SDL_SendGamepadEffect failed for InputDeviceId %d: %s"),D.InputDeviceId.GetId(),UTF8_TO_TCHAR(SDL_GetError()));}}
		static void SetTriggerProperty(FConnectedDualSense& D,const FInputDeviceProperty& P) { bool L=false,R=false; if(P.Name==FInputDeviceTriggerResetProperty::PropertyName()){const auto& V=static_cast<const FInputDeviceTriggerResetProperty&>(P);L=EnumHasAnyFlags(V.AffectedTriggers,EInputDeviceTriggerMask::Left);R=EnumHasAnyFlags(V.AffectedTriggers,EInputDeviceTriggerMask::Right);if(L){ClearTrigger(D.LeftTriggerEffect);D.bLeftTriggerEffectActive=false;}if(R){ClearTrigger(D.RightTriggerEffect);D.bRightTriggerEffectActive=false;}}else if(P.Name==FInputDeviceTriggerFeedbackProperty::PropertyName()){const auto& V=static_cast<const FInputDeviceTriggerFeedbackProperty&>(P);L=EnumHasAnyFlags(V.AffectedTriggers,EInputDeviceTriggerMask::Left);R=EnumHasAnyFlags(V.AffectedTriggers,EInputDeviceTriggerMask::Right);if(L){FeedbackTrigger(D.LeftTriggerEffect,V.Position,V.Strengh);D.bLeftTriggerEffectActive=true;}if(R){FeedbackTrigger(D.RightTriggerEffect,V.Position,V.Strengh);D.bRightTriggerEffectActive=true;}}else if(P.Name==FInputDeviceTriggerVibrationProperty::PropertyName()){const auto& V=static_cast<const FInputDeviceTriggerVibrationProperty&>(P);L=EnumHasAnyFlags(V.AffectedTriggers,EInputDeviceTriggerMask::Left);R=EnumHasAnyFlags(V.AffectedTriggers,EInputDeviceTriggerMask::Right);if(L){VibrationTrigger(D.LeftTriggerEffect,V.TriggerPosition,V.VibrationFrequency,V.VibrationAmplitude);D.bLeftTriggerEffectActive=true;}if(R){VibrationTrigger(D.RightTriggerEffect,V.TriggerPosition,V.VibrationFrequency,V.VibrationAmplitude);D.bRightTriggerEffectActive=true;}}if(L||R)SendPs5Effects(D,L,R,false); }
		void SendControllerEvents(FConnectedDualSense& ConnectedDevice)
		{
			bool CurrentButtonStates[NumGamepadButtons] = { false };
			CurrentButtonStates[0] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
			CurrentButtonStates[1] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_EAST);
			CurrentButtonStates[2] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_WEST);
			CurrentButtonStates[3] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_NORTH);
			CurrentButtonStates[4] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
			CurrentButtonStates[5] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
			CurrentButtonStates[6] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_START);
			CurrentButtonStates[7] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_BACK);
			CurrentButtonStates[8] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK);
			CurrentButtonStates[9] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
			CurrentButtonStates[12] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
			CurrentButtonStates[13] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
			CurrentButtonStates[14] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
			CurrentButtonStates[15] = SDL_GetGamepadButton(ConnectedDevice.Gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);

			const Sint16 LeftX = SDL_GetGamepadAxis(ConnectedDevice.Gamepad, SDL_GAMEPAD_AXIS_LEFTX);
			const Sint16 LeftY = SDL_GetGamepadAxis(ConnectedDevice.Gamepad, SDL_GAMEPAD_AXIS_LEFTY);
			const Sint16 RightX = SDL_GetGamepadAxis(ConnectedDevice.Gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
			const Sint16 RightY = SDL_GetGamepadAxis(ConnectedDevice.Gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
			const Sint16 LeftTrigger = SDL_GetGamepadAxis(ConnectedDevice.Gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
			const Sint16 RightTrigger = SDL_GetGamepadAxis(ConnectedDevice.Gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

			CurrentButtonStates[10] = NormalizeTriggerAxis(LeftTrigger) > TriggerThreshold;
			CurrentButtonStates[11] = NormalizeTriggerAxis(RightTrigger) > TriggerThreshold;
			CurrentButtonStates[16] = LeftY < -LeftStickDeadZone;
			CurrentButtonStates[17] = LeftY > LeftStickDeadZone;
			CurrentButtonStates[18] = LeftX < -LeftStickDeadZone;
			CurrentButtonStates[19] = LeftX > LeftStickDeadZone;
			CurrentButtonStates[20] = RightY < -RightStickDeadZone;
			CurrentButtonStates[21] = RightY > RightStickDeadZone;
			CurrentButtonStates[22] = RightX < -RightStickDeadZone;
			CurrentButtonStates[23] = RightX > RightStickDeadZone;

			SendAnalog(ConnectedDevice, FGamepadKeyNames::LeftAnalogX, LeftX, NormalizeSignedAxis(LeftX), ConnectedDevice.LeftXAnalog, LeftStickDeadZone);
			SendAnalog(ConnectedDevice, FGamepadKeyNames::LeftAnalogY, LeftY, -NormalizeSignedAxis(LeftY), ConnectedDevice.LeftYAnalog, LeftStickDeadZone);
			SendAnalog(ConnectedDevice, FGamepadKeyNames::RightAnalogX, RightX, NormalizeSignedAxis(RightX), ConnectedDevice.RightXAnalog, RightStickDeadZone);
			SendAnalog(ConnectedDevice, FGamepadKeyNames::RightAnalogY, RightY, -NormalizeSignedAxis(RightY), ConnectedDevice.RightYAnalog, RightStickDeadZone);
			SendAnalog(ConnectedDevice, FGamepadKeyNames::LeftTriggerAnalog, LeftTrigger, NormalizeTriggerAxis(LeftTrigger), ConnectedDevice.LeftTriggerAnalog, TriggerThreshold * 32767.0f);
			SendAnalog(ConnectedDevice, FGamepadKeyNames::RightTriggerAnalog, RightTrigger, NormalizeTriggerAxis(RightTrigger), ConnectedDevice.RightTriggerAnalog, TriggerThreshold * 32767.0f);
			SendTouchpadEvents(ConnectedDevice);
			SendSensors(ConnectedDevice);
			SendEdgeButtons(ConnectedDevice);

			const double CurrentTime = FPlatformTime::Seconds();
			for (int32 ButtonIndex = 0; ButtonIndex < NumGamepadButtons; ++ButtonIndex)
			{
				if (CurrentButtonStates[ButtonIndex] != ConnectedDevice.ButtonStates[ButtonIndex])
				{
					if (CurrentButtonStates[ButtonIndex])
					{
						MessageHandler->OnControllerButtonPressed(GamepadButtonKeys[ButtonIndex], ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, false);
						ConnectedDevice.NextRepeatTime[ButtonIndex] = CurrentTime + InitialButtonRepeatDelay;
					}
					else
					{
						MessageHandler->OnControllerButtonReleased(GamepadButtonKeys[ButtonIndex], ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, false);
					}
				}
				else if (CurrentButtonStates[ButtonIndex] && ConnectedDevice.NextRepeatTime[ButtonIndex] <= CurrentTime)
				{
					MessageHandler->OnControllerButtonPressed(GamepadButtonKeys[ButtonIndex], ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, true);
					ConnectedDevice.NextRepeatTime[ButtonIndex] = CurrentTime + ButtonRepeatDelay;
				}

				ConnectedDevice.ButtonStates[ButtonIndex] = CurrentButtonStates[ButtonIndex];
			}
		}

		void SendSensors(FConnectedDualSense& D) { if(!D.bAccelerometerEnabled&&!D.bGyroscopeEnabled)return;float A[3]={},G[3]={};const bool AO=!D.bAccelerometerEnabled||SDL_GetGamepadSensorData(D.Gamepad,SDL_SENSOR_ACCEL,A,3);const bool GO=!D.bGyroscopeEnabled||SDL_GetGamepadSensorData(D.Gamepad,SDL_SENSOR_GYRO,G,3);if(!AO||!GO){if(!D.bSensorFailureLogged){D.bSensorFailureLogged=true;UE_LOG(LogBertaDualSense,Warning,TEXT("SDL sensor poll failed for InputDeviceId %d: %s"),D.InputDeviceId.GetId(),UTF8_TO_TCHAR(SDL_GetError()));}return;}D.bSensorFailureLogged=false;MessageHandler->OnMotionDetected(FVector::ZeroVector,D.bGyroscopeEnabled?FVector(G[0],G[1],G[2]):FVector::ZeroVector,D.bAccelerometerEnabled?FVector(A[0],A[1],A[2])/9.80665f:FVector::ZeroVector,FVector::ZeroVector,D.PlatformUserId,D.InputDeviceId);}
		void SendEdgeButtons(FConnectedDualSense& D) { if(D.ProductId!=DualSenseEdgeProductId)return;const SDL_GamepadButton B[NumEdgeButtons]={SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,SDL_GAMEPAD_BUTTON_LEFT_PADDLE2};const double T=FPlatformTime::Seconds();for(int32 I=0;I<NumEdgeButtons;++I){const bool Down=SDL_GetGamepadButton(D.Gamepad,B[I]);if(Down!=D.EdgeButtonStates[I]){if(Down){MessageHandler->OnControllerButtonPressed(EdgeButtonKeys[I],D.PlatformUserId,D.InputDeviceId,false);D.EdgeNextRepeatTime[I]=T+InitialButtonRepeatDelay;}else MessageHandler->OnControllerButtonReleased(EdgeButtonKeys[I],D.PlatformUserId,D.InputDeviceId,false);}else if(Down&&D.EdgeNextRepeatTime[I]<=T){MessageHandler->OnControllerButtonPressed(EdgeButtonKeys[I],D.PlatformUserId,D.InputDeviceId,true);D.EdgeNextRepeatTime[I]=T+ButtonRepeatDelay;}D.EdgeButtonStates[I]=Down;}}
		void SendAnalog(const FConnectedDualSense& ConnectedDevice, const FGamepadKeyNames::Type Key, const Sint16 NewRawValue, const float NewNormalizedValue, Sint16& PreviousRawValue, const float HeldThreshold)
		{
			if (PreviousRawValue != NewRawValue || FMath::Abs(static_cast<int32>(NewRawValue)) > HeldThreshold)
			{
				MessageHandler->OnControllerAnalog(Key, ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, NewNormalizedValue);
			}
			PreviousRawValue = NewRawValue;
		}

		void SendTouchpadEvents(FConnectedDualSense& ConnectedDevice)
		{
			if (!ConnectedDevice.bHasTouchpad)
			{
				return;
			}

			struct FTouchpadFingerState { bool bDown; float X; float Y; };
			TArray<FTouchpadFingerState, TInlineAllocator<2>> Fingers;
			Fingers.SetNum(ConnectedDevice.TouchpadFingerCount);
			for (int32 FingerIndex = 0; FingerIndex < ConnectedDevice.TouchpadFingerCount; ++FingerIndex)
			{
				float Pressure = 0.0f;
				if (!SDL_GetGamepadTouchpadFinger(ConnectedDevice.Gamepad, 0, FingerIndex, &Fingers[FingerIndex].bDown, &Fingers[FingerIndex].X, &Fingers[FingerIndex].Y, &Pressure))
				{
					if (!ConnectedDevice.bTouchpadQueryFailureLogged)
					{
						ConnectedDevice.bTouchpadQueryFailureLogged = true;
						UE_LOG(LogBertaDualSense, Warning, TEXT("SDL_GetGamepadTouchpadFinger failed for InputDeviceId %d: %s"), ConnectedDevice.InputDeviceId.GetId(), UTF8_TO_TCHAR(SDL_GetError()));
					}
					return;
				}
			}
			ConnectedDevice.bTouchpadQueryFailureLogged = false;

			if (ConnectedDevice.PrimaryTouchpadFinger == INDEX_NONE || !Fingers[ConnectedDevice.PrimaryTouchpadFinger].bDown)
			{
				ConnectedDevice.PrimaryTouchpadFinger = INDEX_NONE;
				for (int32 FingerIndex = 0; FingerIndex < Fingers.Num(); ++FingerIndex)
				{
					if (Fingers[FingerIndex].bDown)
					{
						ConnectedDevice.PrimaryTouchpadFinger = FingerIndex;
						break;
					}
				}
			}

			const bool bTouched = ConnectedDevice.PrimaryTouchpadFinger != INDEX_NONE;
			if (bTouched)
			{
				const FTouchpadFingerState& PrimaryFinger = Fingers[ConnectedDevice.PrimaryTouchpadFinger];
				if (!ConnectedDevice.bTouchpadTouched)
				{
					MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::SpecialLeft_Touched, ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, false);
				}
				if (!ConnectedDevice.bTouchpadTouched || PrimaryFinger.X != ConnectedDevice.TouchpadX)
				{
					MessageHandler->OnControllerAnalog(FGamepadKeyNames::SpecialLeft_X, ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, PrimaryFinger.X);
				}
				if (!ConnectedDevice.bTouchpadTouched || PrimaryFinger.Y != ConnectedDevice.TouchpadY)
				{
					MessageHandler->OnControllerAnalog(FGamepadKeyNames::SpecialLeft_Y, ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, PrimaryFinger.Y);
				}
				ConnectedDevice.TouchpadX = PrimaryFinger.X;
				ConnectedDevice.TouchpadY = PrimaryFinger.Y;
			}
			else if (ConnectedDevice.bTouchpadTouched)
			{
				MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::SpecialLeft_Touched, ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, false);
				MessageHandler->OnControllerAnalog(FGamepadKeyNames::SpecialLeft_X, ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, 0.0f);
				MessageHandler->OnControllerAnalog(FGamepadKeyNames::SpecialLeft_Y, ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, 0.0f);
				ConnectedDevice.TouchpadX = 0.0f;
				ConnectedDevice.TouchpadY = 0.0f;
			}
			ConnectedDevice.bTouchpadTouched = bTouched;
		}
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
			ConnectedDevice.ProductId = ProductId;
			ConnectedDevice.PlatformUserId = PlatformUserId;
			ConnectedDevice.bAccelerometerEnabled = SDL_GamepadHasSensor(Gamepad, SDL_SENSOR_ACCEL) && SDL_SetGamepadSensorEnabled(Gamepad, SDL_SENSOR_ACCEL, true);
			ConnectedDevice.bGyroscopeEnabled = SDL_GamepadHasSensor(Gamepad, SDL_SENSOR_GYRO) && SDL_SetGamepadSensorEnabled(Gamepad, SDL_SENSOR_GYRO, true);

			const SDL_PropertiesID GamepadProperties = SDL_GetGamepadProperties(Gamepad);
			ConnectedDevice.bSupportsRumble = SDL_GetBooleanProperty(GamepadProperties, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false);
			ConnectedDevice.bSupportsRgbLed = SDL_GetBooleanProperty(GamepadProperties, SDL_PROP_GAMEPAD_CAP_RGB_LED_BOOLEAN, false);
			ConnectedDevice.bSupportsPlayerLed = SDL_GetBooleanProperty(GamepadProperties, SDL_PROP_GAMEPAD_CAP_PLAYER_LED_BOOLEAN, false);
			if (ConnectedDevice.bSupportsPlayerLed && !SDL_SetGamepadPlayerIndex(Gamepad, DeviceMapper.GetUserIndexForPlatformUser(PlatformUserId)))
			{
				UE_LOG(LogBertaDualSense, Warning, TEXT("SDL_SetGamepadPlayerIndex failed for InputDeviceId %d: %s"), InputDeviceId.GetId(), UTF8_TO_TCHAR(SDL_GetError()));
			}
			ConnectedDevice.bHasTouchpad = SDL_GetNumGamepadTouchpads(Gamepad) > 0;
			if (ConnectedDevice.bHasTouchpad)
			{
				ConnectedDevice.TouchpadFingerCount = SDL_GetNumGamepadTouchpadFingers(Gamepad, 0);
				ConnectedDevice.bHasTouchpad = ConnectedDevice.TouchpadFingerCount > 0;
			}
			UE_LOG(LogBertaDualSense, Log, TEXT("Connected %s (SDL instance %u, VID=%04X PID=%04X, Serial='%s', Path='%s') as InputDeviceId %d for PlatformUserId %d; SDL_OpenGamepad took %.3f ms."), *ToUnrealString(SDL_GetGamepadName(Gamepad)), InstanceId, VendorId, ProductId, *Serial, *ToUnrealString(SDL_GetGamepadPath(Gamepad)), InputDeviceId.GetId(), PlatformUserId.GetInternalId(), OpenDurationMilliseconds);
		}

		void DisconnectDevice(const SDL_JoystickID InstanceId, FConnectedDualSense& ConnectedDevice)
		{
			FlushInputState(ConnectedDevice);
			FlushTouchpadState(ConnectedDevice);
			FlushEdgeButtons(ConnectedDevice);
			ClearAdvancedEffects(ConnectedDevice);
			StopRumble(ConnectedDevice);
			if (ConnectedDevice.bAccelerometerEnabled) SDL_SetGamepadSensorEnabled(ConnectedDevice.Gamepad, SDL_SENSOR_ACCEL, false);
			if (ConnectedDevice.bGyroscopeEnabled) SDL_SetGamepadSensorEnabled(ConnectedDevice.Gamepad, SDL_SENSOR_GYRO, false);
			if (ConnectedDevice.bSupportsPlayerLed) SDL_SetGamepadPlayerIndex(ConnectedDevice.Gamepad, -1);

			IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
			DeviceMapper.Internal_MapInputDeviceToUser(ConnectedDevice.InputDeviceId, DeviceMapper.GetUserForUnpairedInputDevices(), EInputDeviceConnectionState::Disconnected);
			SDL_CloseGamepad(ConnectedDevice.Gamepad);
			UE_LOG(LogBertaDualSense, Log, TEXT("Disconnected DualSense SDL instance %u from InputDeviceId %d."), InstanceId, ConnectedDevice.InputDeviceId.GetId());
		}

		void FlushEdgeButtons(FConnectedDualSense& D) { for(int32 I=0;I<NumEdgeButtons;++I){if(D.EdgeButtonStates[I])MessageHandler->OnControllerButtonReleased(EdgeButtonKeys[I],D.PlatformUserId,D.InputDeviceId,false);D.EdgeButtonStates[I]=false;D.EdgeNextRepeatTime[I]=0.0;} }
		void ClearAdvancedEffects(FConnectedDualSense& D) { const bool L=D.bLeftTriggerEffectActive,R=D.bRightTriggerEffectActive;if(L){ClearTrigger(D.LeftTriggerEffect);D.bLeftTriggerEffectActive=false;}if(R){ClearTrigger(D.RightTriggerEffect);D.bRightTriggerEffectActive=false;}if(L||R||D.bMicrophoneLedOn){D.bMicrophoneLedOn=false;SendPs5Effects(D,L,R,true);} }
		void FlushTouchpadState(FConnectedDualSense& ConnectedDevice)
		{
			if (ConnectedDevice.bTouchpadTouched)
			{
				MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::SpecialLeft_Touched, ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, false);
				MessageHandler->OnControllerAnalog(FGamepadKeyNames::SpecialLeft_X, ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, 0.0f);
				MessageHandler->OnControllerAnalog(FGamepadKeyNames::SpecialLeft_Y, ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, 0.0f);
			}
			ConnectedDevice.PrimaryTouchpadFinger = INDEX_NONE;
			ConnectedDevice.bTouchpadTouched = false;
			ConnectedDevice.TouchpadX = 0.0f;
			ConnectedDevice.TouchpadY = 0.0f;
			ConnectedDevice.bTouchpadQueryFailureLogged = false;
		}

		void StopRumble(FConnectedDualSense& ConnectedDevice)
		{
			const bool bRumbleWasActive = ConnectedDevice.ForceFeedback.LeftLarge != 0.0f || ConnectedDevice.ForceFeedback.LeftSmall != 0.0f || ConnectedDevice.ForceFeedback.RightLarge != 0.0f || ConnectedDevice.ForceFeedback.RightSmall != 0.0f;
			ConnectedDevice.ForceFeedback = FForceFeedbackValues();
			if (ConnectedDevice.bSupportsRumble && bRumbleWasActive)
			{
				ApplyRumble(ConnectedDevice);
			}
			ConnectedDevice.bRumbleFailureLogged = false;
		}

		void FlushInputState(FConnectedDualSense& ConnectedDevice)
		{
			for (int32 ButtonIndex = 0; ButtonIndex < NumGamepadButtons; ++ButtonIndex)
			{
				if (ConnectedDevice.ButtonStates[ButtonIndex])
				{
					MessageHandler->OnControllerButtonReleased(GamepadButtonKeys[ButtonIndex], ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, false);
				}
				ConnectedDevice.ButtonStates[ButtonIndex] = false;
				ConnectedDevice.NextRepeatTime[ButtonIndex] = 0.0;
			}

			FlushAnalog(FGamepadKeyNames::LeftAnalogX, ConnectedDevice, ConnectedDevice.LeftXAnalog);
			FlushAnalog(FGamepadKeyNames::LeftAnalogY, ConnectedDevice, ConnectedDevice.LeftYAnalog);
			FlushAnalog(FGamepadKeyNames::RightAnalogX, ConnectedDevice, ConnectedDevice.RightXAnalog);
			FlushAnalog(FGamepadKeyNames::RightAnalogY, ConnectedDevice, ConnectedDevice.RightYAnalog);
			FlushAnalog(FGamepadKeyNames::LeftTriggerAnalog, ConnectedDevice, ConnectedDevice.LeftTriggerAnalog);
			FlushAnalog(FGamepadKeyNames::RightTriggerAnalog, ConnectedDevice, ConnectedDevice.RightTriggerAnalog);
		}

		void FlushAnalog(const FGamepadKeyNames::Type Key, const FConnectedDualSense& ConnectedDevice, Sint16& PreviousRawValue)
		{
			if (PreviousRawValue != 0)
			{
				MessageHandler->OnControllerAnalog(Key, ConnectedDevice.PlatformUserId, ConnectedDevice.InputDeviceId, 0.0f);
			}
			PreviousRawValue = 0;
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
		float InitialButtonRepeatDelay = 0.2f;
		float ButtonRepeatDelay = 0.1f;
		bool bGamepadEnumerationFailureLogged = false;
		bool bShutdown = false;
};

void FBertaDualSenseModule::StartupModule()
{
	EKeys::AddKey(FKeyDetails(FName(TEXT("BertaDualSense_Edge_RightPaddle")),FText::FromString(TEXT("DualSense Edge Right Paddle")),FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(FName(TEXT("BertaDualSense_Edge_LeftPaddle")),FText::FromString(TEXT("DualSense Edge Left Paddle")),FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(FName(TEXT("BertaDualSense_Edge_RightFn")),FText::FromString(TEXT("DualSense Edge Right Fn")),FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(FName(TEXT("BertaDualSense_Edge_LeftFn")),FText::FromString(TEXT("DualSense Edge Left Fn")),FKeyDetails::GamepadKey));
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("BertaDualSense"));
	if (!Plugin)
	{
		UE_LOG(LogBertaDualSense, Error, TEXT("Failed to find the BertaDualSense plugin while resolving SDL3.dll."));
		return;
	}

	const FString SDL3DllPath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Binaries/ThirdParty/SDL3/Win64/SDL3.dll"));
	SDL3DllHandle = FPlatformProcess::GetDllHandle(*SDL3DllPath);
	if (!SDL3DllHandle)
	{
		UE_LOG(LogBertaDualSense, Error, TEXT("Failed to load BertaDualSense SDL3.dll from %s."), *SDL3DllPath);
		return;
	}

	const int LinkedSDLVersion = SDL_GetVersion();
	UE_LOG(LogBertaDualSense, Log, TEXT("Loaded SDL3 %d.%d.%d from %s."), SDL_VERSIONNUM_MAJOR(LinkedSDLVersion), SDL_VERSIONNUM_MINOR(LinkedSDLVersion), SDL_VERSIONNUM_MICRO(LinkedSDLVersion), *SDL3DllPath);

	bSDLGamepadSubsystemInitialized = SDL_InitSubSystem(SDL_INIT_GAMEPAD);
	if (!bSDLGamepadSubsystemInitialized)
	{
		UE_LOG(LogBertaDualSense, Error, TEXT("Failed to initialize SDL gamepad subsystem: %s"), UTF8_TO_TCHAR(SDL_GetError()));
		return;
	}

	SDL_SetGamepadEventsEnabled(false);

	IInputDeviceModule::StartupModule();
	bInputDeviceModularFeatureRegistered = true;
	EnginePreExitHandle = FCoreDelegates::OnEnginePreExit.AddRaw(this, &FBertaDualSenseModule::HandleEnginePreExit);

	UE_LOG(LogBertaDualSense, Log, TEXT("Initialized SDL gamepad subsystem with gamepad events disabled."));
}

void FBertaDualSenseModule::ShutdownModule()
{
	if (bInputDeviceModularFeatureRegistered)
	{
		IModularFeatures::Get().UnregisterModularFeature(GetModularFeatureName(), this);
		bInputDeviceModularFeatureRegistered = false;
	}

	if (EnginePreExitHandle.IsValid())
	{
		FCoreDelegates::OnEnginePreExit.Remove(EnginePreExitHandle);
		EnginePreExitHandle.Reset();
	}

	ShutdownInputDevices();
	CreatedInputDevices.Empty();

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

void FBertaDualSenseModule::HandleEnginePreExit()
{
	UE_LOG(LogBertaDualSense, Log, TEXT("Beginning BertaDualSense pre-exit input-device shutdown."));
	ShutdownInputDevices();
	UE_LOG(LogBertaDualSense, Log, TEXT("Completed BertaDualSense pre-exit input-device shutdown."));
}

void FBertaDualSenseModule::ShutdownInputDevices()
{
	for (const TWeakPtr<FBertaDualSenseInputDevice>& InputDevice : CreatedInputDevices)
	{
		if (const TSharedPtr<FBertaDualSenseInputDevice> PinnedInputDevice = InputDevice.Pin())
		{
			PinnedInputDevice->Shutdown();
		}
	}

	CreatedInputDevices.RemoveAll([](const TWeakPtr<FBertaDualSenseInputDevice>& InputDevice)
	{
		return !InputDevice.IsValid();
	});
}

TSharedPtr<IInputDevice> FBertaDualSenseModule::CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	if (!bSDLGamepadSubsystemInitialized)
	{
		return nullptr;
	}

	CreatedInputDevices.RemoveAll([](const TWeakPtr<FBertaDualSenseInputDevice>& InputDevice)
	{
		return !InputDevice.IsValid();
	});

	const TSharedRef<FBertaDualSenseInputDevice> InputDevice = MakeShared<FBertaDualSenseInputDevice>(InMessageHandler);
	CreatedInputDevices.Add(InputDevice);
	return InputDevice;
}

void FBertaDualSenseModule::SetMicrophoneLed(int32 ControllerId, bool bEnabled) { for(const TWeakPtr<FBertaDualSenseInputDevice>& Weak:CreatedInputDevices) if(const TSharedPtr<FBertaDualSenseInputDevice> Device=Weak.Pin()) Device->SetMicrophoneLed(ControllerId,bEnabled); }
IMPLEMENT_MODULE(FBertaDualSenseModule, BertaDualSense)
