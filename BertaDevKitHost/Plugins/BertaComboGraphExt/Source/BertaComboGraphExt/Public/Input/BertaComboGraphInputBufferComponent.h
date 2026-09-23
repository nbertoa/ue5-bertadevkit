#pragma once

#include "Components/ActorComponent.h"
#include "InputAction.h"

#include "BertaComboGraphInputBufferComponent.generated.h"

class UComboGraph;
class UComboGraphAbilityTask_StartGraph;
class UComboGraphNodeAnimBase;
class UEnhancedInputComponent;

UENUM(BlueprintType)
enum class EBertaComboGraphInputBufferEventType : uint8
{
	Buffered,
	Expired,
	Consumed,
	Rejected,
	Cleared
};

USTRUCT(BlueprintType)
struct BERTACOMBOGRAPHEXT_API FBertaComboGraphInputBufferEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Input Buffer") EBertaComboGraphInputBufferEventType Type = EBertaComboGraphInputBufferEventType::Buffered;
	UPROPERTY(BlueprintReadOnly, Category = "Input Buffer") FString ExecutionId;
	UPROPERTY(BlueprintReadOnly, Category = "Input Buffer") FString InputActionPath;
	UPROPERTY(BlueprintReadOnly, Category = "Input Buffer") FString NodePath;
	UPROPERTY(BlueprintReadOnly, Category = "Input Buffer") float TimestampSeconds = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Input Buffer") FString Reason;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBertaComboGraphInputBufferDiagnosticDelegate, const FBertaComboGraphInputBufferEvent&, Event);

/** Local, finite, node-scoped pre-input buffer for Triggered transition edges. */
UCLASS(ClassGroup = (BertaComboGraphExt), meta = (BlueprintSpawnableComponent))
class BERTACOMBOGRAPHEXT_API UBertaComboGraphInputBufferComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UBertaComboGraphInputBufferComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaComboGraphExt|Input Buffer") bool bEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaComboGraphExt|Input Buffer", meta = (ClampMin = "0.01", ClampMax = "1.0")) float BufferDurationSeconds = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaComboGraphExt|Input Buffer") bool bConsumeOnComboWindowOpen = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaComboGraphExt|Input Buffer", meta = (ClampMin = "1", ClampMax = "8")) int32 MaximumBufferedInputs = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BertaComboGraphExt|Input Buffer", meta = (ClampMin = "0.01", ClampMax = "1.0")) float ActiveSamplingIntervalSeconds = 0.025f;
	UPROPERTY(BlueprintAssignable, Category = "BertaComboGraphExt|Input Buffer") FBertaComboGraphInputBufferDiagnosticDelegate OnInputBufferDiagnostic;

	UFUNCTION(BlueprintCallable, Category = "BertaComboGraphExt|Input Buffer") bool StartBuffering();
	UFUNCTION(BlueprintCallable, Category = "BertaComboGraphExt|Input Buffer") void StopBuffering();
	UFUNCTION(BlueprintCallable, Category = "BertaComboGraphExt|Input Buffer") void ClearBufferedInputs();
	UFUNCTION(BlueprintPure, Category = "BertaComboGraphExt|Input Buffer") bool IsBuffering() const { return bBuffering; }

	static bool CanConsumeWithStockInputPath(ETriggerEvent TriggerEvent);
	static bool IsExpired(float CapturedAt, float Now, float Duration);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;

private:
	struct FBufferedInput
	{
		TWeakObjectPtr<const UInputAction> Action;
		TWeakObjectPtr<UComboGraphNodeAnimBase> Node;
		float Timestamp = 0.0f;
	};

	struct FExecutionRecord
	{
		TWeakObjectPtr<UComboGraphAbilityTask_StartGraph> Task;
		TWeakObjectPtr<UComboGraph> Graph;
		TWeakObjectPtr<UComboGraphNodeAnimBase> Node;
		bool bWindowOpen = false;
		bool bConsumePending = false;
		TArray<FBufferedInput> Inputs;
	};

	TArray<FExecutionRecord> Executions;
	uint32 ExecutionsGeneration = 0;
	TArray<FBertaComboGraphInputBufferEvent> PendingDiagnostics;
	TWeakObjectPtr<UEnhancedInputComponent> BoundInputComponent;
	TArray<uint32> InputBindingHandles;
	FDelegateHandle StartedHandle;
	FDelegateHandle EndedHandle;
	FTimerHandle SamplingTimer;
	bool bBuffering = false;
	bool bDispatchingDiagnostics = false;

	void HandleGraphStarted(const UComboGraphAbilityTask_StartGraph& Task, const UComboGraph& Graph);
	void HandleGraphEnded(const UComboGraphAbilityTask_StartGraph& Task, const UComboGraph& Graph);
	void HandleTriggeredInput(const FInputActionInstance& Instance);
	void SampleExecutions();
	void RebuildBindings();
	void RemoveBindings();
	bool ConsumeBufferedInput(FExecutionRecord& Record, float Now);
	void ClearRecord(FExecutionRecord& Record, const TCHAR* Reason);
	FBertaComboGraphInputBufferEvent MakeDiagnostic(EBertaComboGraphInputBufferEventType Type, const FExecutionRecord* Record, const UInputAction* Action, const TCHAR* Reason) const;
	void QueueDiagnostic(EBertaComboGraphInputBufferEventType Type, const FExecutionRecord* Record, const UInputAction* Action, const TCHAR* Reason);
	void DispatchDiagnostics();
	bool IsTaskForOwner(const UComboGraphAbilityTask_StartGraph& Task) const;
	static bool NodeAcceptsTriggeredAction(const UComboGraphNodeAnimBase* Node, const UInputAction* Action);
	static FString ExecutionId(const FExecutionRecord* Record);
};
