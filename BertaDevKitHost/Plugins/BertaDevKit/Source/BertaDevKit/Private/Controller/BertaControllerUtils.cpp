// BertaControllerUtils.cpp
#include "Controller/BertaControllerUtils.h"

#include "GameFramework/Controller.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include "GameFramework/InputDeviceProperties.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GameFramework/PlayerController.h"

namespace
{
	APlayerController* ResolvePlayerController(AController* Controller)
	{
		return Cast<APlayerController>(Controller);
	}

	FDynamicForceFeedbackHandle StartDynamicVibration(APlayerController& PlayerController,
	                                                    const float Intensity,
	                                                    const float Duration,
	                                                    const bool bAffectsLeftLarge,
	                                                    const bool bAffectsLeftSmall,
	                                                    const bool bAffectsRightLarge,
	                                                    const bool bAffectsRightSmall)
	{
		const FDynamicForceFeedbackHandle NativeHandle = PlayerController.PlayDynamicForceFeedback(
			Intensity,
			Duration,
			bAffectsLeftLarge,
			bAffectsLeftSmall,
			bAffectsRightLarge,
			bAffectsRightSmall,
			EDynamicForceFeedbackAction::Start);

		return NativeHandle;
	}
}

FBertaControllerVibrationHandle UBertaControllerUtils::PlayControllerVibration(AController* Controller,
	                                                                                const float Intensity,
	                                                                                const float Duration,
	                                                                                const bool bAffectsLeftLarge,
	                                                                                const bool bAffectsLeftSmall,
	                                                                                const bool bAffectsRightLarge,
	                                                                                const bool bAffectsRightSmall)
{
	FBertaControllerVibrationHandle Handle;
	APlayerController* PlayerController = ResolvePlayerController(Controller);
	if (PlayerController == nullptr ||
		(!bAffectsLeftLarge && !bAffectsLeftSmall && !bAffectsRightLarge && !bAffectsRightSmall))
	{
		return Handle;
	}

	const FDynamicForceFeedbackHandle NativeHandle = StartDynamicVibration(*PlayerController,
	                                                                        FMath::Clamp(Intensity, 0.0f, 1.0f),
	                                                                        Duration,
	                                                                        bAffectsLeftLarge,
	                                                                        bAffectsLeftSmall,
	                                                                        bAffectsRightLarge,
	                                                                        bAffectsRightSmall);
	Handle.AddNativeHandle(NativeHandle);
	return Handle;
}

FBertaControllerVibrationHandle UBertaControllerUtils::PlayControllerVibrationChannels(AController* Controller,
	                                                                                        const float Duration,
	                                                                                        const float LeftLarge,
	                                                                                        const float LeftSmall,
	                                                                                        const float RightLarge,
	                                                                                        const float RightSmall)
{
	FBertaControllerVibrationHandle Handle;
	APlayerController* PlayerController = ResolvePlayerController(Controller);
	if (PlayerController == nullptr)
	{
		return Handle;
	}

	const float ClampedLeftLarge = FMath::Clamp(LeftLarge, 0.0f, 1.0f);
	const float ClampedLeftSmall = FMath::Clamp(LeftSmall, 0.0f, 1.0f);
	const float ClampedRightLarge = FMath::Clamp(RightLarge, 0.0f, 1.0f);
	const float ClampedRightSmall = FMath::Clamp(RightSmall, 0.0f, 1.0f);

	if (ClampedLeftLarge > 0.0f)
	{
		Handle.AddNativeHandle(StartDynamicVibration(*PlayerController, ClampedLeftLarge, Duration, true, false, false, false));
	}
	if (ClampedLeftSmall > 0.0f)
	{
		Handle.AddNativeHandle(StartDynamicVibration(*PlayerController, ClampedLeftSmall, Duration, false, true, false, false));
	}
	if (ClampedRightLarge > 0.0f)
	{
		Handle.AddNativeHandle(StartDynamicVibration(*PlayerController, ClampedRightLarge, Duration, false, false, true, false));
	}
	if (ClampedRightSmall > 0.0f)
	{
		Handle.AddNativeHandle(StartDynamicVibration(*PlayerController, ClampedRightSmall, Duration, false, false, false, true));
	}

	return Handle;
}

void UBertaControllerUtils::StopControllerVibration(AController* Controller, FBertaControllerVibrationHandle& Handle)
{
	if (APlayerController* PlayerController = ResolvePlayerController(Controller))
	{
		for (const uint64 NativeHandle : Handle.NativeHandles)
		{
			PlayerController->PlayDynamicForceFeedback(0.0f, 0.0f, false, false, false, false,
			                                           EDynamicForceFeedbackAction::Stop, NativeHandle);
		}
	}

	Handle.Reset();
}

