#include "Components/BertaUGCCameraCycleComponent.h"

#include "BertaUltimateGameplayCameraExt.h"
#include "Camera/UGC_PlayerCameraManager.h"
#include "Containers/Set.h"
#include "GameFramework/PlayerController.h"

UBertaUGCCameraCycleComponent::UBertaUGCCameraCycleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBertaUGCCameraCycleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ValidatePresets())
	{
		return;
	}

	// The manager may not be ready yet. Public operations reconcile against UGC again.
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogBertaUltimateGameplayCameraExt, Warning,
			TEXT("Camera cycle component %s must be owned by a PlayerController."), *GetNameSafe(this));
		return;
	}

	if (AUGC_PlayerCameraManager* CameraManager = Cast<AUGC_PlayerCameraManager>(PlayerController->PlayerCameraManager))
	{
		if (IsValid(CameraManager))
		{
			const int32 ActiveIndex = FindPresetIndex(CameraManager->GetCurrentCameraDataAsset());
			if (ActiveIndex != INDEX_NONE)
			{
				CurrentCameraIndex = ActiveIndex;
			}
		}
	}
}

bool UBertaUGCCameraCycleComponent::ResolveCameraManager(AUGC_PlayerCameraManager*& OutCameraManager) const
{
	OutCameraManager = nullptr;
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogBertaUltimateGameplayCameraExt, Warning,
			TEXT("Camera cycle component %s must be owned by a valid PlayerController."), *GetNameSafe(this));
		return false;
	}

	OutCameraManager = Cast<AUGC_PlayerCameraManager>(PlayerController->PlayerCameraManager);
	if (!IsValid(OutCameraManager))
	{
		UE_LOG(LogBertaUltimateGameplayCameraExt, Warning,
			TEXT("PlayerController %s has no valid UGC PlayerCameraManager for camera cycling."), *GetNameSafe(PlayerController));
		OutCameraManager = nullptr;
		return false;
	}

	return true;
}

bool UBertaUGCCameraCycleComponent::ValidatePresets() const
{
	if (CameraPresets.IsEmpty())
	{
		return false;
	}

	TSet<const UUGC_CameraDataAssetBase*> SeenPresets;
	for (int32 Index = 0; Index < CameraPresets.Num(); ++Index)
	{
		const UUGC_CameraDataAssetBase* Preset = CameraPresets[Index].Get();
		if (!IsValid(Preset))
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning,
				TEXT("Camera cycle component %s has an invalid preset at index %d."), *GetNameSafe(this), Index);
			return false;
		}
		if (SeenPresets.Contains(Preset))
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning,
				TEXT("Camera cycle component %s lists preset %s more than once; indices would be ambiguous."),
				*GetNameSafe(this), *GetNameSafe(Preset));
			return false;
		}
		SeenPresets.Add(Preset);
	}

	return true;
}

int32 UBertaUGCCameraCycleComponent::FindPresetIndex(const UUGC_CameraDataAssetBase* CameraData) const
{
	return CameraData ? CameraPresets.IndexOfByPredicate(
		[CameraData](const TObjectPtr<UUGC_CameraDataAssetBase>& Preset) { return Preset.Get() == CameraData; }) : INDEX_NONE;
}

bool UBertaUGCCameraCycleComponent::SelectCamera(int32 Index)
{
	AUGC_PlayerCameraManager* CameraManager = nullptr;
	if (!ResolveCameraManager(CameraManager) || !ValidatePresets() || !CameraPresets.IsValidIndex(Index))
	{
		return false;
	}

	UUGC_CameraDataAssetBase* Target = CameraPresets[Index].Get();
	UUGC_CameraDataAssetBase* Current = CameraManager->GetCurrentCameraDataAsset();
	const int32 ActiveIndex = FindPresetIndex(Current);
	if (Current && ActiveIndex == INDEX_NONE)
	{
		UE_LOG(LogBertaUltimateGameplayCameraExt, Verbose,
			TEXT("UGC camera %s is outside the cycle; selection is blocked while it is active."), *GetNameSafe(Current));
		return false;
	}

	if (ActiveIndex != INDEX_NONE)
	{
		CurrentCameraIndex = ActiveIndex;
	}

	if (Current == Target)
	{
		CurrentCameraIndex = Index;
		return true;
	}

	if (Current)
	{
		// PopCameraData(Current) also removes matching entries below the head in UGC.
		CameraManager->PopCameraDataHead();
	}
	CameraManager->PushCameraData(Target);

	if (CameraManager->GetCurrentCameraDataAsset() != Target)
	{
		UE_LOG(LogBertaUltimateGameplayCameraExt, Error,
			TEXT("UGC PlayerCameraManager %s did not keep selected camera %s active after PushCameraData."),
			*GetNameSafe(CameraManager), *GetNameSafe(Target));
		return false;
	}

	CurrentCameraIndex = Index;
	return true;
}

bool UBertaUGCCameraCycleComponent::SelectAdjacentCamera(bool bNext)
{
	AUGC_PlayerCameraManager* CameraManager = nullptr;
	if (!ResolveCameraManager(CameraManager) || !ValidatePresets())
	{
		return false;
	}

	UUGC_CameraDataAssetBase* Current = CameraManager->GetCurrentCameraDataAsset();
	const int32 ActiveIndex = FindPresetIndex(Current);
	if (Current && ActiveIndex == INDEX_NONE)
	{
		return false;
	}

	const int32 TargetIndex = ActiveIndex == INDEX_NONE
		? (bNext ? 0 : CameraPresets.Num() - 1)
		: (bNext ? (ActiveIndex + 1) % CameraPresets.Num()
			: (ActiveIndex == 0 ? CameraPresets.Num() - 1 : ActiveIndex - 1));
	return SelectCamera(TargetIndex);
}

bool UBertaUGCCameraCycleComponent::SelectNextCamera()
{
	return SelectAdjacentCamera(true);
}

bool UBertaUGCCameraCycleComponent::SelectPreviousCamera()
{
	return SelectAdjacentCamera(false);
}
