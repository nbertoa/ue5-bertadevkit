#include "BertaBlackEyeCameraRevealComponent.h"

#include "Actors/BlackEyeCineCameraActorBase.h"
#include "BertaBlackEyeRevealPreset.h"
#include "BertaCinematicParticipant.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogBertaBlackEyeReveal, Log, All);
#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<int32> CVarBertaBlackEyeTrace(TEXT("Berta.BlackEye.Trace"), 0,
    TEXT("Trace Black Eye reveal lifecycles in real time (0=off, 1=on)."));
#define BERTA_REVEAL_TRACE(Event) do { if (CVarBertaBlackEyeTrace.GetValueOnGameThread() != 0) TraceRevealEvent(Event); } while (false)
#else
#define BERTA_REVEAL_TRACE(Event) do {} while (false)
#endif

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
#if !UE_BUILD_SHIPPING
void UBertaBlackEyeCameraRevealComponent::TraceRevealEvent(const FString& Event) const
{
    UE_LOG(LogBertaBlackEyeReveal, Display, TEXT("[Reveal:%s:%u +%.3f] %s"),
        *GetNameSafe(GetOwner()), RevealSessionId,
        SessionStartRealTime > 0.0 ? FPlatformTime::Seconds() - SessionStartRealTime : 0.0, *Event);
}
#endif

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
    BERTA_REVEAL_TRACE(TEXT("Start requested from component"));
    APlayerController* PlayerController = ResolveLocalPlayerController();
    if (!IsValid(PlayerController))
    {
        UE_LOG(LogBertaBlackEyeReveal, Warning, TEXT("Reveal could not start: no local PlayerController in this world."));
        BERTA_REVEAL_TRACE(TEXT("Start rejected: no local PlayerController"));
        return false;
    }
    return StartCameraRevealForController(PlayerController, DurationMode);
}

