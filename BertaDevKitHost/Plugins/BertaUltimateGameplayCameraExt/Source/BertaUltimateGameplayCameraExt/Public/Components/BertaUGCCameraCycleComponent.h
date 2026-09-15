#pragma once

#include "Camera/Data/UGC_CameraData.h"
#include "Components/ActorComponent.h"

#include "BertaUGCCameraCycleComponent.generated.h"

class AUGC_PlayerCameraManager;

/** Selects ordered UGC data assets for one player without owning the camera data stack. */
UCLASS(ClassGroup = (BertaUltimateGameplayCameraExt), meta = (BlueprintSpawnableComponent))
class BERTAULTIMATEGAMEPLAYCAMERAEXT_API UBertaUGCCameraCycleComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UBertaUGCCameraCycleComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BertaUltimateGameplayCameraExt|Camera Cycle")
	TArray<TObjectPtr<UUGC_CameraDataAssetBase>> CameraPresets;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "BertaUltimateGameplayCameraExt|Camera Cycle")
	int32 CurrentCameraIndex = INDEX_NONE;

	UFUNCTION(BlueprintCallable, Category = "BertaUltimateGameplayCameraExt|Camera Cycle", meta = (ReturnDisplayName = "Success"))
	bool SelectCamera(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "BertaUltimateGameplayCameraExt|Camera Cycle", meta = (ReturnDisplayName = "Success"))
	bool SelectNextCamera();

	UFUNCTION(BlueprintCallable, Category = "BertaUltimateGameplayCameraExt|Camera Cycle", meta = (ReturnDisplayName = "Success"))
	bool SelectPreviousCamera();

protected:
	virtual void BeginPlay() override;

private:
	bool ResolveCameraManager(AUGC_PlayerCameraManager*& OutCameraManager) const;
	bool ValidatePresets() const;
	int32 FindPresetIndex(const UUGC_CameraDataAssetBase* CameraData) const;
	bool SelectAdjacentCamera(bool bNext);
};
