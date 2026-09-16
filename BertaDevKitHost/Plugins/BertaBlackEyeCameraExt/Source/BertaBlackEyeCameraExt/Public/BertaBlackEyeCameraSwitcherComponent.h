#pragma once

#include "Camera/PlayerCameraManager.h"
#include "Components/ActorComponent.h"
#include "BertaBlackEyeCameraSwitcherComponent.generated.h"

class ABlackEyeCineCameraActorBase;
class APlayerController;

/** Explicit prototype cycling among configured Black Eye cameras; owns no camera stack. */
UCLASS(ClassGroup = "Berta Black Eye Camera", meta = (BlueprintSpawnableComponent))
class BERTABLACKEYECAMERAEXT_API UBertaBlackEyeCameraSwitcherComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBertaBlackEyeCameraSwitcherComponent();

    /** Ordered, unique Black Eye cameras in this world; an empty/null/duplicate entry rejects selection. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Switcher")
    TArray<TObjectPtr<ABlackEyeCineCameraActorBase>> Cameras;

    /** Requested switch duration for SetViewTargetWithBlend; zero is allowed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Switcher", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendTime = 0.4f;

    /** Passed to the active PlayerCameraManager with BlendExponent. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Switcher")
    TEnumAsByte<EViewTargetBlendFunction> BlendFunction = VTBlend_Linear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Switcher", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendExponent = 2.0f;

    /** Ask the active manager to lock outgoing POV during a camera selection blend. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Switcher")
    bool bLockOutgoing = false;

    /** Select a valid index for the local controller; already-selected camera returns true without blending. */
    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Switcher")
    bool SelectCamera(int32 Index);

    /** Wrap to the next camera; select index zero when the current ViewTarget is outside the list. */
    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Switcher")
    bool SelectNextCamera();

    /** Wrap to the previous camera; select the last entry when the current ViewTarget is outside the list. */
    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Switcher")
    bool SelectPreviousCamera();

    /** Index of the current/pending ViewTarget in Cameras, or INDEX_NONE. */
    UFUNCTION(BlueprintPure, Category = "Berta Black Eye Camera|Switcher")
    int32 GetSelectedCameraIndex() const;

    /** Selected configured camera, or nullptr when selection is unavailable. */
    UFUNCTION(BlueprintPure, Category = "Berta Black Eye Camera|Switcher")
    ABlackEyeCineCameraActorBase* GetSelectedCamera() const;

    /** Rejects empty, null, duplicate, cross-world and negative/non-finite timing configuration. */
    UFUNCTION(BlueprintPure, Category = "Berta Black Eye Camera|Switcher")
    bool IsCameraSelectionValid() const;

private:
    APlayerController* ResolveLocalPlayerController() const;
    int32 FindCurrentCameraIndex(APlayerController* PlayerController) const;
};