bool UBertaBlackEyeCameraRevealComponent::StartCameraRevealForController(APlayerController* PlayerController,
    EBertaBlackEyeRevealDurationMode RequestedMode)
{
    BERTA_REVEAL_TRACE(FString::Printf(TEXT("Start requested: Controller=%s Mode=%s"),
        *GetNameSafe(PlayerController), RequestedMode == EBertaBlackEyeRevealDurationMode::Manual ? TEXT("Manual") : TEXT("Timed")));
    if (bFinishing || bEndingPlay || IsRevealActive())
    {
        BERTA_REVEAL_TRACE(TEXT("Start rejected: component busy or ending play"));
        return false;
    }

    UWorld* World = GetWorld();
    if (!IsValid(TargetCamera) || TargetCamera->GetWorld() != World)
    {
        UE_LOG(LogBertaBlackEyeReveal, Warning, TEXT("Reveal could not start: TargetCamera is missing, destroyed, or in another world."));
        BERTA_REVEAL_TRACE(TEXT("Start rejected: TargetCamera invalid"));
        return false;
    }
    if (!IsValid(PlayerController) || !PlayerController->IsLocalPlayerController() ||
        PlayerController->GetWorld() != World || !IsValid(PlayerController->PlayerCameraManager))
    {
        UE_LOG(LogBertaBlackEyeReveal, Warning, TEXT("Reveal could not start: no usable local PlayerController camera manager."));
        BERTA_REVEAL_TRACE(TEXT("Start rejected: PlayerController or camera manager invalid"));
        return false;
    }

    const FBertaBlackEyeRevealSettings Settings = IsValid(RevealPreset) ? RevealPreset->Settings : InlineSettings;
    if (!HasValidTiming(Settings))
    {
        UE_LOG(LogBertaBlackEyeReveal, Warning, TEXT("Reveal could not start: timing or exponent is negative or non-finite."));
        BERTA_REVEAL_TRACE(TEXT("Start rejected: invalid timing"));
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
    ActiveReturnTargetPolicy = ReturnTargetPolicy;
    ActiveExternalCameraChangePolicy = ExternalCameraChangePolicy;
    ActiveExplicitReturnTarget = ExplicitReturnTarget;
    BERTA_REVEAL_TRACE(FString::Printf(TEXT("Captured: ViewTarget=%s Pawn=%s ControlRotation=%s TargetCamera=%s"),
        *GetNameSafe(SavedViewTarget.Get()), *GetNameSafe(SavedPawn.Get()),
        *SavedControlRotation.ToCompactString(), *GetNameSafe(ActiveTargetCamera.Get())));

    if (!ApplyConfiguredInputLocks(PlayerController))
    {
        RemoveConfiguredInputLocks();
        ActivePlayerController.Reset();
        ActiveTargetCamera.Reset();
        SavedViewTarget.Reset();
        SavedPawn.Reset();
        ActiveExplicitReturnTarget.Reset();
        BERTA_REVEAL_TRACE(TEXT("Start rejected: input blocker could not be applied; locks rolled back"));
        return false;
    }

    ++SessionSerial;
#if !UE_BUILD_SHIPPING
    ++RevealSessionId;
    SessionStartRealTime = FPlatformTime::Seconds();
    LastSuccessfulRevealStartRealTime = SessionStartRealTime;
    LastRevealController = PlayerController;
    LastRevealStartMode = RequestedMode;
#endif
    RevealState = EBertaBlackEyeRevealState::BlendingIn;
    BERTA_REVEAL_TRACE(FString::Printf(TEXT("Start accepted: BlendIn=%.3f Hold=%.3f BlendOut=%.3f Return=%s Explicit=%s External=%s Input(Move=%d Look=%d Full=%d)"),
        ActiveSettings.BlendInTime, ActiveSettings.HoldTime, ActiveSettings.BlendOutTime,
        *StaticEnum<EBertaBlackEyeReturnTargetPolicy>()->GetNameStringByValue(static_cast<int64>(ActiveReturnTargetPolicy)),
        *GetNameSafe(ActiveExplicitReturnTarget.Get()),
        *StaticEnum<EBertaBlackEyeExternalCameraChangePolicy>()->GetNameStringByValue(static_cast<int64>(ActiveExternalCameraChangePolicy)),
        bMoveLockAdded, bLookLockAdded, bBlockerPushed));
    TargetCamera->OnDestroyed.AddDynamic(this, &ThisClass::HandleTargetDestroyed);
    PlayerController->OnDestroyed.AddDynamic(this, &ThisClass::HandleControllerDestroyed);

    NotifyParticipantsPaused();
    BERTA_REVEAL_TRACE(FString::Printf(TEXT("Participants paused: %d"), NotifiedParticipants.Num()));
    if (RevealState != EBertaBlackEyeRevealState::BlendingIn) return true;

    bGameplayLockApplied = true;
    ApplyGameplayLock();
    BERTA_REVEAL_TRACE(TEXT("Gameplay lock applied"));
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

int32 UBertaBlackEyeCameraRevealComponent::ResumeNotifiedParticipants()
{
    // Clear ownership before callbacks so a reentrant EndPlay cannot resume twice.
    TArray<TWeakObjectPtr<AActor>> ToResume = MoveTemp(NotifiedParticipants);
    NotifiedParticipants.Reset();
    int32 ResumedCount = 0;
    for (TWeakObjectPtr<AActor>& Participant : ToResume)
    {
        if (Participant.IsValid())
        {
            IBertaCinematicParticipant::Execute_OnCinematicResume(Participant.Get(), this);
            ++ResumedCount;
        }
    }
    return ResumedCount;
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
        BERTA_REVEAL_TRACE(TEXT("Controller lost before BlendIn"));
        BeginBlendOut();
        return;
    }
    if (!ActiveTargetCamera.IsValid())
    {
        BERTA_REVEAL_TRACE(TEXT("TargetCamera lost before BlendIn"));
        BeginBlendOut();
        return;
    }

    BERTA_REVEAL_TRACE(FString::Printf(TEXT("BlendIn requested: Target=%s Time=%.3f"),
        *GetNameSafe(ActiveTargetCamera.Get()), ActiveSettings.BlendInTime));
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
        BERTA_REVEAL_TRACE(TEXT("Controller or TargetCamera lost during BlendIn"));
        BeginBlendOut();
        return;
    }
    BeginHoldOrActive();
}

