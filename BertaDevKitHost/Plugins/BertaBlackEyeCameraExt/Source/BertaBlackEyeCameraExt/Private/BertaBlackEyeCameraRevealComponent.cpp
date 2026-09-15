#include "BertaBlackEyeCameraRevealComponent.h"

#include "Actors/BlackEyeCineCameraActorBase.h"
#include "BertaBlackEyeRevealPreset.h"
#include "BertaCinematicParticipant.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaBlackEyeReveal, Log, All);

namespace
{
bool HasValidTiming(const FBertaBlackEyeRevealSettings& Settings)
{
    return FMath::IsFinite(Settings.BlendInTime) && Settings.BlendInTime >= 0.0f &&
        FMath::IsFinite(Settings.HoldTime) && Settings.HoldTime >= 0.0f &&
        FMath::IsFinite(Settings.BlendOutTime) && Settings.BlendOutTime >= 0.0f &&
        FMath::IsFinite(Settings.BlendInExponent) && Settings.BlendInExponent >= 0.0f &&
        FMath::IsFinite(Settings.BlendOutExponent) && Settings.BlendOutExponent >= 0.0f;
}
}

UBertaBlackEyeCameraRevealComponent::UBertaBlackEyeCameraRevealComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UBertaBlackEyeCameraRevealComponent::IsRevealActive() const
{
    return RevealState != EBertaBlackEyeRevealState::Idle;
}

float UBertaBlackEyeCameraRevealComponent::GetPhaseTimeRemaining() const
{
    const UWorld* World = GetWorld();
    return World && PhaseTimer.IsValid() ? FMath::Max(0.0f, World->GetTimerManager().GetTimerRemaining(PhaseTimer)) : 0.0f;
}

APlayerController* UBertaBlackEyeCameraRevealComponent::ResolveLocalPlayerController() const
{
    const UWorld* World = GetWorld();
    if (!IsValid(World)) return nullptr;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PlayerController = It->Get();
        if (IsValid(PlayerController) && PlayerController->IsLocalPlayerController()) return PlayerController;
    }
    return nullptr;
}

bool UBertaBlackEyeCameraRevealComponent::StartCameraReveal()
{
    APlayerController* PlayerController = ResolveLocalPlayerController();
    if (!IsValid(PlayerController))
    {
        UE_LOG(LogBertaBlackEyeReveal, Warning, TEXT("Reveal could not start: no local PlayerController in this world."));
        return false;
    }
    return StartCameraRevealForController(PlayerController, DurationMode);
}

