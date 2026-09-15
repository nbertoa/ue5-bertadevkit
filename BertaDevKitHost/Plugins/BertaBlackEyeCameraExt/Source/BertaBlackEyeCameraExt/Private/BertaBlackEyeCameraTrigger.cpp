#include "BertaBlackEyeCameraTrigger.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

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
    BoxCollision->OnComponentEndOverlap.AddDynamic(this, &ThisClass::HandleEndOverlap);

    RevealComponent = CreateDefaultSubobject<UBertaBlackEyeCameraRevealComponent>(TEXT("RevealComponent"));
    RevealComponent->OnRevealStarted.AddDynamic(this, &ThisClass::HandleComponentStarted);
    RevealComponent->OnRevealCameraReached.AddDynamic(this, &ThisClass::HandleComponentCameraReached);
    RevealComponent->OnRevealEnding.AddDynamic(this, &ThisClass::HandleComponentEnding);
    RevealComponent->OnRevealFinished.AddDynamic(this, &ThisClass::HandleComponentFinished);
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
    if (!IsValid(OtherActor) || !bEnabled || EndMode == EBertaBlackEyeRevealEndMode::Manual ||
        RevealComponent->IsRevealActive() || (bTriggerOnce && bHasTriggered) || !CanActorTrigger(OtherActor)) return;

    APawn* Pawn = Cast<APawn>(OtherActor);
    APlayerController* PlayerController = Pawn ? Cast<APlayerController>(Pawn->GetController()) : ResolveLocalPlayerController();
    if (!IsValid(PlayerController) || !PlayerController->IsLocalPlayerController()) return;

    const EBertaBlackEyeRevealDurationMode RequestedMode = EndMode == EBertaBlackEyeRevealEndMode::Timed
        ? EBertaBlackEyeRevealDurationMode::Timed : EBertaBlackEyeRevealDurationMode::Manual;
    if (EndMode == EBertaBlackEyeRevealEndMode::OnEndOverlap)
    {
        OverlapOwnerActor = OtherActor;
        OverlapOwnerController = PlayerController;
        OverlapActorForDestroyCallback = OtherActor;
        OtherActor->OnDestroyed.AddDynamic(this, &ThisClass::HandleOverlapOwnerDestroyed);
    }
    if (!StartForController(PlayerController, RequestedMode))
    {
        ClearOverlapOwner();
        return;
    }
    if (!RevealComponent->IsRevealActive()) ClearOverlapOwner();
}

void ABertaBlackEyeCameraTrigger::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (EndMode == EBertaBlackEyeRevealEndMode::OnEndOverlap && OtherActor == OverlapActorForDestroyCallback &&
        !BoxCollision->IsOverlappingActor(OtherActor) &&
        OverlapOwnerController.Get() == RevealComponent->GetActivePlayerController())
    {
        ClearOverlapOwner();
        RevealComponent->StopCameraReveal();
    }
}

void ABertaBlackEyeCameraTrigger::HandleOverlapOwnerDestroyed(AActor* DestroyedActor)
{
    if (DestroyedActor == OverlapActorForDestroyCallback)
    {
        ClearOverlapOwner();
        RevealComponent->StopCameraReveal();
    }
}

APlayerController* ABertaBlackEyeCameraTrigger::ResolveLocalPlayerController() const
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

bool ABertaBlackEyeCameraTrigger::StartForController(APlayerController* PlayerController,
    EBertaBlackEyeRevealDurationMode RequestedMode)
{
    bStartInProgress = true;
    const bool bStarted = RevealComponent->StartCameraRevealForController(PlayerController, RequestedMode);
    bStartInProgress = false;
    if (bStarted) bHasTriggered = !bResetRequestedDuringStart;
    bResetRequestedDuringStart = false;
    return bStarted;
}

