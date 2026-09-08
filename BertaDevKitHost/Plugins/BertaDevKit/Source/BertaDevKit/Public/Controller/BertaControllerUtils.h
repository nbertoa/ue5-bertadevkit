// BertaControllerUtils.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/InputDevicePropertyHandle.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaControllerUtils.generated.h"

class AController;
class UForceFeedbackEffect;
class UInputDeviceProperty;
class UBertaControllerUtils;

/** Opaque value handle for one or more native dynamic force-feedback actions. */
USTRUCT(BlueprintType)
struct BERTADEVKIT_API FBertaControllerVibrationHandle
{
	GENERATED_BODY()

private:
	friend class UBertaControllerUtils;

	// UE 5.8's FDynamicForceFeedbackHandle is a uint64. These values are only meaningful to their originating PlayerController.
	TArray<uint64> NativeHandles;

	void AddNativeHandle(const uint64 NativeHandle)
	{
		if (NativeHandle != 0)
		{
			NativeHandles.Add(NativeHandle);
		}
	}

	bool IsValid() const { return !NativeHandles.IsEmpty(); }
	void Reset() { NativeHandles.Reset(); }
};

/** Blueprint conveniences for controller force feedback, light output, and Input Device Properties. */
UCLASS()
class BERTADEVKIT_API UBertaControllerUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Starts native dynamic controller vibration. Duration follows UE semantics: positive values are finite,
	 * zero expires on the next feedback update, and negative values run until stopped. Returns an invalid handle
	 * when Controller is not a PlayerController or no channels are enabled.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|Controller|Feedback",
		meta = (AdvancedDisplay = "bAffectsLeftLarge,bAffectsLeftSmall,bAffectsRightLarge,bAffectsRightSmall", DisplayName = "Play Controller Vibration", ReturnDisplayName = "Vibration Handle"))
	static FBertaControllerVibrationHandle PlayControllerVibration(AController* Controller,
	                                                               float Intensity = 1.0f,
	                                                               float Duration = 0.2f,
	                                                               bool bAffectsLeftLarge = true,
	                                                               bool bAffectsLeftSmall = true,
	                                                               bool bAffectsRightLarge = true,
	                                                               bool bAffectsRightSmall = true);

	/**
	 * Starts native dynamic vibration with an independent magnitude for each motor. Each non-zero channel is a
	 * separate native action; UE 5.8 aggregates dynamic actions by taking the maximum value for each channel.
	 */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Controller|Feedback",
		meta = (DisplayName = "Play Controller Vibration Channels", ReturnDisplayName = "Vibration Handle"))
	static FBertaControllerVibrationHandle PlayControllerVibrationChannels(AController* Controller,
	                                                                       float Duration,
	                                                                       float LeftLarge,
	                                                                       float LeftSmall,
	                                                                       float RightLarge,
	                                                                       float RightSmall);

	/** Stops only the native dynamic actions represented by Handle, then resets Handle even when it has expired. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Controller|Feedback",
		meta = (DisplayName = "Stop Controller Vibration"))
	static void StopControllerVibration(AController* Controller, UPARAM(ref) FBertaControllerVibrationHandle& Handle);

	/** Returns whether Handle contains native vibration actions. A copied handle is an independent value. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Controller|Feedback",
		meta = (DisplayName = "Is Controller Vibration Handle Valid"))
	static bool IsControllerVibrationHandleValid(const FBertaControllerVibrationHandle& Handle);

	/** Sets the resolved PlayerController's controller light color. Does nothing for a non-PlayerController. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Controller|Feedback", meta = (DisplayName = "Set Controller Light Color"))
	static void SetControllerLightColor(AController* Controller, FColor Color);

	/** Resets the resolved PlayerController's controller light color. Does nothing for a non-PlayerController. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Controller|Feedback", meta = (DisplayName = "Reset Controller Light Color"))
	static void ResetControllerLightColor(AController* Controller);

	/** Plays ForceFeedbackEffect using PlayerController's native client-RPC path. Does nothing when the effect is null. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Controller|Feedback",
		meta = (AdvancedDisplay = "bIgnoreTimeDilation,bPlayWhilePaused", DisplayName = "Play Controller Force Feedback Effect"))
	static void PlayControllerForceFeedbackEffect(AController* Controller,
	                                               UForceFeedbackEffect* ForceFeedbackEffect,
	                                               FName Tag = NAME_None,
	                                               bool bLooping = false,
	                                               bool bIgnoreTimeDilation = false,
	                                               bool bPlayWhilePaused = false);

	/**
	 * Stops effects matching every specified criterion. UE stops all effects when both ForceFeedbackEffect is null
	 * and Tag is None; a null effect or None tag acts as an unrestricted criterion.
	 */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Controller|Feedback",
		meta = (DisplayName = "Stop Controller Force Feedback Effect"))
	static void StopControllerForceFeedbackEffect(AController* Controller,
	                                               UForceFeedbackEffect* ForceFeedbackEffect,
	                                               FName Tag = NAME_None);

	/**
	 * Activates PropertyClass for the PlayerController's Platform User. With no explicit device selected, UE targets
	 * that user's default input device. Returns InvalidHandle when the controller, user, class, or subsystem is unavailable.
	 */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Controller|Device Properties",
		meta = (AdvancedDisplay = "bLooping,bIgnoreTimeDilation,bPlayWhilePaused", DisplayName = "Activate Controller Device Property", ReturnDisplayName = "Device Property Handle"))
	static FInputDevicePropertyHandle ActivateControllerDeviceProperty(AController* Controller,
	                                                                   TSubclassOf<UInputDeviceProperty> PropertyClass,
	                                                                   bool bLooping = false,
	                                                                   bool bIgnoreTimeDilation = false,
	                                                                   bool bPlayWhilePaused = false);

	/** Returns whether Handle identifies an active Input Device Property. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Controller|Device Properties",
		meta = (DisplayName = "Is Input Device Property Active"))
	static bool IsInputDevicePropertyActive(FInputDevicePropertyHandle Handle);

	/** Requests removal of the Input Device Property identified by Handle. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Controller|Device Properties",
		meta = (DisplayName = "Remove Input Device Property"))
	static void RemoveInputDeviceProperty(FInputDevicePropertyHandle Handle);

	/** Requests removal of every Input Device Property identified by Handles. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Controller|Device Properties",
		meta = (DisplayName = "Remove Input Device Properties"))
	static void RemoveInputDeviceProperties(const TSet<FInputDevicePropertyHandle>& Handles);

	/** Removes every active Input Device Property for all local Platform Users. */
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Controller|Device Properties",
		meta = (DisplayName = "Remove All Input Device Properties (Global)"))
	static void RemoveAllInputDeviceProperties();
};
