#include "BertaBlackEyeCameraTrigger.h"

#include "Actors/BlackEyeCineCameraActorBase.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaBlackEyeCameraExt, Log, All);

ABertaBlackEyeCameraTrigger::ABertaBlackEyeCameraTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    BoxCollision->SetupAttachment(Root);
    BoxCollision->SetCollisionProfileName(TEXT("Trigger"));
    BoxCollision->SetGenerateOverlapEvents(true);
    BoxCollision->InitBoxExtent(FVector(100.0f));
    BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleBeginOverlap);
}

bool ABertaBlackEyeCameraTrigger::CanActorTrigger_Implementation(AActor* Actor) const
{
    const APawn* Pawn = Cast<APawn>(Actor);
    const APlayerController* PlayerController = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    return IsValid(PlayerController) && PlayerController->IsLocalPlayerController();
}

void ABertaBlackEyeCameraTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bEnabled || RevealState != ERevealState::Idle || (bTriggerOnce && bHasTriggered) || !CanActorTrigger(OtherActor))
    {
        return;
    }

    APawn* Pawn = Cast<APawn>(OtherActor);
    APlayerController* PlayerController = Pawn ? Cast<APlayerController>(Pawn->GetController()) : ResolvePlayerController();
    if (IsValid(PlayerController) && PlayerController->IsLocalPlayerController())
    {
        TryStartCameraReveal(PlayerController);
    }
}

APlayerController* ABertaBlackEyeCameraTrigger::ResolvePlayerController() const
{
    const UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return nullptr;
    }

    // Manual start has no overlapping pawn to identify a player. Use the first local controller in this world.
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PlayerController = It->Get();
        if (IsValid(PlayerController) && PlayerController->IsLocalPlayerController())
        {
            return PlayerController;
        }
    }
    return nullptr;
}

void ABertaBlackEyeCameraTrigger::StartCameraReveal()
{
    if (!bEnabled || RevealState != ERevealState::Idle || (bTriggerOnce && bHasTriggered))
    {
        return;
    }

    APlayerController* PlayerController = ResolvePlayerController();
    if (!IsValid(PlayerController))
    {
        UE_LOG(LogBertaBlackEyeCameraExt, Warning, TEXT("Camera reveal could not start: no local PlayerController in this world."));
        return;
    }
    TryStartCameraReveal(PlayerController);
}

bool ABertaBlackEyeCameraTrigger::TryStartCameraReveal(APlayerController* PlayerController)
{
    if (!bEnabled || RevealState != ERevealState::Idle || (bTriggerOnce && bHasTriggered))
    {
        return false;
    }

    if (!IsValid(TargetCamera) || TargetCamera->GetWorld() != GetWorld())
    {
        UE_LOG(LogBertaBlackEyeCameraExt, Warning, TEXT("Camera reveal could not start: TargetCamera is missing, destroyed, or in another world."));
        return false;
    }
    if (!IsValid(PlayerController) || !PlayerController->IsLocalPlayerController() ||
        PlayerController->GetWorld() != GetWorld() || !IsValid(PlayerController->PlayerCameraManager))
    {
        UE_LOG(LogBertaBlackEyeCameraExt, Warning, TEXT("Camera reveal could not start: PlayerController has no usable local camera manager."));
        return false;
    }

    // GetViewTarget may return a pending destination if another blend is already in progress.
    SavedViewTarget = PlayerController->GetViewTarget();
    SavedControlRotation = PlayerController->GetControlRotation();
    ActivePlayerController = PlayerController;

    if (!ApplyConfiguredPlayerInputLocks(PlayerController))
    {
        RemoveConfiguredPlayerInputLocks();
        ActivePlayerController.Reset();
        SavedViewTarget.Reset();
        return false;
    }

    ++SequenceSerial;
    RevealState = ERevealState::BlendingIn;
    bHasTriggered = true;
    bResetAfterFinish = false;

    // Mark ownership before the hook so a Blueprint reentrant stop still removes its gameplay lock.
    bGameplayLockApplied = true;
    ApplyGameplayLock();
    if (RevealState != ERevealState::BlendingIn) return true;

    OnCinematicStarted();
    if (RevealState != ERevealState::BlendingIn) return true;

    OnRevealStarted.Broadcast();
    if (RevealState != ERevealState::BlendingIn) return true;

    BeginBlendIn();
    return true;
}

