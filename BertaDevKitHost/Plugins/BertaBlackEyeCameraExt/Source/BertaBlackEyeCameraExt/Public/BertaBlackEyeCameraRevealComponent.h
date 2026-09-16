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

    /** Black Eye cine camera in this component's world; a missing target rejects Start. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Camera")
    TObjectPtr<ABlackEyeCineCameraActorBase> TargetCamera;

    /** Optional shared timing/input settings; copied at Start without changing the target camera. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Reveal")
    TObjectPtr<UBertaBlackEyeRevealPreset> RevealPreset;

    /** Used only when no valid RevealPreset is assigned; copied at Start. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Reveal", meta = (EditCondition = "RevealPreset == nullptr", EditConditionHides))
    FBertaBlackEyeRevealSettings InlineSettings;

    /** Timed exits after HoldTime; Manual waits for StopCameraReveal. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Reveal")
    EBertaBlackEyeRevealDurationMode DurationMode = EBertaBlackEyeRevealDurationMode::Timed;

    /** Snapshotted per session; fallback is resolved at exit if the requested actor disappeared. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Return")
    EBertaBlackEyeReturnTargetPolicy ReturnTargetPolicy = EBertaBlackEyeReturnTargetPolicy::CapturedViewTarget;

    /** Any Actor can be a UE ViewTarget; only used for ExplicitTarget and weakly snapshotted at Start. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Return",
        meta = (EditCondition = "ReturnTargetPolicy == EBertaBlackEyeReturnTargetPolicy::ExplicitTarget", EditConditionHides))
    TObjectPtr<AActor> ExplicitReturnTarget;

    /** RespectExternalChange checks the ViewTarget at exit, without continuous monitoring. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Return")
    EBertaBlackEyeExternalCameraChangePolicy ExternalCameraChangePolicy =
        EBertaBlackEyeExternalCameraChangePolicy::RestoreConfiguredTarget;

    /** Send pause/resume only to valid interface actors that this session actually notified. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Participants")
    bool bNotifyParticipants = false;

    /** Configured level actor references; duplicates are notified only once per session. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Berta Black Eye Camera|Participants", meta = (EditCondition = "bNotifyParticipants"))
    TArray<TObjectPtr<AActor>> Participants;

    /** Fired after session acceptance and gameplay hooks, before requesting BlendIn. */
    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealStarted;

    /** Blend-in duration elapsed; visual convergence is not measured. */
    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealCameraReached;

    /** Fired on exit before camera ownership is checked and any return is requested. */
    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealEnding;

    /** Fired after owned cleanup for any finished session except EndPlay. */
    UPROPERTY(BlueprintAssignable, Category = "Berta Black Eye Camera|Events")
    FBertaCameraRevealEvent OnRevealFinished;

    /** Start for a usable local controller in this world; false means no session was accepted. */
    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Reveal")
    bool StartCameraReveal();

    /** C++ entry point for a specific local controller and Timed/Manual mode; false means no session was accepted. */
    bool StartCameraRevealForController(APlayerController* PlayerController,
        EBertaBlackEyeRevealDurationMode RequestedMode);

    /** Request a controlled exit from BlendIn, Hold, or Manual Active; no hard cancellation. */
    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Reveal")
    void StopCameraReveal();

    /** Alias of StopCameraReveal; does not clear trigger one-shot state. */
    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Reveal")
    void ResetCameraReveal();

    /** True in every phase except Idle, including the return blend. */
    UFUNCTION(BlueprintPure, Category = "Berta Black Eye Camera|Reveal")
    bool IsRevealActive() const;

    /** Observable lifecycle phase; CameraReached reflects elapsed BlendIn time, not measured convergence. */
    UFUNCTION(BlueprintPure, Category = "Berta Black Eye Camera|Reveal")
    EBertaBlackEyeRevealState GetRevealState() const { return RevealState; }

    /** C++ diagnostics for the active session; captured actors are weak and may disappear. */
    APlayerController* GetActivePlayerController() const { return ActivePlayerController.Get(); }
    ABlackEyeCineCameraActorBase* GetActiveTargetCamera() const { return ActiveTargetCamera.Get(); }
    AActor* GetSavedViewTarget() const { return SavedViewTarget.Get(); }
    FRotator GetSavedControlRotation() const { return SavedControlRotation; }
    const FBertaBlackEyeRevealSettings& GetActiveSettings() const { return ActiveSettings; }
    EBertaBlackEyeReturnTargetPolicy GetActiveReturnTargetPolicy() const { return ActiveReturnTargetPolicy; }
    EBertaBlackEyeExternalCameraChangePolicy GetActiveExternalCameraChangePolicy() const { return ActiveExternalCameraChangePolicy; }
    AActor* GetActiveExplicitReturnTarget() const { return ActiveExplicitReturnTarget.Get(); }
    /** Compare with the current/pending ViewTarget before Berta requests its own return blend. */
    bool IsRevealStillControllingViewTarget() const;
    /** Remaining time on the current phase timer, or zero when there is none. */
    float GetPhaseTimeRemaining() const;
#if !UE_BUILD_SHIPPING
    /** Non-Shipping metadata for on-demand replay selection in the current world. */
    double GetLastSuccessfulRevealStartRealTime() const { return LastSuccessfulRevealStartRealTime; }
    APlayerController* GetLastRevealController() const { return LastRevealController.Get(); }
    EBertaBlackEyeRevealDurationMode GetLastRevealStartMode() const { return LastRevealStartMode; }
#endif

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Blueprint gameplay extension hooks; the C++ input locks are applied/removed independently. */
    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void ApplyGameplayLock();
    virtual void ApplyGameplayLock_Implementation();

    /** Release the project's contribution previously added by ApplyGameplayLock. */
    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void RemoveGameplayLock();
    virtual void RemoveGameplayLock_Implementation();

    /** Session start hook before OnRevealStarted and the BlendIn request. */
    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void OnCinematicStarted();
    virtual void OnCinematicStarted_Implementation();

    /** Exit hook before OnRevealEnding and the camera ownership/return decision. */
    UFUNCTION(BlueprintNativeEvent, Category = "Berta Black Eye Camera|Events")
    void OnCinematicEnding();
    virtual void OnCinematicEnding_Implementation();

    /** Finished hook after owned cleanup and before OnRevealFinished. */
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
