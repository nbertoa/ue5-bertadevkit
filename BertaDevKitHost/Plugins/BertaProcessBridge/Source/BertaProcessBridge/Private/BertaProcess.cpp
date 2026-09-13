#include "BertaProcess.h"

#include "Async/Async.h"
#include "BertaProcessBridge.h"
#include "BertaProcessSubsystem.h"
#include "Containers/Queue.h"
#include "Containers/StringConv.h"
#include "HAL/CriticalSection.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"

namespace
{
	enum class EBertaQueuedProcessEventType : uint8
	{
		Output,
		Finished
	};

	struct FBertaQueuedProcessEvent
	{
		EBertaQueuedProcessEventType Type = EBertaQueuedProcessEventType::Output;
		FString Output;
		EBertaProcessFinishReason Reason = EBertaProcessFinishReason::Completed;
		bool bHasExitCode = false;
		int32 ExitCode = 0;
		double DurationSeconds = 0.0;
	};
}

/**
 * Serializes native worker events through one Game Thread drain. This preserves the
 * worker's output-before-terminal ordering without touching UObjects off-thread.
 */
class FBertaProcessCallbackDispatcher final
	: public TSharedFromThis<FBertaProcessCallbackDispatcher, ESPMode::ThreadSafe>
{
public:
	explicit FBertaProcessCallbackDispatcher(UBertaProcess* InProcess)
		: Process(InProcess)
	{
	}

	void EnqueueOutput(FString Output)
	{
		if (Output.IsEmpty())
		{
			return;
		}

		FBertaQueuedProcessEvent Event;
		Event.Type = EBertaQueuedProcessEventType::Output;
		Event.Output = MoveTemp(Output);
		Enqueue(MoveTemp(Event));
	}

	void EnqueueFinished(
		const EBertaProcessFinishReason Reason,
		const bool bHasExitCode,
		const int32 ExitCode,
		const double DurationSeconds)
	{
		FBertaQueuedProcessEvent Event;
		Event.Type = EBertaQueuedProcessEventType::Finished;
		Event.Reason = Reason;
		Event.bHasExitCode = bHasExitCode;
		Event.ExitCode = ExitCode;
		Event.DurationSeconds = DurationSeconds;
		Enqueue(MoveTemp(Event));
	}

	void Suppress()
	{
		FScopeLock Lock(&Mutex);
		bSuppressed = true;
		QueuedEvents.Reset();
		Process.Reset();
	}

private:
	void Enqueue(FBertaQueuedProcessEvent Event)
	{
		bool bScheduleDrain = false;
		{
			FScopeLock Lock(&Mutex);
			if (bSuppressed)
			{
				return;
			}

			QueuedEvents.Add(MoveTemp(Event));
			if (!bDrainScheduled)
			{
				bDrainScheduled = true;
				bScheduleDrain = true;
			}
		}

		if (bScheduleDrain)
		{
			TSharedRef<FBertaProcessCallbackDispatcher, ESPMode::ThreadSafe> Self = AsShared();
			AsyncTask(ENamedThreads::GameThread, [Self]()
			{
				Self->DrainOnGameThread();
			});
		}
	}

	void DrainOnGameThread()
	{
		check(IsInGameThread());

		for (;;)
		{
			TArray<FBertaQueuedProcessEvent> Events;
			TWeakObjectPtr<UBertaProcess> ProcessToNotify;
			{
				FScopeLock Lock(&Mutex);
				if (bSuppressed || QueuedEvents.IsEmpty())
				{
					bDrainScheduled = false;
					return;
				}

				Events = MoveTemp(QueuedEvents);
				QueuedEvents.Reset();
				ProcessToNotify = Process;
			}

			UBertaProcess* ProcessObject = ProcessToNotify.Get();
			if (ProcessObject == nullptr)
			{
				Suppress();
				return;
			}

			for (FBertaQueuedProcessEvent& Event : Events)
			{
				if (Event.Type == EBertaQueuedProcessEventType::Output)
				{
					ProcessObject->HandleNativeOutput(MoveTemp(Event.Output));
				}
				else
				{
					ProcessObject->HandleNativeFinished(
						Event.Reason,
						Event.bHasExitCode,
						Event.ExitCode,
						Event.DurationSeconds);
				}
			}
		}
	}

	FCriticalSection Mutex;
	TArray<FBertaQueuedProcessEvent> QueuedEvents;
	TWeakObjectPtr<UBertaProcess> Process;
	bool bDrainScheduled = false;
	bool bSuppressed = false;
};