void UBertaBlackEyeCameraRevealComponent::BeginHoldOrActive()
{
    BERTA_REVEAL_TRACE(TEXT("CameraReached: BlendIn duration elapsed"));
    if (ActiveDurationMode == EBertaBlackEyeRevealDurationMode::Manual)
    {
        RevealState = EBertaBlackEyeRevealState::Active;
        BERTA_REVEAL_TRACE(TEXT("Manual active entered"));
        OnRevealCameraReached.Broadcast();
        return;
    }

    RevealState = EBertaBlackEyeRevealState::Holding;
    BERTA_REVEAL_TRACE(FString::Printf(TEXT("Hold entered: %.3f seconds"), ActiveSettings.HoldTime));
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

bool UBertaBlackEyeCameraRevealComponent::IsRevealStillControllingViewTarget() const
{
    const APlayerController* PlayerController = ActivePlayerController.Get();
    return IsValid(PlayerController) && ActiveTargetCamera.IsValid() &&
        PlayerController->GetViewTarget() == ActiveTargetCamera.Get();
}

AActor* UBertaBlackEyeCameraRevealComponent::ResolveReturnViewTarget(
    APlayerController* PlayerController, bool& bUsedFallback) const
{
    bUsedFallback = false;
    switch (ActiveReturnTargetPolicy)
    {
    case EBertaBlackEyeReturnTargetPolicy::CurrentPawn:
        if (IsValid(PlayerController->GetPawn())) return PlayerController->GetPawn();
        bUsedFallback = true;
        BERTA_REVEAL_TRACE(TEXT("CurrentPawn unavailable; trying captured ViewTarget"));
        if (SavedViewTarget.IsValid()) return SavedViewTarget.Get();
        break;
    case EBertaBlackEyeReturnTargetPolicy::ExplicitTarget:
        if (ActiveExplicitReturnTarget.IsValid()) return ActiveExplicitReturnTarget.Get();
        bUsedFallback = true;
        UE_LOG(LogBertaBlackEyeReveal, Warning,
            TEXT("ExplicitReturnTarget is missing or destroyed; falling back to the captured ViewTarget, current pawn, or controller."));
        BERTA_REVEAL_TRACE(TEXT("ExplicitReturnTarget missing or destroyed; fallback required"));
        if (SavedViewTarget.IsValid()) return SavedViewTarget.Get();
        if (IsValid(PlayerController->GetPawn())) return PlayerController->GetPawn();
        break;
    case EBertaBlackEyeReturnTargetPolicy::CapturedViewTarget:
    default:
        if (SavedViewTarget.IsValid()) return SavedViewTarget.Get();
        bUsedFallback = true;
        UE_LOG(LogBertaBlackEyeReveal, Warning,
            TEXT("Captured ViewTarget is missing or destroyed; falling back to the captured pawn or controller."));
        BERTA_REVEAL_TRACE(TEXT("Captured ViewTarget missing or destroyed; fallback required"));
        if (SavedPawn.IsValid()) return SavedPawn.Get();
        break;
    }
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
    BERTA_REVEAL_TRACE(TEXT("Ending entered"));
    OnCinematicEnding();
    if (RevealState != EBertaBlackEyeRevealState::BlendingOut) return;
    OnRevealEnding.Broadcast();
    if (RevealState != EBertaBlackEyeRevealState::BlendingOut) return;

    APlayerController* PlayerController = ActivePlayerController.Get();
    if (!IsValid(PlayerController))
    {
        BERTA_REVEAL_TRACE(TEXT("Controller lost on exit; only Berta-owned cleanup will run"));
        FinishReveal();
        return;
    }

    // GetViewTarget includes UE's pending blend destination. Check before requesting our own return.
    if (ActiveExternalCameraChangePolicy == EBertaBlackEyeExternalCameraChangePolicy::RespectExternalChange &&
        ActiveTargetCamera.IsValid() && !IsRevealStillControllingViewTarget())
    {
        BERTA_REVEAL_TRACE(FString::Printf(TEXT("External ViewTarget change detected: Current=%s TargetCamera=%s; skipping return and ControlRotation"),
            *GetNameSafe(PlayerController->GetViewTarget()), *GetNameSafe(ActiveTargetCamera.Get())));
        UE_LOG(LogBertaBlackEyeReveal, Display,
            TEXT("External ViewTarget %s took camera ownership from reveal %s; finishing without restoring camera or ControlRotation."),
            *GetNameSafe(PlayerController->GetViewTarget()), *GetNameSafe(GetOwner()));
        FinishReveal();
        return;
    }

    RestoreControlRotation();
    BERTA_REVEAL_TRACE(TEXT("ControlRotation restored (pre-blend)"));
    bool bUsedFallback = false;
    AActor* ReturnTarget = ResolveReturnViewTarget(PlayerController, bUsedFallback);
    BERTA_REVEAL_TRACE(FString::Printf(TEXT("ReturnPolicy=%s ResolvedReturnTarget=%s Fallback=%s"),
        *StaticEnum<EBertaBlackEyeReturnTargetPolicy>()->GetNameStringByValue(static_cast<int64>(ActiveReturnTargetPolicy)),
        *GetNameSafe(ReturnTarget), bUsedFallback ? TEXT("true") : TEXT("false")));
    UWorld* World = GetWorld();
    if (!World || !ActiveTargetCamera.IsValid())
    {
        // Complete immediately when the outgoing camera was destroyed or no timer can finish a blend.
        PlayerController->SetViewTarget(ReturnTarget);
        BERTA_REVEAL_TRACE(TEXT("Immediate return requested: no outgoing camera or timer world"));
    }
    else
    {
        PlayerController->SetViewTargetWithBlend(ReturnTarget, ActiveSettings.BlendOutTime,
            ActiveSettings.BlendOutFunction.GetValue(), ActiveSettings.BlendOutExponent,
            ActiveSettings.bLockOutgoingOnBlendOut);
        BERTA_REVEAL_TRACE(FString::Printf(TEXT("BlendOut requested: Time=%.3f"), ActiveSettings.BlendOutTime));
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
    BERTA_REVEAL_TRACE(TEXT("ControlRotation restored (final)"));
    FinishReveal();
}

void UBertaBlackEyeCameraRevealComponent::StopCameraReveal()
{
    BERTA_REVEAL_TRACE(FString::Printf(TEXT("Stop requested: State=%s"),
        *StaticEnum<EBertaBlackEyeRevealState>()->GetNameStringByValue(static_cast<int64>(RevealState))));
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
        BERTA_REVEAL_TRACE(TEXT("TargetCamera destroyed"));
        ActiveTargetCamera.Reset();
        if (RevealState == EBertaBlackEyeRevealState::BlendingOut)
        {
            ClearPhaseTimer();
            if (APlayerController* PlayerController = ActivePlayerController.Get())
            {
                bool bUsedFallback = false;
                AActor* ReturnTarget = ResolveReturnViewTarget(PlayerController, bUsedFallback);
                PlayerController->SetViewTarget(ReturnTarget);
                BERTA_REVEAL_TRACE(FString::Printf(TEXT("TargetCamera lost during BlendOut; immediate return=%s Fallback=%s"),
                    *GetNameSafe(ReturnTarget), bUsedFallback ? TEXT("true") : TEXT("false")));
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
    if (DestroyedActor == ActivePlayerController.Get(true))
    {
        BERTA_REVEAL_TRACE(TEXT("Controller destroyed"));
        StopCameraReveal();
    }
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
    BERTA_REVEAL_TRACE(TEXT("Input locks removed"));
    const int32 ResumedCount = ResumeNotifiedParticipants();
    BERTA_REVEAL_TRACE(FString::Printf(TEXT("Participants resumed: %d"), ResumedCount));
    (void)ResumedCount;
    if (bGameplayLockApplied)
    {
        bGameplayLockApplied = false;
        RemoveGameplayLock();
        BERTA_REVEAL_TRACE(TEXT("Gameplay lock removed"));
    }

    ActivePlayerController.Reset();
    ActiveTargetCamera.Reset();
    SavedViewTarget.Reset();
    SavedPawn.Reset();
    ActiveExplicitReturnTarget.Reset();
    RevealState = EBertaBlackEyeRevealState::Idle;
    BERTA_REVEAL_TRACE(TEXT("Finished"));
    if (!bEndingPlay)
    {
        OnCinematicFinished();
        if (!bEndingPlay) OnRevealFinished.Broadcast();
    }
    bFinishing = false;
}

void UBertaBlackEyeCameraRevealComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    BERTA_REVEAL_TRACE(FString::Printf(TEXT("EndPlay cleanup requested: Reason=%d"), static_cast<int32>(EndPlayReason)));
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
        // Once our own BlendOut has started, the pending target is naturally no longer the reveal camera.
        if (RevealState == EBertaBlackEyeRevealState::BlendingOut ||
            ActiveExternalCameraChangePolicy != EBertaBlackEyeExternalCameraChangePolicy::RespectExternalChange ||
            !ActiveTargetCamera.IsValid() || IsRevealStillControllingViewTarget())
        {
            bool bUsedFallback = false;
            AActor* ReturnTarget = ResolveReturnViewTarget(PlayerController, bUsedFallback);
            PlayerController->SetViewTarget(ReturnTarget);
            RestoreControlRotation();
            BERTA_REVEAL_TRACE(FString::Printf(TEXT("EndPlay immediate return=%s Fallback=%s; ControlRotation restored"),
                *GetNameSafe(ReturnTarget), bUsedFallback ? TEXT("true") : TEXT("false")));
        }
        else BERTA_REVEAL_TRACE(TEXT("EndPlay external ViewTarget takeover respected; camera and ControlRotation left intact"));
    }

    RemoveConfiguredInputLocks();
    BERTA_REVEAL_TRACE(TEXT("EndPlay input locks removed"));
    const int32 ResumedCount = ResumeNotifiedParticipants();
    BERTA_REVEAL_TRACE(FString::Printf(TEXT("EndPlay participants resumed: %d"), ResumedCount));
    (void)ResumedCount;
    if (bGameplayLockApplied)
    {
        bGameplayLockApplied = false;
        RemoveGameplayLock();
        BERTA_REVEAL_TRACE(TEXT("EndPlay gameplay lock removed"));
    }
    ActivePlayerController.Reset();
    ActiveTargetCamera.Reset();
    SavedViewTarget.Reset();
    SavedPawn.Reset();
    ActiveExplicitReturnTarget.Reset();
    RevealState = EBertaBlackEyeRevealState::Idle;
    BERTA_REVEAL_TRACE(TEXT("EndPlay cleanup finished"));
    Super::EndPlay(EndPlayReason);
}

void UBertaBlackEyeCameraRevealComponent::ApplyGameplayLock_Implementation() {}
void UBertaBlackEyeCameraRevealComponent::RemoveGameplayLock_Implementation() {}
void UBertaBlackEyeCameraRevealComponent::OnCinematicStarted_Implementation() {}
void UBertaBlackEyeCameraRevealComponent::OnCinematicEnding_Implementation() {}
void UBertaBlackEyeCameraRevealComponent::OnCinematicFinished_Implementation() {}

#undef BERTA_REVEAL_TRACE