bool UBertaBlackEyeCameraRevealComponent::StartCameraRevealForController(APlayerController* PlayerController,
    EBertaBlackEyeRevealDurationMode RequestedMode)
{
    if (bFinishing || bEndingPlay || IsRevealActive()) return false;

    UWorld* World = GetWorld();
    if (!IsValid(TargetCamera) || TargetCamera->GetWorld() != World)
    {
        UE_LOG(LogBertaBlackEyeReveal, Warning, TEXT("Reveal could not start: TargetCamera is missing, destroyed, or in another world."));
        return false;
    }
    if (!IsValid(PlayerController) || !PlayerController->IsLocalPlayerController() ||
        PlayerController->GetWorld() != World || !IsValid(PlayerController->PlayerCameraManager))
    {
        UE_LOG(LogBertaBlackEyeReveal, Warning, TEXT("Reveal could not start: no usable local PlayerController camera manager."));
        return false;
    }

    const FBertaBlackEyeRevealSettings Settings = IsValid(RevealPreset) ? RevealPreset->Settings : InlineSettings;
    if (!HasValidTiming(Settings))
    {
        UE_LOG(LogBertaBlackEyeReveal, Warning, TEXT("Reveal could not start: timing or exponent is negative or non-finite."));
        return false;
    }

    // UE can return a pending destination when an unrelated blend is already in progress.
    SavedViewTarget = PlayerController->GetViewTarget();
    SavedPawn = PlayerController->GetPawn();
    SavedControlRotation = PlayerController->GetControlRotation();
    ActivePlayerController = PlayerController;
    ActiveTargetCamera = TargetCamera;
    ActiveSettings = Settings;
    ActiveDurationMode = RequestedMode;

    if (!ApplyConfiguredInputLocks(PlayerController))
    {
        RemoveConfiguredInputLocks();
        ActivePlayerController.Reset();
        ActiveTargetCamera.Reset();
        SavedViewTarget.Reset();
        SavedPawn.Reset();
        return false;
    }

    ++SessionSerial;
    RevealState = EBertaBlackEyeRevealState::BlendingIn;
    TargetCamera->OnDestroyed.AddDynamic(this, &ThisClass::HandleTargetDestroyed);
    PlayerController->OnDestroyed.AddDynamic(this, &ThisClass::HandleControllerDestroyed);

    NotifyParticipantsPaused();
    if (RevealState != EBertaBlackEyeRevealState::BlendingIn) return true;

    bGameplayLockApplied = true;
    ApplyGameplayLock();
    if (RevealState != EBertaBlackEyeRevealState::BlendingIn) return true;

    OnCinematicStarted();
    if (RevealState != EBertaBlackEyeRevealState::BlendingIn) return true;

    OnRevealStarted.Broadcast();
    if (RevealState == EBertaBlackEyeRevealState::BlendingIn) BeginBlendIn();
    return true;
}

bool UBertaBlackEyeCameraRevealComponent::ApplyConfiguredInputLocks(APlayerController* PlayerController)
{
    if (ActiveSettings.bDisableMoveInput)
    {
        PlayerController->SetIgnoreMoveInput(true);
        bMoveLockAdded = true;
    }
    if (ActiveSettings.bDisableLookInput)
    {
        PlayerController->SetIgnoreLookInput(true);
        bLookLockAdded = true;
    }
    if (ActiveSettings.bDisablePlayerInput)
    {
        // The binding-free blocker stops legacy and Enhanced Input components below it in the input stack.
        InputBlocker = NewObject<UInputComponent>(this);
        if (!IsValid(InputBlocker)) return false;
        InputBlocker->Priority = TNumericLimits<int32>::Max();
        InputBlocker->bBlockInput = true;
        InputBlocker->RegisterComponent();
        PlayerController->PushInputComponent(InputBlocker);
        bBlockerPushed = PlayerController->IsInputComponentInStack(InputBlocker);
        if (!bBlockerPushed)
        {
            UE_LOG(LogBertaBlackEyeReveal, Warning, TEXT("Reveal could not push its temporary input blocker."));
            return false;
        }
    }
    return true;
}

void UBertaBlackEyeCameraRevealComponent::RemoveConfiguredInputLocks()
{
    APlayerController* PlayerController = ActivePlayerController.Get();
    if (bBlockerPushed && IsValid(PlayerController) && IsValid(InputBlocker))
    {
        PlayerController->PopInputComponent(InputBlocker);
    }
    bBlockerPushed = false;
    if (IsValid(InputBlocker)) InputBlocker->DestroyComponent();
    InputBlocker = nullptr;

    if (bMoveLockAdded && IsValid(PlayerController)) PlayerController->SetIgnoreMoveInput(false);
    bMoveLockAdded = false;
    if (bLookLockAdded && IsValid(PlayerController)) PlayerController->SetIgnoreLookInput(false);
    bLookLockAdded = false;
}

