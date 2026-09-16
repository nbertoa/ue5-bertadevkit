#pragma once

#include "BertaBlackEyeCameraRevealComponent.h"
#include "GameFramework/Actor.h"
#include "BertaBlackEyeCameraTrigger.generated.h"

class APlayerController;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;

/** Box activation policy and Blueprint façade for a reusable reveal component. */
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

    /** Configure camera, preset, return/input policies, and participants on this component. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Berta Black Eye Camera|Reveal")
    TObjectPtr<UBertaBlackEyeCameraRevealComponent> RevealComponent;

    /** Controls overlap activation/exit; distinct from RevealComponent's Timed/Manual duration mode. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Trigger")
    EBertaBlackEyeRevealEndMode EndMode = EBertaBlackEyeRevealEndMode::Timed;

    /** After a successful trigger-wrapper start, reject later starts until ResetCameraTrigger. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Trigger")
    bool bTriggerOnce = true;

    /** Gates trigger overlap/explicit Start; call SetTriggerEnabled(false) to stop an active reveal. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Trigger")
    bool bEnabled = true;

    /** Forwarded from RevealComponent; there is one lifecycle and one event source. */
    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealStarted;

    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealCameraReached;

    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealEnding;

    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealFinished;

    /** Start through the trigger's enabled/one-shot policy; overlaps do not start in Manual mode. */
    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Trigger")
    void StartCameraReveal();

    /** Stop its current reveal and clear any matching OnEndOverlap owner; does not reset one-shot use. */
    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Trigger")
    void StopCameraReveal();

    /** Request a controlled exit; clear one-shot use immediately if idle, otherwise after Finish. */
    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Trigger")
    void ResetCameraTrigger();

    /** Set bEnabled; false also requests StopCameraReveal for an active session. */
    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Trigger")
    void SetTriggerEnabled(bool bNewEnabled);

    /** Default accepts a locally controlled Pawn; override to filter overlap actors. */
    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Trigger")
    bool CanActorTrigger(AActor* Actor) const;
    virtual bool CanActorTrigger_Implementation(AActor* Actor) const;

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Actor hooks are invoked through the component's forwarded lifecycle. */
    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void ApplyGameplayLock();
    virtual void ApplyGameplayLock_Implementation();

    /** Release the trigger Blueprint's own gameplay contribution when its reveal finishes. */
    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void RemoveGameplayLock();
    virtual void RemoveGameplayLock_Implementation();

    /** Forwarded start hook before the trigger's OnRevealStarted delegate. */
    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void OnCinematicStarted();
    virtual void OnCinematicStarted_Implementation();

    /** Forwarded exit hook before the trigger's OnRevealEnding delegate. */
    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void OnCinematicEnding();
    virtual void OnCinematicEnding_Implementation();

    /** Forwarded finish hook after the trigger's gameplay contribution is removed. */
    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void OnCinematicFinished();
    virtual void OnCinematicFinished_Implementation();

private:
    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void HandleOverlapOwnerDestroyed(AActor* DestroyedActor);

    UFUNCTION()
    void HandleComponentStarted();

    UFUNCTION()
    void HandleComponentCameraReached();

    UFUNCTION()
    void HandleComponentEnding();

    UFUNCTION()
    void HandleComponentFinished();

    APlayerController* ResolveLocalPlayerController() const;
    bool StartForController(APlayerController* PlayerController, EBertaBlackEyeRevealDurationMode RequestedMode);
    void ClearOverlapOwner();

    bool bHasTriggered = false;
    bool bResetAfterFinish = false;
    bool bActorGameplayLockApplied = false;
    bool bStartInProgress = false;
    bool bResetRequestedDuringStart = false;
    TWeakObjectPtr<AActor> OverlapOwnerActor;
    TWeakObjectPtr<APlayerController> OverlapOwnerController;
    AActor* OverlapActorForDestroyCallback = nullptr;
};