/**
 * FInteractiveProcess cannot satisfy this plugin's exact-input and deterministic
 * cleanup contracts in UE 5.8. This focused runner uses UE's RAII process and pipe
 * wrappers while keeping all UObject interaction in the dispatcher above.
 */
class FBertaNativeProcessRunner final : public FRunnable
{
public:
	FBertaNativeProcessRunner(
		const FBertaProcessLaunchOptions& InOptions,
		TSharedRef<FBertaProcessCallbackDispatcher, ESPMode::ThreadSafe> InDispatcher)
		: ExecutablePath(InOptions.ExecutablePath)
		, Arguments(InOptions.Arguments)
		, WorkingDirectory(InOptions.WorkingDirectory)
		, bHidden(InOptions.bHidden)
		, Dispatcher(MoveTemp(InDispatcher))
		, ProcessOutputPipe(UE::HAL::NewPipe)
		, ProcessInputPipe(UE::HAL::NewPipe)
	{
	}

	virtual ~FBertaNativeProcessRunner() override
	{
		StopAndWait(true);
	}

	bool Launch()
	{
		if (!ProcessOutputPipe || !ProcessInputPipe)
		{
			return false;
		}

		UE::HAL::FProcessStartInfo StartInfo{};
		StartInfo.Uri = *ExecutablePath;
		StartInfo.Arguments = *Arguments;
		StartInfo.WorkingDirectory = WorkingDirectory.IsEmpty() ? nullptr : *WorkingDirectory;
		StartInfo.bDetached = false;
		StartInfo.bHidden = bHidden;
		StartInfo.StdIn = ProcessInputPipe;
		StartInfo.StdOut = ProcessOutputPipe;
		StartInfo.StdErr = ProcessOutputPipe;

		Process = UE::HAL::FProcess(StartInfo);
		if (!Process)
		{
			return false;
		}

		{
			FScopeLock Lock(&StateMutex);
			State = EWorkerState::Active;
			StartTimeSeconds = FPlatformTime::Seconds();
		}

		const FString ThreadName = FString::Printf(TEXT("BertaProcessBridge_%p"), this);
		Thread = FRunnableThread::Create(this, *ThreadName);
		if (Thread == nullptr)
		{
			Process.Kill(true).WaitForExit();
			Process = nullptr;
			FScopeLock Lock(&StateMutex);
			State = EWorkerState::Terminal;
			TerminalDurationSeconds = FPlatformTime::Seconds() - StartTimeSeconds;
			return false;
		}

		return true;
	}

	bool IsActive() const
	{
		FScopeLock Lock(&StateMutex);
		return State == EWorkerState::Active || State == EWorkerState::CancelRequested;
	}

	double GetDurationSeconds() const
	{
		FScopeLock Lock(&StateMutex);
		if (State == EWorkerState::NotStarted)
		{
			return 0.0;
		}
		if (State == EWorkerState::Terminal)
		{
			return TerminalDurationSeconds;
		}
		return FPlatformTime::Seconds() - StartTimeSeconds;
	}

	bool EnqueueInput(const FString& Text)
	{
		FScopeLock Lock(&StateMutex);
		if (State != EWorkerState::Active)
		{
			return false;
		}

		if (!Text.IsEmpty())
		{
			InputMessages.Enqueue(Text);
		}
		return true;
	}

	bool RequestCancel(const bool bKillTree)
	{
		FScopeLock Lock(&StateMutex);
		if (State != EWorkerState::Active)
		{
			return false;
		}

		bShouldKillTree = bKillTree;
		State = EWorkerState::CancelRequested;
		return true;
	}

	void StopAndWait(const bool bKillTree)
	{
		RequestCancel(bKillTree);
		if (Thread != nullptr)
		{
			Thread->WaitForCompletion();
			delete Thread;
			Thread = nullptr;
		}
	}