void ABertaBlackEyeCameraTrigger::StartCameraReveal()
{
    if (!bEnabled || RevealComponent->IsRevealActive() || (bTriggerOnce && bHasTriggered)) return;
    APlayerController* PlayerController = ResolveLocalPlayerController();
    if (!IsValid(PlayerController)) return;

    const EBertaBlackEyeRevealDurationMode RequestedMode = EndMode == EBertaBlackEyeRevealEndMode::Timed
        ? EBertaBlackEyeRevealDurationMode::Timed : EBertaBlackEyeRevealDurationMode::Manual;
    StartForController(PlayerController, RequestedMode);
}

void ABertaBlackEyeCameraTrigger::StopCameraReveal()
{
    ClearOverlapOwner();
    RevealComponent->StopCameraReveal();
}

void ABertaBlackEyeCameraTrigger::ResetCameraTrigger()
{
    if (bStartInProgress) bResetRequestedDuringStart = true;
    if (RevealComponent->IsRevealActive())
    {
        bResetAfterFinish = true;
        StopCameraReveal();
    }
    else
    {
        bHasTriggered = false;
        bResetAfterFinish = false;
    }
}

void ABertaBlackEyeCameraTrigger::SetTriggerEnabled(bool bNewEnabled)
{
    bEnabled = bNewEnabled;
    if (!bEnabled) StopCameraReveal();
}

void ABertaBlackEyeCameraTrigger::ClearOverlapOwner()
{
    if (OverlapOwnerActor.IsValid())
    {
        OverlapOwnerActor->OnDestroyed.RemoveDynamic(this, &ThisClass::HandleOverlapOwnerDestroyed);
    }
    OverlapOwnerActor.Reset();
    OverlapOwnerController.Reset();
    OverlapActorForDestroyCallback = nullptr;
}

void ABertaBlackEyeCameraTrigger::HandleComponentStarted()
{
    bActorGameplayLockApplied = true;
    ApplyGameplayLock();
    if (RevealComponent->GetRevealState() != EBertaBlackEyeRevealState::BlendingIn) return;
    OnCinematicStarted();
    if (RevealComponent->GetRevealState() == EBertaBlackEyeRevealState::BlendingIn) OnRevealStarted.Broadcast();
}

void ABertaBlackEyeCameraTrigger::HandleComponentCameraReached()
{
    OnRevealCameraReached.Broadcast();
}

void ABertaBlackEyeCameraTrigger::HandleComponentEnding()
{
    OnCinematicEnding();
    if (!IsActorBeingDestroyed()) OnRevealEnding.Broadcast();
}

void ABertaBlackEyeCameraTrigger::HandleComponentFinished()
{
    ClearOverlapOwner();
    if (bActorGameplayLockApplied)
    {
        bActorGameplayLockApplied = false;
        RemoveGameplayLock();
    }
    if (IsActorBeingDestroyed()) return;
    if (bResetAfterFinish)
    {
        bHasTriggered = false;
        bResetAfterFinish = false;
    }
    OnCinematicFinished();
    if (!IsActorBeingDestroyed()) OnRevealFinished.Broadcast();
}

void ABertaBlackEyeCameraTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearOverlapOwner();
    RevealComponent->OnRevealStarted.RemoveDynamic(this, &ThisClass::HandleComponentStarted);
    RevealComponent->OnRevealCameraReached.RemoveDynamic(this, &ThisClass::HandleComponentCameraReached);
    RevealComponent->OnRevealEnding.RemoveDynamic(this, &ThisClass::HandleComponentEnding);
    RevealComponent->OnRevealFinished.RemoveDynamic(this, &ThisClass::HandleComponentFinished);
    if (bActorGameplayLockApplied)
    {
        bActorGameplayLockApplied = false;
        RemoveGameplayLock();
    }
    Super::EndPlay(EndPlayReason);
}

void ABertaBlackEyeCameraTrigger::ApplyGameplayLock_Implementation() {}
void ABertaBlackEyeCameraTrigger::RemoveGameplayLock_Implementation() {}
void ABertaBlackEyeCameraTrigger::OnCinematicStarted_Implementation() {}
void ABertaBlackEyeCameraTrigger::OnCinematicEnding_Implementation() {}
void ABertaBlackEyeCameraTrigger::OnCinematicFinished_Implementation() {}