bool ABertaBlackEyeCameraTrigger::ApplyConfiguredPlayerInputLocks(APlayerController* PlayerController)
{
    if (bDisableMoveInput)
    {
        PlayerController->SetIgnoreMoveInput(true);
        bMoveLockAdded = true;
    }
    if (bDisableLookInput)
    {
        PlayerController->SetIgnoreLookInput(true);
        bLookLockAdded = true;
    }
    if (bDisablePlayerInput)
    {
        // PlayerInput walks components from highest priority down. A binding-free blocker stops both
        // legacy and Enhanced Input components below it without changing the pawn/controller enabled flags.
        InputBlocker = NewObject<UInputComponent>(this);
        if (!IsValid(InputBlocker))
        {
            UE_LOG(LogBertaBlackEyeCameraExt, Warning, TEXT("Camera reveal could not create its temporary input blocker."));
            return false;
        }
        InputBlocker->Priority = TNumericLimits<int32>::Max();
        InputBlocker->bBlockInput = true;
        InputBlocker->RegisterComponent();
        PlayerController->PushInputComponent(InputBlocker);
        bBlockerPushed = PlayerController->IsInputComponentInStack(InputBlocker);
        if (!bBlockerPushed)
        {
            UE_LOG(LogBertaBlackEyeCameraExt, Warning, TEXT("Camera reveal could not push its temporary input blocker."));
            return false;
        }
    }
    return true;
}

void ABertaBlackEyeCameraTrigger::RemoveConfiguredPlayerInputLocks()
{
    APlayerController* PlayerController = ActivePlayerController.Get();
    if (bBlockerPushed && IsValid(PlayerController) && IsValid(InputBlocker))
    {
        PlayerController->PopInputComponent(InputBlocker);
    }
    bBlockerPushed = false;
    if (IsValid(InputBlocker))
    {
        InputBlocker->DestroyComponent();
    }
    InputBlocker = nullptr;

    if (bMoveLockAdded && IsValid(PlayerController))
    {
        PlayerController->SetIgnoreMoveInput(false);
    }
    bMoveLockAdded = false;

    if (bLookLockAdded && IsValid(PlayerController))
    {
        PlayerController->SetIgnoreLookInput(false);
    }
    bLookLockAdded = false;
}

void ABertaBlackEyeCameraTrigger::BeginBlendIn()
{
    APlayerController* PlayerController = ActivePlayerController.Get();
    if (RevealState != ERevealState::BlendingIn) return;
    if (!IsValid(PlayerController))
    {
        FinishReveal();
        return;
    }
    if (!IsValid(TargetCamera))
    {
        BeginBlendOut();
        return;
    }

    PlayerController->SetViewTargetWithBlend(TargetCamera, BlendInTime, BlendInFunction.GetValue(),
        BlendInExponent, bLockOutgoingOnBlendIn);

    if (RevealState != ERevealState::BlendingIn) return;
    if (BlendInTime <= 0.0f)
    {
        HandleBlendInFinished(SequenceSerial);
    }
    else if (UWorld* World = GetWorld())
    {
        FTimerDelegate Callback = FTimerDelegate::CreateUObject(this, &ThisClass::HandleBlendInFinished, SequenceSerial);
        World->GetTimerManager().SetTimer(PhaseTimer, Callback, BlendInTime, false);
    }
    else
    {
        BeginBlendOut();
    }
}