void UBertaBlackEyeCameraRevealComponent::NotifyParticipantsPaused()
{
    if (!bNotifyParticipants) return;

    // External Blueprint callbacks may edit the configured array; iterate a stable start-time copy.
    const TArray<TObjectPtr<AActor>> ConfiguredParticipants = Participants;
    TSet<AActor*> Seen;
    for (AActor* Participant : ConfiguredParticipants)
    {
        if (RevealState != EBertaBlackEyeRevealState::BlendingIn) break;
        if (!IsValid(Participant) || !Participant->GetClass()->ImplementsInterface(UBertaCinematicParticipant::StaticClass()) ||
            Seen.Contains(Participant)) continue;

        Seen.Add(Participant);
        // Record ownership before the external callback, which may synchronously stop this session.
        NotifiedParticipants.Add(Participant);
        IBertaCinematicParticipant::Execute_OnCinematicPause(Participant, this);
    }
}

void UBertaBlackEyeCameraRevealComponent::ResumeNotifiedParticipants()
{
    // Clear ownership before callbacks so a reentrant EndPlay cannot resume twice.
    TArray<TWeakObjectPtr<AActor>> ToResume = MoveTemp(NotifiedParticipants);
    NotifiedParticipants.Reset();
    for (TWeakObjectPtr<AActor>& Participant : ToResume)
    {
        if (Participant.IsValid())
        {
            IBertaCinematicParticipant::Execute_OnCinematicResume(Participant.Get(), this);
        }
    }
}

void UBertaBlackEyeCameraRevealComponent::UnbindSessionActors()
{
    if (ActiveTargetCamera.IsValid())
    {
        ActiveTargetCamera->OnDestroyed.RemoveDynamic(this, &ThisClass::HandleTargetDestroyed);
    }
    if (ActivePlayerController.IsValid())
    {
        ActivePlayerController->OnDestroyed.RemoveDynamic(this, &ThisClass::HandleControllerDestroyed);
    }
}

void UBertaBlackEyeCameraRevealComponent::BeginBlendIn()
{
    if (RevealState != EBertaBlackEyeRevealState::BlendingIn) return;
    APlayerController* PlayerController = ActivePlayerController.Get();
    if (!IsValid(PlayerController))
    {
        BeginBlendOut();
        return;
    }
    if (!ActiveTargetCamera.IsValid())
    {
        BeginBlendOut();
        return;
    }

    PlayerController->SetViewTargetWithBlend(ActiveTargetCamera.Get(), ActiveSettings.BlendInTime,
        ActiveSettings.BlendInFunction.GetValue(), ActiveSettings.BlendInExponent,
        ActiveSettings.bLockOutgoingOnBlendIn);

    if (RevealState != EBertaBlackEyeRevealState::BlendingIn) return;
    if (ActiveSettings.BlendInTime <= 0.0f)
    {
        HandleBlendInFinished(SessionSerial);
    }
    else if (UWorld* World = GetWorld())
    {
        FTimerDelegate Callback = FTimerDelegate::CreateUObject(this, &ThisClass::HandleBlendInFinished, SessionSerial);
        World->GetTimerManager().SetTimer(PhaseTimer, Callback, ActiveSettings.BlendInTime, false);
    }
    else
    {
        BeginBlendOut();
    }
}

void UBertaBlackEyeCameraRevealComponent::HandleBlendInFinished(uint32 ExpectedSerial)
{
    if (ExpectedSerial != SessionSerial || RevealState != EBertaBlackEyeRevealState::BlendingIn) return;
    ClearPhaseTimer();
    if (!ActivePlayerController.IsValid() || !ActiveTargetCamera.IsValid())
    {
        BeginBlendOut();
        return;
    }
    BeginHoldOrActive();
}

void UBertaBlackEyeCameraRevealComponent::BeginHoldOrActive()
{
    if (ActiveDurationMode == EBertaBlackEyeRevealDurationMode::Manual)
    {
        RevealState = EBertaBlackEyeRevealState::Active;
        OnRevealCameraReached.Broadcast();
        return;
    }

    RevealState = EBertaBlackEyeRevealState::Holding;
    OnRevealCameraReached.Broadcast();
    if (RevealState != EBertaBlackEyeRevealState::Holding) return;
    if (ActiveSettings.HoldTime <= 0.0f)
    {
        BeginBlendOut();
    }
    else if (UWorld* World = GetWorld())
    {
        FTimerDelegate Callback = FTimerDelegate::CreateUObject(this, &ThisClass::HandleHoldFinished, SessionSerial);
        World->GetTimerManager().SetTimer(PhaseTimer, Callback, ActiveSettings.HoldTime, false);
    }
    else
    {
        BeginBlendOut();
    }
}

