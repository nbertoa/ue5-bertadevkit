#include "Debug/BertaUGCCameraDebugLibrary.h"

#include "Camera/CameraModifier.h"
#include "Camera/Data/UGC_CameraData.h"
#include "Camera/UGC_PlayerCameraManager.h"
#include "Camera/Modifiers/UGC_CameraModifier.h"
#include "Components/BertaUGCCameraCycleComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

bool UBertaUGCCameraDebugLibrary::GetUGCCameraDebugSummary(APlayerController* PlayerController, FString& OutSummary)
{
	OutSummary.Reset();
#if UE_BUILD_SHIPPING
	return false;
#else
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return false;
	AUGC_PlayerCameraManager* Manager = Cast<AUGC_PlayerCameraManager>(PlayerController->PlayerCameraManager);
	if (!IsValid(Manager)) return false;

	const UUGC_CameraDataAssetBase* ActivePreset = Manager->GetCurrentCameraDataAsset();
	if (!IsValid(ActivePreset)) ActivePreset = nullptr;
	const UBertaUGCCameraCycleComponent* Cycle = PlayerController->FindComponentByClass<UBertaUGCCameraCycleComponent>();
	if (!IsValid(Cycle)) Cycle = nullptr;
	const int32 ActiveCycleIndex = Cycle ? Cycle->CameraPresets.IndexOfByPredicate(
		[ActivePreset](const TObjectPtr<UUGC_CameraDataAssetBase>& Preset) { return ActivePreset && Preset.Get() == ActivePreset; }) : INDEX_NONE;
	const FMinimalViewInfo& POV = Manager->GetCameraCacheView();
	OutSummary = FString::Printf(TEXT("Controller=%s Manager=%s Pawn=%s\nActivePreset=%s CycleActiveIndex=%d\nFinalPOV Location=%s Rotation=%s FOV=%.2f"),
		*GetNameSafe(PlayerController), *GetNameSafe(Manager), *GetNameSafe(PlayerController->GetPawn()),
		ActivePreset ? *ActivePreset->GetPathName() : TEXT("<none>"), ActiveCycleIndex,
		*POV.Location.ToString(), *POV.Rotation.ToString(), POV.FOV);

	const USpringArmComponent* Arm = Manager->GetOwnerSpringArmComponent();
	OutSummary += IsValid(Arm) ? FString::Printf(TEXT("\nSpringArm=%s TargetArmLength=%.2f SocketOffset=%s TargetOffset=%s UsePawnControlRotation=%s"),
		*Arm->GetPathName(), Arm->TargetArmLength, *Arm->SocketOffset.ToString(), *Arm->TargetOffset.ToString(),
		Arm->bUsePawnControlRotation ? TEXT("true") : TEXT("false")) : TEXT("\nSpringArm=<none>");

	int32 EnabledUGC = 0;
	int32 DisabledUGC = 0;
	int32 EnabledExternal = 0;
	int32 DisabledExternal = 0;
	Manager->ForEachCameraModifier([&](UCameraModifier* Modifier)
	{
		if (!IsValid(Modifier)) return true;
		const bool bUGC = Modifier->IsA<UUGC_CameraModifier>();
		const bool bDisabled = Modifier->IsDisabled();
		if (bUGC) (bDisabled ? DisabledUGC : EnabledUGC)++;
		else (bDisabled ? DisabledExternal : EnabledExternal)++;
		return true;
	});
	OutSummary += FString::Printf(TEXT("\nModifiers UGC=%d enabled/%d disabled External=%d enabled/%d disabled"),
		EnabledUGC, DisabledUGC, EnabledExternal, DisabledExternal);
	return true;
#endif
}