void ABertaBlackEyeCameraTrigger::HandleBlendInFinished(uint32 ExpectedSequence)
{
    if (ExpectedSequence != SequenceSerial || RevealState != ERevealState::BlendingIn) return;
    ClearPhaseTimer();

    if (!ActivePlayerController.IsValid())
    {
        FinishReveal();
        return;
    }
    if (!IsValid(TargetCamera))
    {
        BeginBlendOut();
        return;
    }

    RevealState = ERevealState::Holding;
    OnRevealCameraReached.Broadcast();
    if (RevealState == ERevealState::Holding)
    {
        BeginHold();
    }
}

void ABertaBlackEyeCameraTrigger::BeginHold()
{
    if (HoldTime <= 0.0f)
    {
        BeginBlendOut();
    }
    else if (UWorld* World = GetWorld())
    {
        FTimerDelegate Callback = FTimerDelegate::CreateUObject(this, &ThisClass::HandleHoldFinished, SequenceSerial);
        World->GetTimerManager().SetTimer(PhaseTimer, Callback, HoldTime, false);
    }
    else
    {
        BeginBlendOut();
    }
}

void ABertaBlackEyeCameraTrigger::HandleHoldFinished(uint32 ExpectedSequence)
{
    if (ExpectedSequence != SequenceSerial || RevealState != ERevealState::Holding) return;
    BeginBlendOut();
}

AActor* ABertaBlackEyeCameraTrigger::ResolveRestoreViewTarget(APlayerController* PlayerController) const
{
    if (SavedViewTarget.IsValid()) return SavedViewTarget.Get();
    if (IsValid(PlayerController->GetPawn())) return PlayerController->GetPawn();
    return PlayerController;
}

void ABertaBlackEyeCameraTrigger::RestoreControlRotation()
{
    if (APlayerController* PlayerController = ActivePlayerController.Get())
    {
        PlayerController->SetControlRotation(SavedControlRotation);
    }
}

void ABertaBlackEyeCameraTrigger::BeginBlendOut()
{
    if (RevealState != ERevealState::BlendingIn && RevealState != ERevealState::Holding) return;
    ClearPhaseTimer();
    ++SequenceSerial;
    RevealState = ERevealState::BlendingOut;

    OnCinematicEnding();
    if (RevealState != ERevealState::BlendingOut) return;
    OnRevealEnding.Broadcast();
    if (RevealState != ERevealState::BlendingOut) return;

    APlayerController* PlayerController = ActivePlayerController.Get();
    if (!IsValid(PlayerController))
    {
        FinishReveal();
        return;
    }

    // Black Eye can write ControlRotation while it is viewed. Restore before the return blend
    // and once more after its duration, before releasing our look-input lock.
    RestoreControlRotation();
    if (!SavedViewTarget.IsValid())
    {
        UE_LOG(LogBertaBlackEyeCameraExt, Warning,
            TEXT("Saved ViewTarget was destroyed during camera reveal; returning to the captured controller's pawn or controller."));
    }
    AActor* ReturnTarget = ResolveRestoreViewTarget(PlayerController);
    UWorld* World = GetWorld();
    if (!World && BlendOutTime > 0.0f)
    {
        // Without a world timer, return immediately so input is not released mid-blend.
        PlayerController->SetViewTarget(ReturnTarget);
    }
    else
    {
        PlayerController->SetViewTargetWithBlend(ReturnTarget, BlendOutTime, BlendOutFunction.GetValue(),
            BlendOutExponent, bLockOutgoingOnBlendOut);
    }

    if (RevealState != ERevealState::BlendingOut) return;
    if (BlendOutTime <= 0.0f)
    {
        HandleBlendOutFinished(SequenceSerial);
    }
    else if (World)
    {
        FTimerDelegate Callback = FTimerDelegate::CreateUObject(this, &ThisClass::HandleBlendOutFinished, SequenceSerial);
        World->GetTimerManager().SetTimer(PhaseTimer, Callback, BlendOutTime, false);
    }
    else
    {
        HandleBlendOutFinished(SequenceSerial);
    }
}