bool UBertaControllerUtils::IsControllerVibrationHandleValid(const FBertaControllerVibrationHandle& Handle)
{
	return Handle.IsValid();
}

void UBertaControllerUtils::SetControllerLightColor(AController* Controller, const FColor Color)
{
	if (APlayerController* PlayerController = ResolvePlayerController(Controller))
	{
		PlayerController->SetControllerLightColor(Color);
	}
}

void UBertaControllerUtils::ResetControllerLightColor(AController* Controller)
{
	if (APlayerController* PlayerController = ResolvePlayerController(Controller))
	{
		PlayerController->ResetControllerLightColor();
	}
}

void UBertaControllerUtils::PlayControllerForceFeedbackEffect(AController* Controller,
	                                                              UForceFeedbackEffect* ForceFeedbackEffect,
	                                                              const FName Tag,
	                                                              const bool bLooping,
	                                                              const bool bIgnoreTimeDilation,
	                                                              const bool bPlayWhilePaused)
{
	if (APlayerController* PlayerController = ResolvePlayerController(Controller); PlayerController != nullptr && ForceFeedbackEffect != nullptr)
	{
		FForceFeedbackParameters Parameters;
		Parameters.Tag = Tag;
		Parameters.bLooping = bLooping;
		Parameters.bIgnoreTimeDilation = bIgnoreTimeDilation;
		Parameters.bPlayWhilePaused = bPlayWhilePaused;
		PlayerController->ClientPlayForceFeedback(ForceFeedbackEffect, Parameters);
	}
}

void UBertaControllerUtils::StopControllerForceFeedbackEffect(AController* Controller,
	                                                              UForceFeedbackEffect* ForceFeedbackEffect,
	                                                              const FName Tag)
{
	if (APlayerController* PlayerController = ResolvePlayerController(Controller))
	{
		PlayerController->ClientStopForceFeedback(ForceFeedbackEffect, Tag);
	}
}

FInputDevicePropertyHandle UBertaControllerUtils::ActivateControllerDeviceProperty(AController* Controller,
	                                                                                    const TSubclassOf<UInputDeviceProperty> PropertyClass,
	                                                                                    const bool bLooping,
	                                                                                    const bool bIgnoreTimeDilation,
	                                                                                    const bool bPlayWhilePaused)
{
	APlayerController* PlayerController = ResolvePlayerController(Controller);
	if (PlayerController == nullptr || !PropertyClass)
	{
		return FInputDevicePropertyHandle::InvalidHandle;
	}

	const FPlatformUserId PlatformUserId = PlayerController->GetPlatformUserId();
	UInputDeviceSubsystem* InputDeviceSubsystem = UInputDeviceSubsystem::Get();
	if (!PlatformUserId.IsValid() || InputDeviceSubsystem == nullptr)
	{
		return FInputDevicePropertyHandle::InvalidHandle;
	}

	FActivateDevicePropertyParams Parameters;
	Parameters.UserId = PlatformUserId;
	Parameters.bLooping = bLooping;
	Parameters.bIgnoreTimeDilation = bIgnoreTimeDilation;
	Parameters.bPlayWhilePaused = bPlayWhilePaused;
	return InputDeviceSubsystem->ActivateDevicePropertyOfClass(PropertyClass, Parameters);
}

bool UBertaControllerUtils::IsInputDevicePropertyActive(const FInputDevicePropertyHandle Handle)
{
	if (!Handle.IsValid())
	{
		return false;
	}

	if (UInputDeviceSubsystem* InputDeviceSubsystem = UInputDeviceSubsystem::Get())
	{
		return InputDeviceSubsystem->IsPropertyActive(Handle);
	}

	return false;
}

void UBertaControllerUtils::RemoveInputDeviceProperty(const FInputDevicePropertyHandle Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}

	if (UInputDeviceSubsystem* InputDeviceSubsystem = UInputDeviceSubsystem::Get())
	{
		InputDeviceSubsystem->RemoveDevicePropertyByHandle(Handle);
	}
}

void UBertaControllerUtils::RemoveInputDeviceProperties(const TSet<FInputDevicePropertyHandle>& Handles)
{
	if (Handles.IsEmpty())
	{
		return;
	}

	if (UInputDeviceSubsystem* InputDeviceSubsystem = UInputDeviceSubsystem::Get())
	{
		InputDeviceSubsystem->RemoveDevicePropertyHandles(Handles);
	}
}

void UBertaControllerUtils::RemoveAllInputDeviceProperties()
{
	if (UInputDeviceSubsystem* InputDeviceSubsystem = UInputDeviceSubsystem::Get())
	{
		InputDeviceSubsystem->RemoveAllDeviceProperties();
	}
}