void UBertaBlackEyeCameraRevealComponent::HandleHoldFinished(uint32 ExpectedSerial)
{
    if (ExpectedSerial == SessionSerial && RevealState == EBertaBlackEyeRevealState::Holding) BeginBlendOut();
}

AActor* UBertaBlackEyeCameraRevealComponent::ResolveRestoreViewTarget(APlayerController* PlayerController) const
{
    if (SavedViewTarget.IsValid()) return SavedViewTarget.Get();
    if (SavedPawn.IsValid()) return SavedPawn.Get();
    return PlayerController;
}

void UBertaBlackEyeCameraRevealComponent::RestoreControlRotation()
{
    if (APlayerController* PlayerController = ActivePlayerController.Get())
    {
        PlayerController->SetControlRotation(SavedControlRotation);
    }
}

void UBertaBlackEyeCameraRevealComponent::BeginBlendOut()
{
    if (RevealState != EBertaBlackEyeRevealState::BlendingIn &&
        RevealState != EBertaBlackEyeRevealState::Holding &&
        RevealState != EBertaBlackEyeRevealState::Active) return;

    ClearPhaseTimer();
    ++SessionSerial;
    RevealState = EBertaBlackEyeRevealState::BlendingOut;
    OnCinematicEnding();
    if (RevealState != EBertaBlackEyeRevealState::BlendingOut) return;
    OnRevealEnding.Broadcast();
    if (RevealState != EBertaBlackEyeRevealState::BlendingOut) return;

    APlayerController* PlayerController = ActivePlayerController.Get();
    if (!IsValid(PlayerController))
    {
        FinishReveal();
        return;
    }

    RestoreControlRotation();
    if (!SavedViewTarget.IsValid())
    {
        UE_LOG(LogBertaBlackEyeReveal, Warning,
            TEXT("Saved ViewTarget was destroyed; returning to the captured controller's pawn or controller."));
    }

    AActor* ReturnTarget = ResolveRestoreViewTarget(PlayerController);
    UWorld* World = GetWorld();
    if (!World || !ActiveTargetCamera.IsValid())
    {
        // Complete immediately when the outgoing camera was destroyed or no timer can finish a blend.
        PlayerController->SetViewTarget(ReturnTarget);
    }
    else
    {
        PlayerController->SetViewTargetWithBlend(ReturnTarget, ActiveSettings.BlendOutTime,
            ActiveSettings.BlendOutFunction.GetValue(), ActiveSettings.BlendOutExponent,
            ActiveSettings.bLockOutgoingOnBlendOut);
    }

    if (RevealState != EBertaBlackEyeRevealState::BlendingOut) return;
    if (ActiveSettings.BlendOutTime <= 0.0f || !World || !ActiveTargetCamera.IsValid())
    {
        HandleBlendOutFinished(SessionSerial);
    }
    else
    {
        FTimerDelegate Callback = FTimerDelegate::CreateUObject(this, &ThisClass::HandleBlendOutFinished, SessionSerial);
        World->GetTimerManager().SetTimer(PhaseTimer, Callback, ActiveSettings.BlendOutTime, false);
    }
}

void UBertaBlackEyeCameraRevealComponent::HandleBlendOutFinished(uint32 ExpectedSerial)
{
    if (ExpectedSerial != SessionSerial || RevealState != EBertaBlackEyeRevealState::BlendingOut) return;
    RestoreControlRotation();
    FinishReveal();
}

void UBertaBlackEyeCameraRevealComponent::StopCameraReveal()
{
    if (!bFinishing) BeginBlendOut();
}

