#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Input/BertaComboGraphInputBufferComponent.h"

#include "BertaComboGraphTraceComponent.generated.h"

class UAnimMontage;
class UComboGraph;
class UComboGraphAbilityTask_StartGraph;
class UComboGraphNodeAnimBase;

UENUM(BlueprintType)
enum class EBertaComboGraphObservationKind : uint8
{
	Direct,
	Observed,
	ResolvedFromTopology,
	Inferred,
	Unknown
};

UENUM(BlueprintType)
enum class EBertaComboGraphTraceEventType : uint8
{
	GraphStarted,
	GraphEnded,
	NodeEntered,
	NodeExited,
	QueuedNodeChanged,
	ComboWindowOpened,
	ComboWindowClosed,
	GameplayEvent,
	TransitionObserved,
	MontageChanged,
	Warning,
	InputBuffered,
	InputBufferConsumed,
	InputBufferExpired,
	InputBufferRejected,
	InputBufferCleared
};

USTRUCT(BlueprintType)
struct BERTACOMBOGRAPHEXT_API FBertaComboGraphTraceEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Trace") float RelativeTimeSeconds = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") EBertaComboGraphTraceEventType Type = EBertaComboGraphTraceEventType::GraphStarted;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") EBertaComboGraphObservationKind Observation = EBertaComboGraphObservationKind::Direct;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") bool bInferred = false;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") FString ExecutionId;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") FString GraphPath;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") FString PreviousNodePath;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") FString CurrentNodePath;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") FString QueuedNodePath;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") FString ResolvedEdgePath;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") FString MontagePath;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") FGameplayTag GameplayEventTag;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") FString InstigatorPath;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") FString TargetPath;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") int32 TargetDataCount = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") bool bHasHitResult = false;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") float EventMagnitude = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Trace") FString Message;
};

USTRUCT(BlueprintType)
struct BERTACOMBOGRAPHEXT_API FBertaComboGraphStateSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Snapshot") FString ActorPath;
	UPROPERTY(BlueprintReadOnly, Category = "Snapshot") FString GraphPath;
	UPROPERTY(BlueprintReadOnly, Category = "Snapshot") FString TaskPath;
	UPROPERTY(BlueprintReadOnly, Category = "Snapshot") FString CurrentNodePath;
	UPROPERTY(BlueprintReadOnly, Category = "Snapshot") FString PreviousNodePath;
	UPROPERTY(BlueprintReadOnly, Category = "Snapshot") FString QueuedNodePath;
	UPROPERTY(BlueprintReadOnly, Category = "Snapshot") FString MontagePath;
	UPROPERTY(BlueprintReadOnly, Category = "Snapshot") bool bComboWindowOpen = false;
	UPROPERTY(BlueprintReadOnly, Category = "Snapshot") bool bTransitionAppearsQueued = false;
	UPROPERTY(BlueprintReadOnly, Category = "Snapshot") float RuntimeTimestampSeconds = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Snapshot") int32 ActiveGraphCount = 0;
};

/** Opt-in event-correlated Combo Graph trace. It samples only while executions are active. */
UCLASS(ClassGroup = (BertaComboGraphExt), meta = (BlueprintSpawnableComponent))
class BERTACOMBOGRAPHEXT_API UBertaComboGraphTraceComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UBertaComboGraphTraceComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaComboGraphExt|Diagnostics", meta = (ClampMin = "1", ClampMax = "10000"))
	int32 MaximumEventCount = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaComboGraphExt|Diagnostics") bool bLogEvents = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaComboGraphExt|Diagnostics") bool bIncludeInferredEvents = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaComboGraphExt|Diagnostics", meta = (ClampMin = "0.01", ClampMax = "1.0")) float ActiveSamplingIntervalSeconds = 0.025f;

	UFUNCTION(BlueprintCallable, Category = "BertaComboGraphExt|Diagnostics") void StartTracing();
	UFUNCTION(BlueprintCallable, Category = "BertaComboGraphExt|Diagnostics") void StopTracing();
	UFUNCTION(BlueprintCallable, Category = "BertaComboGraphExt|Diagnostics") void ClearTrace();
	UFUNCTION(BlueprintPure, Category = "BertaComboGraphExt|Diagnostics") bool IsTracing() const { return bTracing; }
	UFUNCTION(BlueprintPure, Category = "BertaComboGraphExt|Diagnostics") TArray<FBertaComboGraphTraceEvent> GetTraceEvents() const { return Events; }
	UFUNCTION(BlueprintPure, Category = "BertaComboGraphExt|Diagnostics") TArray<FBertaComboGraphStateSnapshot> GetStateSnapshots() const;
	UFUNCTION(BlueprintPure, Category = "BertaComboGraphExt|Diagnostics") FString DumpTraceToText() const;

	static FString FormatTraceEvent(const FBertaComboGraphTraceEvent& Event);
	static FString FormatStateSnapshot(const FBertaComboGraphStateSnapshot& Snapshot);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;

private:
	struct FExecutionRecord
	{
		TWeakObjectPtr<UComboGraphAbilityTask_StartGraph> Task;
		TWeakObjectPtr<UComboGraph> Graph;
		TWeakObjectPtr<UComboGraphNodeAnimBase> CurrentNode;
		TWeakObjectPtr<UComboGraphNodeAnimBase> PreviousNode;
		TWeakObjectPtr<UComboGraphNodeAnimBase> QueuedNode;
		TWeakObjectPtr<UAnimMontage> Montage;
		bool bWindowOpen = false;
	};

	UPROPERTY(Transient) TArray<FBertaComboGraphTraceEvent> Events;
	UPROPERTY(Transient) TObjectPtr<UBertaComboGraphInputBufferComponent> BoundInputBuffer = nullptr;
	TArray<FExecutionRecord> Executions;
	FDelegateHandle StartedHandle;
	FDelegateHandle EndedHandle;
	FTimerHandle SamplingTimer;
	float TraceStartTime = 0.0f;
	bool bTracing = false;

	void HandleGraphStarted(const UComboGraphAbilityTask_StartGraph& Task, const UComboGraph& Graph);
	void HandleGraphEnded(const UComboGraphAbilityTask_StartGraph& Task, const UComboGraph& Graph);
	UFUNCTION() void HandleGameplayEvent(FGameplayTag EventTag, FGameplayEventData EventData);
	UFUNCTION() void HandleInputBufferDiagnostic(const FBertaComboGraphInputBufferEvent& Event);
	void SampleExecutions();
	void SampleExecution(FExecutionRecord& Record);
	void RebindGameplayEventDelegate();
	void AddEvent(FBertaComboGraphTraceEvent&& Event);
	FBertaComboGraphTraceEvent MakeEvent(const FExecutionRecord& Record, EBertaComboGraphTraceEventType Type, EBertaComboGraphObservationKind Observation) const;
	bool IsTaskForOwner(const UComboGraphAbilityTask_StartGraph& Task) const;
	void UpdateSamplingTimer();
	static FString ExecutionId(const UComboGraphAbilityTask_StartGraph* Task, const UComboGraph* Graph);
};
