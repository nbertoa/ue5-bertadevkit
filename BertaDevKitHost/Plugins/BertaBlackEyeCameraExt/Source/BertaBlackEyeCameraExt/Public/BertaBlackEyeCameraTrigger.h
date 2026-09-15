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

    /** Configure TargetCamera, preset, timing, input and participants on this component. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Berta Black Eye Camera|Reveal")
    TObjectPtr<UBertaBlackEyeCameraRevealComponent> RevealComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Trigger")
    EBertaBlackEyeRevealEndMode EndMode = EBertaBlackEyeRevealEndMode::Timed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Trigger")
    bool bTriggerOnce = true;

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

    /** Actor hooks are invoked through the component's forwarded lifecycle. */
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
