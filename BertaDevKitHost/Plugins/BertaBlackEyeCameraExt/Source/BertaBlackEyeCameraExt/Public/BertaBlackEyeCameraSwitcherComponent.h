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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Switcher")
    TArray<TObjectPtr<ABlackEyeCineCameraActorBase>> Cameras;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Switcher", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendTime = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Switcher")
    TEnumAsByte<EViewTargetBlendFunction> BlendFunction = VTBlend_Linear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Switcher", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendExponent = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Switcher")
    bool bLockOutgoing = false;

    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Switcher")
    bool SelectCamera(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Switcher")
    bool SelectNextCamera();

    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Switcher")
    bool SelectPreviousCamera();

    UFUNCTION(BlueprintPure, Category = "Berta Black Eye Camera|Switcher")
    int32 GetSelectedCameraIndex() const;

    UFUNCTION(BlueprintPure, Category = "Berta Black Eye Camera|Switcher")
    ABlackEyeCineCameraActorBase* GetSelectedCamera() const;

    /** Rejects empty, null, duplicate, cross-world and invalid-timing configuration. */
    UFUNCTION(BlueprintPure, Category = "Berta Black Eye Camera|Switcher")
    bool IsCameraSelectionValid() const;

private:
    APlayerController* ResolveLocalPlayerController() const;
    int32 FindCurrentCameraIndex(APlayerController* PlayerController) const;
};