	virtual uint32 Run() override
	{
		for (;;)
		{
			ReadAvailableOutput();

			bool bCancel = false;
			bool bKillTree = false;
			{
				FScopeLock Lock(&StateMutex);
				if (State == EWorkerState::CancelRequested)
				{
					State = EWorkerState::Finalizing;
					bCancel = true;
					bKillTree = bShouldKillTree;
				}
			}

			if (bCancel)
			{
				if (Process.IsRunning())
				{
					Process.Kill(bKillTree);
				}
				Process.WaitForExit();
				ReadRemainingOutput();
				Finalize(EBertaProcessFinishReason::Canceled);
				return 0;
			}

			if (!Process.IsRunning())
			{
				EBertaProcessFinishReason Reason = EBertaProcessFinishReason::Completed;
				{
					FScopeLock Lock(&StateMutex);
					if (State == EWorkerState::CancelRequested)
					{
						Reason = EBertaProcessFinishReason::Canceled;
					}
					State = EWorkerState::Finalizing;
				}

				Process.WaitForExit();
				ReadRemainingOutput();
				Finalize(Reason);
				return 0;
			}

			WritePendingInput();
			FPlatformProcess::Sleep(0.01f);
		}
	}

	virtual void Stop() override
	{
		RequestCancel(true);
	}

private:
	enum class EWorkerState : uint8
	{
		NotStarted,
		Active,
		CancelRequested,
		Finalizing,
		Terminal
	};

	void ReadAvailableOutput()
	{
		FString Output = ProcessOutputPipe.Read();
		if (!Output.IsEmpty())
		{
			Dispatcher->EnqueueOutput(MoveTemp(Output));
		}
	}

	void ReadRemainingOutput()
	{
		for (;;)
		{
			FString Output = ProcessOutputPipe.Read();
			if (Output.IsEmpty())
			{
				return;
			}
			Dispatcher->EnqueueOutput(MoveTemp(Output));
		}
	}

	void WritePendingInput()
	{
		FString Message;
		while (InputMessages.Dequeue(Message))
		{
			FTCHARToUTF8 Utf8Message(*Message);
			int32 Offset = 0;
			while (Offset < Utf8Message.Length())
			{
				int32 BytesWritten = 0;
				const bool bWriteSucceeded = FPlatformProcess::WritePipe(
					ProcessInputPipe.NativeHandle(),
					reinterpret_cast<const uint8*>(Utf8Message.Get()) + Offset,
					Utf8Message.Length() - Offset,
					&BytesWritten);
				if (!bWriteSucceeded || BytesWritten <= 0)
				{
					UE_LOG(LogBertaProcessBridge, Verbose, TEXT("The child process stopped accepting stdin."));
					return;
				}
				Offset += BytesWritten;
			}
		}
	}

	void Finalize(const EBertaProcessFinishReason Reason)
	{
		const TOptional<int32> ExitCode = Process.GetExitCode();
		double DurationSeconds = 0.0;
		{
			FScopeLock Lock(&StateMutex);
			TerminalDurationSeconds = FPlatformTime::Seconds() - StartTimeSeconds;
			DurationSeconds = TerminalDurationSeconds;
			State = EWorkerState::Terminal;
		}

		Dispatcher->EnqueueFinished(
			Reason,
			ExitCode.IsSet(),
			ExitCode.Get(0),
			DurationSeconds);
	}

	FString ExecutablePath;
	FString Arguments;
	FString WorkingDirectory;
	bool bHidden = true;
	TSharedRef<FBertaProcessCallbackDispatcher, ESPMode::ThreadSafe> Dispatcher;
	UE::HAL::FInputPipe ProcessOutputPipe;
	UE::HAL::FOutputPipe ProcessInputPipe;
	UE::HAL::FProcess Process;
	FRunnableThread* Thread = nullptr;
	TQueue<FString, EQueueMode::Mpsc> InputMessages;
	mutable FCriticalSection StateMutex;
	EWorkerState State = EWorkerState::NotStarted;
	bool bShouldKillTree = false;
	double StartTimeSeconds = 0.0;
	double TerminalDurationSeconds = 0.0;
};

UBertaProcess::~UBertaProcess()
{
	SuppressCallbacksAndReleaseNativeProcess(true);
}

void UBertaProcess::BeginDestroy()
{
	SuppressCallbacksAndReleaseNativeProcess(true);
	Super::BeginDestroy();
}

bool UBertaProcess::IsRunning() const
{
	return State == EBertaProcessState::Running && NativeRunner && NativeRunner->IsActive();
}

EBertaProcessState UBertaProcess::GetState() const
{
	return State;
}

