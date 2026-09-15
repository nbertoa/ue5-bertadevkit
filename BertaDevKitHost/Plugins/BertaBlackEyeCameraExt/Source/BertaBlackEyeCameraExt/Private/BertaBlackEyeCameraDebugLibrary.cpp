#include "BertaBlackEyeCameraDebugLibrary.h"

#include "Actors/BlackEyeCineCameraActorBase.h"
#include "BertaBlackEyeCameraRevealComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

bool UBertaBlackEyeCameraDebugLibrary::GetBlackEyeCameraDebugSummary(APlayerController* PlayerController,
    FString& OutSummary)
{
    OutSummary.Reset();
#if UE_BUILD_SHIPPING
    OutSummary = TEXT("Berta Black Eye diagnostics are unavailable in Shipping.");
    return false;
#else
    if (!IsValid(PlayerController) || !PlayerController->IsLocalPlayerController() ||
        !IsValid(PlayerController->PlayerCameraManager) || !IsValid(PlayerController->GetWorld()))
    {
        OutSummary = TEXT("A valid local PlayerController with PlayerCameraManager is required.");
        return false;
    }

    AActor* ViewTarget = PlayerController->GetViewTarget();
    ABlackEyeCineCameraActorBase* BlackEyeCamera = Cast<ABlackEyeCineCameraActorBase>(ViewTarget);
    const FRotator Rotation = PlayerController->GetControlRotation();
    OutSummary = FString::Printf(
        TEXT("PlayerController: %s\nPlayerCameraManager: %s\nCurrentViewTarget: %s (%s)\n")
        TEXT("IsBlackEyeCamera: %s\nControlRotation: Pitch=%.2f Yaw=%.2f Roll=%.2f\n"),
        *GetNameSafe(PlayerController), *GetNameSafe(PlayerController->PlayerCameraManager),
        *GetNameSafe(ViewTarget), ViewTarget ? *GetNameSafe(ViewTarget->GetClass()) : TEXT("None"),
        BlackEyeCamera ? TEXT("true") : TEXT("false"), Rotation.Pitch, Rotation.Yaw, Rotation.Roll);

    if (BlackEyeCamera)
    {
        OutSummary += FString::Printf(TEXT("BlackEye: Class=%s SetControlRotation=%s\n"),
            *GetNameSafe(BlackEyeCamera->GetClass()), BlackEyeCamera->bSetControlRotation ? TEXT("true") : TEXT("false"));
    }

    int32 RevealCount = 0;
    UWorld* World = PlayerController->GetWorld();
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UBertaBlackEyeCameraRevealComponent*> Components;
        It->GetComponents(Components);
        for (UBertaBlackEyeCameraRevealComponent* Reveal : Components)
        {
            if (!IsValid(Reveal) || !Reveal->IsRevealActive() ||
                Reveal->GetActivePlayerController() != PlayerController) continue;

            ++RevealCount;
            const FBertaBlackEyeRevealSettings& Settings = Reveal->GetActiveSettings();
            OutSummary += FString::Printf(
                TEXT("Reveal %d: Owner=%s State=%s Target=%s SavedViewTarget=%s SavedControlRotation=%s\n")
                TEXT("  BlendIn=%.2f Hold=%.2f BlendOut=%.2f PhaseRemaining=%.2f")
                TEXT(" MoveLocked=%s LookLocked=%s FullInputLocked=%s\n"),
                RevealCount, *GetNameSafe(Reveal->GetOwner()),
                *StaticEnum<EBertaBlackEyeRevealState>()->GetNameStringByValue(static_cast<int64>(Reveal->GetRevealState())),
                *GetNameSafe(Reveal->GetActiveTargetCamera()), *GetNameSafe(Reveal->GetSavedViewTarget()),
                *Reveal->GetSavedControlRotation().ToCompactString(), Settings.BlendInTime,
                Settings.HoldTime, Settings.BlendOutTime, Reveal->GetPhaseTimeRemaining(),
                Settings.bDisableMoveInput ? TEXT("true") : TEXT("false"),
                Settings.bDisableLookInput ? TEXT("true") : TEXT("false"),
                Settings.bDisablePlayerInput ? TEXT("true") : TEXT("false"));
        }
    }
    if (RevealCount == 0) OutSummary += TEXT("Reveal: None\n");
    if (RevealCount > 1) OutSummary += TEXT("Reveal conflict: multiple active reveals for this controller.\n");
    return true;
#endif
}
