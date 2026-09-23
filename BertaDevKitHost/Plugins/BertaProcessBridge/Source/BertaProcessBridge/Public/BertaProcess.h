#pragma once

#include "BertaProcessTypes.h"
#include "Templates/SharedPointer.h"
#include "Templates/UniquePtr.h"
#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"

#include "BertaProcess.generated.h"

class FBertaNativeProcessRunner;
class FBertaProcessCallbackDispatcher;
class UBertaProcess;
class UBertaProcessSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FBertaProcessOutputDelegate,
	UBertaProcess*, Process,
	FString, Output);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FBertaProcessFinishedDelegate,
	UBertaProcess*, Process,
	FBertaProcessResult, Result);

/** A single external process successfully launched by BertaProcessBridge. */
UCLASS(BlueprintType)
class BERTAPROCESSBRIDGE_API UBertaProcess : public UObject
{
	GENERATED_BODY()

public:
	virtual ~UBertaProcess() override;
	virtual void BeginDestroy() override;

	/** Text chunks read from the process's combined stdout and stderr stream. */
	UPROPERTY(BlueprintAssignable, Category = "BertaProcessBridge|Process")
	FBertaProcessOutputDelegate OnOutput;

	/** Broadcast exactly once after normal completion or explicit cancellation. */
	UPROPERTY(BlueprintAssignable, Category = "BertaProcessBridge|Process")
	FBertaProcessFinishedDelegate OnFinished;

	UFUNCTION(BlueprintPure, Category = "BertaProcessBridge|Process")
	bool IsRunning() const;

	UFUNCTION(BlueprintPure, Category = "BertaProcessBridge|Process")
	EBertaProcessState GetState() const;

	UFUNCTION(BlueprintPure, Category = "BertaProcessBridge|Process")
	double GetDurationSeconds() const;

	UFUNCTION(BlueprintCallable, Category = "BertaProcessBridge|Process", meta = (ReturnDisplayName = "Has Result"))
	bool TryGetResult(FBertaProcessResult& OutResult) const;

	/** Accepts Text for queued stdin delivery if capacity permits; true does not mean delivered. No terminator is added. */
	UFUNCTION(BlueprintCallable, Category = "BertaProcessBridge|Process|Input")
	bool SendString(const FString& Text);

	/** Accepts Line and one platform line terminator for queued stdin delivery if capacity permits. */
	UFUNCTION(BlueprintCallable, Category = "BertaProcessBridge|Process|Input")
	bool SendLine(const FString& Line);

	/** Requests cancellation. The native worker supplies the single terminal result. */
	UFUNCTION(BlueprintCallable, Category = "BertaProcessBridge|Process")
	bool Cancel(bool bKillTree = true);

private:
	friend class UBertaProcessSubsystem;
	friend class FBertaProcessCallbackDispatcher;

	bool LaunchInternal(const FBertaProcessLaunchOptions& Options, UBertaProcessSubsystem* InOwnerSubsystem);
	void HandleNativeOutput(FString Output);
	void HandleNativeFinished(
		EBertaProcessFinishReason Reason,
		bool bHasExitCode,
		int32 ExitCode,
		double DurationSeconds);
	void ShutdownForOwner();
	void SuppressCallbacksAndReleaseNativeProcess(bool bCancelIfActive);

	UPROPERTY(Transient)
	EBertaProcessState State = EBertaProcessState::Canceled;

	UPROPERTY(Transient)
	FBertaProcessResult FinalResult;

	UPROPERTY(Transient)
	bool bHasFinalResult = false;

	TWeakObjectPtr<UBertaProcessSubsystem> OwnerSubsystem;
	TSharedPtr<FBertaProcessCallbackDispatcher, ESPMode::ThreadSafe> CallbackDispatcher;
	TUniquePtr<FBertaNativeProcessRunner> NativeRunner;
};
