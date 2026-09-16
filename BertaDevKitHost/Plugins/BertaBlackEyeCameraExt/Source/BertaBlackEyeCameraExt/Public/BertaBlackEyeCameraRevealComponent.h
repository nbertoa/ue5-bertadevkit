#pragma once

#include "BertaBlackEyeRevealSettings.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "BertaBlackEyeCameraRevealComponent.generated.h"

class ABlackEyeCineCameraActorBase;
class APlayerController;
class APawn;
class UBertaBlackEyeRevealPreset;
class UInputComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBertaCameraRevealEvent);

/** Owns one local camera reveal session, independent of collision or gameplay systems. */
UCLASS(ClassGroup = "Berta Black Eye Camera", Blueprintable, meta = (BlueprintSpawnableComponent))
class BERTABLACKEYECAMERAEXT_API UBertaBlackEyeCameraRevealComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBertaBlackEyeCameraRevealComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Camera")
    TObjectPtr<ABlackEyeCineCameraActorBase> TargetCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Reveal")
    TObjectPtr<UBertaBlackEyeRevealPreset> RevealPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Reveal", meta = (EditCondition = "RevealPreset == nullptr", EditConditionHides))
    FBertaBlackEyeRevealSettings InlineSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Reveal")
    EBertaBlackEyeRevealDurationMode DurationMode = EBertaBlackEyeRevealDurationMode::Timed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Return")
    EBertaBlackEyeReturnTargetPolicy ReturnTargetPolicy = EBertaBlackEyeReturnTargetPolicy::CapturedViewTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Return",
        meta = (EditCondition = "ReturnTargetPolicy == EBertaBlackEyeReturnTargetPolicy::ExplicitTarget", EditConditionHides))
    TObjectPtr<AActor> ExplicitReturnTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Return")
    EBertaBlackEyeExternalCameraChangePolicy ExternalCameraChangePolicy =
        EBertaBlackEyeExternalCameraChangePolicy::RestoreConfiguredTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Participants")
    bool bNotifyParticipants = false;

    /** Configured level actor references; duplicates are notified only once per session. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Participants", meta = (EditCondition = "bNotifyParticipants"))
    TArray<TObjectPtr<AActor>> Participants;

    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealStarted;

    /** Blend-in duration elapsed; visual convergence is not measured. */
    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealCameraReached;

    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealEnding;

    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealFinished;

    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Reveal")
    bool StartCameraReveal();

    /** Captures the supplied local controller; useful when an overlapping pawn identifies the player. */
    bool StartCameraRevealForController(APlayerController* PlayerController,
        EBertaBlackEyeRevealDurationMode RequestedMode);

    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Reveal")
    void StopCameraReveal();

    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Reveal")
    void ResetCameraReveal();

    UFUNCTION(BlueprintPure, Category = "Berta Black Eye Camera|Reveal")
    bool IsRevealActive() const;

    UFUNCTION(BlueprintPure, Category = "Berta Black Eye Camera|Reveal")
    EBertaBlackEyeRevealState GetRevealState() const { return RevealState; }

    APlayerController* GetActivePlayerController() const { return ActivePlayerController.Get(); }
    ABlackEyeCineCameraActorBase* GetActiveTargetCamera() const { return ActiveTargetCamera.Get(); }
    AActor* GetSavedViewTarget() const { return SavedViewTarget.Get(); }
    FRotator GetSavedControlRotation() const { return SavedControlRotation; }
    const FBertaBlackEyeRevealSettings& GetActiveSettings() const { return ActiveSettings; }
    EBertaBlackEyeReturnTargetPolicy GetActiveReturnTargetPolicy() const { return ActiveReturnTargetPolicy; }
    EBertaBlackEyeExternalCameraChangePolicy GetActiveExternalCameraChangePolicy() const { return ActiveExternalCameraChangePolicy; }
    AActor* GetActiveExplicitReturnTarget() const { return ActiveExplicitReturnTarget.Get(); }
    bool IsRevealStillControllingViewTarget() const;
    float GetPhaseTimeRemaining() const;
#if !UE_BUILD_SHIPPING
    double GetLastSuccessfulRevealStartRealTime() const { return LastSuccessfulRevealStartRealTime; }
    APlayerController* GetLastRevealController() const { return LastRevealController.Get(); }
    EBertaBlackEyeRevealDurationMode GetLastRevealStartMode() const { return LastRevealStartMode; }
#endif

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** These extension hooks never own the C++ input locks. */
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
    void HandleTargetDestroyed(AActor* DestroyedActor);

    UFUNCTION()
    void HandleControllerDestroyed(AActor* DestroyedActor);

    APlayerController* ResolveLocalPlayerController() const;
    bool ApplyConfiguredInputLocks(APlayerController* PlayerController);
    void RemoveConfiguredInputLocks();
    void NotifyParticipantsPaused();
    int32 ResumeNotifiedParticipants();
    void UnbindSessionActors();
    void BeginBlendIn();
    void HandleBlendInFinished(uint32 ExpectedSerial);
    void BeginHoldOrActive();
    void HandleHoldFinished(uint32 ExpectedSerial);
    void BeginBlendOut();
    void HandleBlendOutFinished(uint32 ExpectedSerial);
    AActor* ResolveReturnViewTarget(APlayerController* PlayerController, bool& bUsedFallback) const;
    void RestoreControlRotation();
    void FinishReveal();
    void ClearPhaseTimer();
#if !UE_BUILD_SHIPPING
    void TraceRevealEvent(const FString& Event) const;
#endif

    EBertaBlackEyeRevealState RevealState = EBertaBlackEyeRevealState::Idle;
    EBertaBlackEyeRevealDurationMode ActiveDurationMode = EBertaBlackEyeRevealDurationMode::Timed;
    EBertaBlackEyeReturnTargetPolicy ActiveReturnTargetPolicy = EBertaBlackEyeReturnTargetPolicy::CapturedViewTarget;
    EBertaBlackEyeExternalCameraChangePolicy ActiveExternalCameraChangePolicy =
        EBertaBlackEyeExternalCameraChangePolicy::RestoreConfiguredTarget;
    TWeakObjectPtr<AActor> ActiveExplicitReturnTarget;
    FBertaBlackEyeRevealSettings ActiveSettings;
    FTimerHandle PhaseTimer;
    uint32 SessionSerial = 0;
#if !UE_BUILD_SHIPPING
    uint32 RevealSessionId = 0;
    double SessionStartRealTime = 0.0;
    double LastSuccessfulRevealStartRealTime = 0.0;
    TWeakObjectPtr<APlayerController> LastRevealController;
    EBertaBlackEyeRevealDurationMode LastRevealStartMode = EBertaBlackEyeRevealDurationMode::Timed;
#endif
    bool bMoveLockAdded = false;
    bool bLookLockAdded = false;
    bool bBlockerPushed = false;
    bool bGameplayLockApplied = false;
    bool bFinishing = false;
    bool bEndingPlay = false;
    FRotator SavedControlRotation = FRotator::ZeroRotator;
    TWeakObjectPtr<AActor> SavedViewTarget;
    TWeakObjectPtr<APawn> SavedPawn;
    TWeakObjectPtr<APlayerController> ActivePlayerController;
    TWeakObjectPtr<ABlackEyeCineCameraActorBase> ActiveTargetCamera;
    TArray<TWeakObjectPtr<AActor>> NotifiedParticipants;

    UPROPERTY(Transient)
    TObjectPtr<UInputComponent> InputBlocker;
};
