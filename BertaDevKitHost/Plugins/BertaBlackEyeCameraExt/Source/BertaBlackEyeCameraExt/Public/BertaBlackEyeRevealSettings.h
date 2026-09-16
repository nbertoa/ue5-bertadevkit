#pragma once

#include "Camera/PlayerCameraManager.h"
#include "BertaBlackEyeRevealSettings.generated.h"

/** Trigger activation/exit policy; OnEndOverlap uses a Manual component session. */
UENUM(BlueprintType)
enum class EBertaBlackEyeRevealEndMode : uint8
{
    /** Start on overlap or explicit Start, then exit after HoldTime. */
    Timed,
    /** Start on overlap and exit only when its matching overlap owner leaves or is destroyed. */
    OnEndOverlap,
    /** Never start from overlap; use the trigger's explicit Start/Stop calls. */
    Manual
};

/** Session duration policy independent of the trigger's overlap policy. */
UENUM(BlueprintType)
enum class EBertaBlackEyeRevealDurationMode : uint8
{
    /** Exit automatically after the requested blend-in and HoldTime. */
    Timed,
    /** Stay active after blend-in until StopCameraReveal is called. */
    Manual
};

/** Observable phase of one reveal session; CameraReached marks elapsed blend-in time. */
UENUM(BlueprintType)
enum class EBertaBlackEyeRevealState : uint8
{
    /** No session owns input, participants, or a camera return. */
    Idle,
    /** Session accepted; the transition into TargetCamera is being prepared or requested. */
    BlendingIn,
    /** Timed session is waiting for HoldTime to elapse. */
    Holding,
    /** Manual session is waiting for an explicit Stop. */
    Active,
    /** Exit phase: Ending hooks run, then return or immediate owned cleanup follows. */
    BlendingOut
};

/** Decide whether an external ViewTarget change should override Berta's configured return. */
UENUM(BlueprintType)
enum class EBertaBlackEyeExternalCameraChangePolicy : uint8
{
    /** Complete the configured return even if another system selected a different ViewTarget. */
    RestoreConfiguredTarget,
    /** At exit, leave an external ViewTarget and ControlRotation intact; still remove Berta-owned locks. */
    RespectExternalChange
};

/** Choose the return ViewTarget; the effective policy is captured when a session starts. */
UENUM(BlueprintType)
enum class EBertaBlackEyeReturnTargetPolicy : uint8
{
    /** Return to the captured ViewTarget, falling back to the captured pawn then controller. */
    CapturedViewTarget,
    /** Resolve the controller's pawn at exit, falling back to the captured ViewTarget then controller. */
    CurrentPawn,
    /** Use ExplicitReturnTarget, falling back to the captured ViewTarget, current pawn, then controller. */
    ExplicitTarget
};

/** Camera transition and input behavior shared by triggers and scripted reveals. */
USTRUCT(BlueprintType)
struct BERTABLACKEYECAMERAEXT_API FBertaBlackEyeRevealSettings
{
    GENERATED_BODY()

    /** Requested transition duration; zero makes the camera change immediate. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendInTime = 0.8f;

    /** Wait after CameraReached in Timed mode only; zero exits immediately. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float HoldTime = 1.0f;

    /** Requested return duration; zero completes without a return timer. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendOutTime = 0.8f;

    /** Passed to the active PlayerCameraManager through SetViewTargetWithBlend. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    TEnumAsByte<EViewTargetBlendFunction> BlendInFunction = VTBlend_Linear;

    /** Passed to the active PlayerCameraManager through SetViewTargetWithBlend. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    TEnumAsByte<EViewTargetBlendFunction> BlendOutFunction = VTBlend_Linear;

    /** Blend-function exponent; runtime rejects negative or non-finite values. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendInExponent = 2.0f;

    /** Blend-function exponent; runtime rejects negative or non-finite values. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendOutExponent = 2.0f;

    /** Ask the active camera manager to lock the outgoing POV during blend-in. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    bool bLockOutgoingOnBlendIn = false;

    /** Ask the active camera manager to lock the outgoing POV during the normal return blend. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    bool bLockOutgoingOnBlendOut = false;

    /** Push a temporary high-priority, binding-free input blocker; remove it on cleanup. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bDisablePlayerInput = false;

    /** Add and later remove this reveal's IgnoreMoveInput contribution. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bDisableMoveInput = false;

    /** Add and later remove this reveal's IgnoreLookInput contribution; normal return restores rotation first. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bDisableLookInput = false;
};