double UBertaProcess::GetDurationSeconds() const
{
	if (State == EBertaProcessState::Running && NativeRunner)
	{
		return NativeRunner->GetDurationSeconds();
	}
	return bHasFinalResult ? FinalResult.DurationSeconds : 0.0;
}

bool UBertaProcess::TryGetResult(FBertaProcessResult& OutResult) const
{
	OutResult = FBertaProcessResult{};
	if (!bHasFinalResult)
	{
		return false;
	}

	OutResult = FinalResult;
	return true;
}

bool UBertaProcess::SendString(const FString& Text)
{
	return State == EBertaProcessState::Running && NativeRunner && NativeRunner->EnqueueInput(Text);
}

bool UBertaProcess::SendLine(const FString& Line)
{
	return SendString(Line + LINE_TERMINATOR);
}

bool UBertaProcess::Cancel(const bool bKillTree)
{
	return State == EBertaProcessState::Running && NativeRunner && NativeRunner->RequestCancel(bKillTree);
}

bool UBertaProcess::LaunchInternal(
	const FBertaProcessLaunchOptions& Options,
	UBertaProcessSubsystem* InOwnerSubsystem)
{
	check(IsInGameThread());
	check(InOwnerSubsystem != nullptr);

	State = EBertaProcessState::Running;
	bHasFinalResult = false;
	FinalResult = FBertaProcessResult{};
	OwnerSubsystem = InOwnerSubsystem;
	CallbackDispatcher = MakeShared<FBertaProcessCallbackDispatcher, ESPMode::ThreadSafe>(this);
	NativeRunner = MakeUnique<FBertaNativeProcessRunner>(Options, CallbackDispatcher.ToSharedRef());

	if (!NativeRunner->Launch())
	{
		SuppressCallbacksAndReleaseNativeProcess(false);
		State = EBertaProcessState::Canceled;
		OwnerSubsystem.Reset();
		return false;
	}

	return true;
}

void UBertaProcess::HandleNativeOutput(FString Output)
{
	check(IsInGameThread());
	if (State == EBertaProcessState::Running && !Output.IsEmpty())
	{
		OnOutput.Broadcast(this, MoveTemp(Output));
	}
}

void UBertaProcess::HandleNativeFinished(
	const EBertaProcessFinishReason Reason,
	const bool bHasExitCode,
	const int32 ExitCode,
	const double DurationSeconds)
{
	check(IsInGameThread());
	if (State != EBertaProcessState::Running)
	{
		return;
	}

	FinalResult = FBertaProcessResult{};
	FinalResult.Reason = Reason;
	FinalResult.bHasExitCode = bHasExitCode;
	FinalResult.ExitCode = bHasExitCode ? ExitCode : 0;
	FinalResult.DurationSeconds = DurationSeconds;
	bHasFinalResult = true;
	State = Reason == EBertaProcessFinishReason::Completed
		? EBertaProcessState::Completed
		: EBertaProcessState::Canceled;

	OnFinished.Broadcast(this, FinalResult);

	if (UBertaProcessSubsystem* Subsystem = OwnerSubsystem.Get())
	{
		Subsystem->NotifyProcessFinished(this);
	}
	OwnerSubsystem.Reset();

	if (CallbackDispatcher)
	{
		CallbackDispatcher->Suppress();
	}
	NativeRunner.Reset();
}

void UBertaProcess::ShutdownForOwner()
{
	check(IsInGameThread());

	double DurationSeconds = GetDurationSeconds();
	if (CallbackDispatcher)
	{
		CallbackDispatcher->Suppress();
	}
	if (NativeRunner)
	{
		NativeRunner->StopAndWait(true);
		DurationSeconds = NativeRunner->GetDurationSeconds();
		NativeRunner.Reset();
	}

	if (State == EBertaProcessState::Running)
	{
		State = EBertaProcessState::Canceled;
		FinalResult = FBertaProcessResult{};
		FinalResult.Reason = EBertaProcessFinishReason::Canceled;
		FinalResult.DurationSeconds = DurationSeconds;
		bHasFinalResult = true;
	}
	OwnerSubsystem.Reset();
}

void UBertaProcess::SuppressCallbacksAndReleaseNativeProcess(const bool bCancelIfActive)
{
	if (CallbackDispatcher)
	{
		CallbackDispatcher->Suppress();
	}
	if (NativeRunner)
	{
		if (bCancelIfActive)
		{
			NativeRunner->RequestCancel(true);
		}
		NativeRunner.Reset();
	}
}