void UBertaBlackEyeCameraRevealComponent::ResetCameraReveal()
{
    StopCameraReveal();
}

void UBertaBlackEyeCameraRevealComponent::HandleTargetDestroyed(AActor* DestroyedActor)
{
    if (DestroyedActor == ActiveTargetCamera.Get(true))
    {
        ActiveTargetCamera.Reset();
        if (RevealState == EBertaBlackEyeRevealState::BlendingOut)
        {
            ClearPhaseTimer();
            if (APlayerController* PlayerController = ActivePlayerController.Get())
            {
                PlayerController->SetViewTarget(ResolveRestoreViewTarget(PlayerController));
            }
            HandleBlendOutFinished(SessionSerial);
        }
        else
        {
            StopCameraReveal();
        }
    }
}

void UBertaBlackEyeCameraRevealComponent::HandleControllerDestroyed(AActor* DestroyedActor)
{
    if (DestroyedActor == ActivePlayerController.Get(true)) StopCameraReveal();
}

void UBertaBlackEyeCameraRevealComponent::ClearPhaseTimer()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(PhaseTimer);
    PhaseTimer.Invalidate();
}

void UBertaBlackEyeCameraRevealComponent::FinishReveal()
{
    if (bFinishing || RevealState == EBertaBlackEyeRevealState::Idle) return;
    bFinishing = true;
    ClearPhaseTimer();
    ++SessionSerial;
    UnbindSessionActors();

    // Generic locks are independent of any Blueprint parent-call behavior.
    RemoveConfiguredInputLocks();
    ResumeNotifiedParticipants();
    if (bGameplayLockApplied)
    {
        bGameplayLockApplied = false;
        RemoveGameplayLock();
    }

    ActivePlayerController.Reset();
    ActiveTargetCamera.Reset();
    SavedViewTarget.Reset();
    SavedPawn.Reset();
    RevealState = EBertaBlackEyeRevealState::Idle;
    if (!bEndingPlay)
    {
        OnCinematicFinished();
        if (!bEndingPlay) OnRevealFinished.Broadcast();
    }
    bFinishing = false;
}

void UBertaBlackEyeCameraRevealComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bEndingPlay) return;
    bEndingPlay = true;
    if (bFinishing)
    {
        Super::EndPlay(EndPlayReason);
        return;
    }
    bFinishing = true;
    const bool bWasActive = IsRevealActive();
    ClearPhaseTimer();
    ++SessionSerial;
    UnbindSessionActors();

    UWorld* World = GetWorld();
    APlayerController* PlayerController = ActivePlayerController.Get();
    if (bWasActive && IsValid(PlayerController) && IsValid(World) && !World->bIsTearingDown &&
        (EndPlayReason == EEndPlayReason::Destroyed || EndPlayReason == EEndPlayReason::RemovedFromWorld))
    {
        PlayerController->SetViewTarget(ResolveRestoreViewTarget(PlayerController));
        RestoreControlRotation();
    }

    RemoveConfiguredInputLocks();
    ResumeNotifiedParticipants();
    if (bGameplayLockApplied)
    {
        bGameplayLockApplied = false;
        RemoveGameplayLock();
    }
    ActivePlayerController.Reset();
    ActiveTargetCamera.Reset();
    SavedViewTarget.Reset();
    SavedPawn.Reset();
    RevealState = EBertaBlackEyeRevealState::Idle;
    Super::EndPlay(EndPlayReason);
}

void UBertaBlackEyeCameraRevealComponent::ApplyGameplayLock_Implementation() {}
void UBertaBlackEyeCameraRevealComponent::RemoveGameplayLock_Implementation() {}
void UBertaBlackEyeCameraRevealComponent::OnCinematicStarted_Implementation() {}
void UBertaBlackEyeCameraRevealComponent::OnCinematicEnding_Implementation() {}
void UBertaBlackEyeCameraRevealComponent::OnCinematicFinished_Implementation() {}
