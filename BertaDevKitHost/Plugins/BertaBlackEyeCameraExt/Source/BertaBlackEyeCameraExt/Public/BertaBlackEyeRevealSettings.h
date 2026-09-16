#pragma once

#include "Camera/PlayerCameraManager.h"
#include "BertaBlackEyeRevealSettings.generated.h"

UENUM(BlueprintType)
enum class EBertaBlackEyeRevealEndMode : uint8
{
    Timed,
    OnEndOverlap,
    Manual
};

UENUM(BlueprintType)
enum class EBertaBlackEyeRevealDurationMode : uint8
{
    Timed,
    Manual
};

UENUM(BlueprintType)
enum class EBertaBlackEyeRevealState : uint8
{
    Idle,
    BlendingIn,
    Holding,
    Active,
    BlendingOut
};

UENUM(BlueprintType)
enum class EBertaBlackEyeExternalCameraChangePolicy : uint8
{
    RestoreConfiguredTarget,
    RespectExternalChange
};

UENUM(BlueprintType)
enum class EBertaBlackEyeReturnTargetPolicy : uint8
{
    CapturedViewTarget,
    CurrentPawn,
    ExplicitTarget
};

/** Camera transition and input behavior shared by triggers and scripted reveals. */
USTRUCT(BlueprintType)
struct BERTABLACKEYECAMERAEXT_API FBertaBlackEyeRevealSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendInTime = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float HoldTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendOutTime = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    TEnumAsByte<EViewTargetBlendFunction> BlendInFunction = VTBlend_Linear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    TEnumAsByte<EViewTargetBlendFunction> BlendOutFunction = VTBlend_Linear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendInExponent = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendOutExponent = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    bool bLockOutgoingOnBlendIn = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    bool bLockOutgoingOnBlendOut = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bDisablePlayerInput = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bDisableMoveInput = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bDisableLookInput = false;
};
