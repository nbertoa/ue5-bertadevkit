#pragma once

#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

#include "BertaBlackEyeCameraTrigger.generated.h"

class ABlackEyeCineCameraActorBase;
class APlayerController;
class UBoxComponent;
class UInputComponent;
class UPrimitiveComponent;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBertaCameraRevealEvent);

/** Temporarily reveals a Black Eye camera, then returns to the player's previous view target. */
UCLASS(Blueprintable)
class BERTABLACKEYECAMERAEXT_API ABertaBlackEyeCameraTrigger : public AActor
{
    GENERATED_BODY()

public:
    ABertaBlackEyeCameraTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Berta Black Eye Camera|Trigger")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Berta Black Eye Camera|Trigger")
    TObjectPtr<UBoxComponent> BoxCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Camera")
    TObjectPtr<ABlackEyeCineCameraActorBase> TargetCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendInTime = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float HoldTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendOutTime = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Timing")
    TEnumAsByte<EViewTargetBlendFunction> BlendInFunction = VTBlend_Linear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Timing")
    TEnumAsByte<EViewTargetBlendFunction> BlendOutFunction = VTBlend_Linear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendInExponent = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Timing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BlendOutExponent = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Timing")
    bool bLockOutgoingOnBlendIn = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Timing")
    bool bLockOutgoingOnBlendOut = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Trigger")
    bool bTriggerOnce = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Trigger")
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Input")
    bool bDisablePlayerInput = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Input")
    bool bDisableMoveInput = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Input")
    bool bDisableLookInput = false;

    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealStarted;

    /** The requested blend-in duration has elapsed; visual convergence is not measured. */
    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealCameraReached;

    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealEnding;

    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealFinished;

    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Trigger")
    void StartCameraReveal();

    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Trigger")
    void StopCameraReveal();

    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Trigger")
    void ResetCameraTrigger();

    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Trigger")
    void SetTriggerEnabled(bool bNewEnabled);

    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Trigger")
    bool CanActorTrigger(AActor* Actor) const;
    virtual bool CanActorTrigger_Implementation(AActor* Actor) const;

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Extension hooks do not own the generic C++ input locks. */
    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void ApplyGameplayLock();
    virtual void ApplyGameplayLock_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void RemoveGameplayLock();
    virtual void RemoveGameplayLock_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void OnCinematicStarted();
    virtual void OnCinematicStarted_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void OnCinematicEnding();
    virtual void OnCinematicEnding_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void OnCinematicFinished();
    virtual void OnCinematicFinished_Implementation();

private:
    enum class ERevealState : uint8
    {
        Idle,
        BlendingIn,
        Holding,
        BlendingOut,
        Finished
    };

    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    APlayerController* ResolvePlayerController() const;
    bool TryStartCameraReveal(APlayerController* PlayerController);
    bool ApplyConfiguredPlayerInputLocks(APlayerController* PlayerController);
    void RemoveConfiguredPlayerInputLocks();
    void BeginBlendIn();
    void HandleBlendInFinished(uint32 ExpectedSequence);
    void BeginHold();
    void HandleHoldFinished(uint32 ExpectedSequence);
    void BeginBlendOut();
    void HandleBlendOutFinished(uint32 ExpectedSequence);
    AActor* ResolveRestoreViewTarget(APlayerController* PlayerController) const;
    void RestoreControlRotation();
    void FinishReveal();
    void ClearPhaseTimer();

    ERevealState RevealState = ERevealState::Idle;
    FTimerHandle PhaseTimer;
    uint32 SequenceSerial = 0;
    bool bHasTriggered = false;
    bool bResetAfterFinish = false;
    bool bMoveLockAdded = false;
    bool bLookLockAdded = false;
    bool bBlockerPushed = false;
    bool bGameplayLockApplied = false;
    FRotator SavedControlRotation = FRotator::ZeroRotator;
    TWeakObjectPtr<AActor> SavedViewTarget;
    TWeakObjectPtr<APlayerController> ActivePlayerController;

    UPROPERTY(Transient)
    TObjectPtr<UInputComponent> InputBlocker;
};
