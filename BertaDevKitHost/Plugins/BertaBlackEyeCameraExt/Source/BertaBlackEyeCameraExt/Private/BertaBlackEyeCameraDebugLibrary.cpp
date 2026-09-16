#include "BertaBlackEyeCameraDebugLibrary.h"

#include "Actors/BlackEyeCineCameraActorBase.h"
#include "BertaBlackEyeCameraRevealComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#if !UE_BUILD_SHIPPING
#include "HAL/PlatformTime.h"
#endif

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
    UBertaBlackEyeCameraRevealComponent* LastReplayableReveal = nullptr;
    double LastStart = 0.0;
    UWorld* World = PlayerController->GetWorld();
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UBertaBlackEyeCameraRevealComponent*> Components;
        It->GetComponents(Components);
        for (UBertaBlackEyeCameraRevealComponent* Reveal : Components)
        {
#if !UE_BUILD_SHIPPING
            if (IsValid(Reveal) && Reveal->GetLastRevealController() == PlayerController &&
                Reveal->GetLastSuccessfulRevealStartRealTime() > LastStart)
            {
                LastReplayableReveal = Reveal;
                LastStart = Reveal->GetLastSuccessfulRevealStartRealTime();
            }
#endif
            if (!IsValid(Reveal) || !Reveal->IsRevealActive() ||
                Reveal->GetActivePlayerController() != PlayerController) continue;

            ++RevealCount;
            const FBertaBlackEyeRevealSettings& Settings = Reveal->GetActiveSettings();
            OutSummary += FString::Printf(
                TEXT("Reveal %d: Owner=%s State=%s Target=%s SavedViewTarget=%s SavedControlRotation=%s\n")
                TEXT("  Return=%s ExplicitReturn=%s ExternalChange=%s StillControlsViewTarget=%s\n")
                TEXT("  BlendIn=%.2f Hold=%.2f BlendOut=%.2f PhaseRemaining=%.2f")
                TEXT(" MoveLocked=%s LookLocked=%s FullInputLocked=%s\n"),
                RevealCount, *GetNameSafe(Reveal->GetOwner()),
                *StaticEnum<EBertaBlackEyeRevealState>()->GetNameStringByValue(static_cast<int64>(Reveal->GetRevealState())),
                *GetNameSafe(Reveal->GetActiveTargetCamera()), *GetNameSafe(Reveal->GetSavedViewTarget()),
                *Reveal->GetSavedControlRotation().ToCompactString(),
                *StaticEnum<EBertaBlackEyeReturnTargetPolicy>()->GetNameStringByValue(static_cast<int64>(Reveal->GetActiveReturnTargetPolicy())),
                Reveal->GetActiveReturnTargetPolicy() == EBertaBlackEyeReturnTargetPolicy::ExplicitTarget
                    ? *GetNameSafe(Reveal->GetActiveExplicitReturnTarget()) : TEXT("n/a"),
                *StaticEnum<EBertaBlackEyeExternalCameraChangePolicy>()->GetNameStringByValue(static_cast<int64>(Reveal->GetActiveExternalCameraChangePolicy())),
                Reveal->GetRevealState() == EBertaBlackEyeRevealState::BlendingOut ? TEXT("n/a (own return requested)")
                    : (Reveal->IsRevealStillControllingViewTarget() ? TEXT("true") : TEXT("false")),
                Settings.BlendInTime,
                Settings.HoldTime, Settings.BlendOutTime, Reveal->GetPhaseTimeRemaining(),
                Settings.bDisableMoveInput ? TEXT("true") : TEXT("false"),
                Settings.bDisableLookInput ? TEXT("true") : TEXT("false"),
                Settings.bDisablePlayerInput ? TEXT("true") : TEXT("false"));
        }
    }
    if (RevealCount == 0) OutSummary += TEXT("Reveal: None\n");
    if (RevealCount > 1) OutSummary += TEXT("Reveal conflict: multiple active reveals for this controller.\n");
    if (LastReplayableReveal)
    {
        OutSummary += FString::Printf(TEXT("LastReplayableReveal: Component=%s Mode=%s LastStartedAgo=%.1fs\n"),
            *GetNameSafe(LastReplayableReveal),
            LastReplayableReveal->GetLastRevealStartMode() == EBertaBlackEyeRevealDurationMode::Manual
                ? TEXT("Manual") : TEXT("Timed"), FPlatformTime::Seconds() - LastStart);
    }
    return true;
#endif
}