void ABertaBlackEyeCameraTrigger::HandleBlendOutFinished(uint32 ExpectedSequence)
{
    if (ExpectedSequence != SequenceSerial || RevealState != ERevealState::BlendingOut) return;
    ClearPhaseTimer();
    RestoreControlRotation();
    FinishReveal();
}

void ABertaBlackEyeCameraTrigger::StopCameraReveal()
{
    if (RevealState == ERevealState::BlendingIn || RevealState == ERevealState::Holding)
    {
        BeginBlendOut();
    }
}

void ABertaBlackEyeCameraTrigger::ResetCameraTrigger()
{
    if (RevealState == ERevealState::BlendingIn || RevealState == ERevealState::Holding ||
        RevealState == ERevealState::BlendingOut)
    {
        bResetAfterFinish = true;
        StopCameraReveal();
    }
    else
    {
        bHasTriggered = false;
        bResetAfterFinish = false;
        RevealState = ERevealState::Idle;
    }
}

void ABertaBlackEyeCameraTrigger::SetTriggerEnabled(bool bNewEnabled)
{
    bEnabled = bNewEnabled;
    if (!bEnabled)
    {
        StopCameraReveal();
    }
}

void ABertaBlackEyeCameraTrigger::ClearPhaseTimer()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PhaseTimer);
    }
    PhaseTimer.Invalidate();
}

void ABertaBlackEyeCameraTrigger::FinishReveal()
{
    if (RevealState == ERevealState::Idle || RevealState == ERevealState::Finished) return;
    ClearPhaseTimer();
    ++SequenceSerial;

    // C++ locks are released independently of Blueprint parent-call behavior.
    RemoveConfiguredPlayerInputLocks();
    if (bGameplayLockApplied)
    {
        bGameplayLockApplied = false;
        RemoveGameplayLock();
    }

    ActivePlayerController.Reset();
    SavedViewTarget.Reset();
    if (bResetAfterFinish)
    {
        bHasTriggered = false;
        bResetAfterFinish = false;
    }
    RevealState = bTriggerOnce && bHasTriggered ? ERevealState::Finished : ERevealState::Idle;

    OnCinematicFinished();
    OnRevealFinished.Broadcast();
}

void ABertaBlackEyeCameraTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    const bool bWasActive = RevealState == ERevealState::BlendingIn || RevealState == ERevealState::Holding ||
        RevealState == ERevealState::BlendingOut;
    ClearPhaseTimer();
    ++SequenceSerial;

    UWorld* World = GetWorld();
    APlayerController* PlayerController = ActivePlayerController.Get();
    if (bWasActive && IsValid(PlayerController) && IsValid(World) && !World->bIsTearingDown &&
        (EndPlayReason == EEndPlayReason::Destroyed || EndPlayReason == EEndPlayReason::RemovedFromWorld))
    {
        // The trigger cannot remain alive for a timed return blend during destruction.
        PlayerController->SetViewTarget(ResolveRestoreViewTarget(PlayerController));
        RestoreControlRotation();
    }

    RemoveConfiguredPlayerInputLocks();
    if (bGameplayLockApplied)
    {
        bGameplayLockApplied = false;
        RemoveGameplayLock();
    }
    ActivePlayerController.Reset();
    SavedViewTarget.Reset();
    RevealState = ERevealState::Idle;
    Super::EndPlay(EndPlayReason);
}

void ABertaBlackEyeCameraTrigger::ApplyGameplayLock_Implementation() {}
void ABertaBlackEyeCameraTrigger::RemoveGameplayLock_Implementation() {}
void ABertaBlackEyeCameraTrigger::OnCinematicStarted_Implementation() {}
void ABertaBlackEyeCameraTrigger::OnCinematicEnding_Implementation() {}
void ABertaBlackEyeCameraTrigger::OnCinematicFinished_Implementation() {}
