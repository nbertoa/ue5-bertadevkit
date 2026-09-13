#include "BertaSerialDispatcher.h"

#include "Async/Async.h"
#include "BertaSerialPort.h"
#include "Misc/ScopeLock.h"

FBertaSerialDispatcher::FBertaSerialDispatcher(UBertaSerialPort* InPort)
	: Port(InPort)
{
}

bool FBertaSerialDispatcher::PublishBytes(TArray<uint8> Data)
{
	if (Data.IsEmpty())
	{
		return true;
	}

	bool bShouldSchedule = false;
	bool bAccepted = true;
	{
		FScopeLock Lock(&Mutex);
		if (bSuppressed || bHasTerminal)
		{
			return false;
		}
		if (Data.Num() > MaxPendingReceiveBytes - PendingBytes.Num())
		{
			bHasTerminal = true;
			TerminalReason = EBertaSerialCloseReason::ReceiveBufferOverflow;
			TerminalErrorMessage = FString::Printf(
				TEXT("The pending serial receive backlog exceeded its %d-byte safety limit."),
				MaxPendingReceiveBytes);
			bAccepted = false;
		}
		else
		{
			if (PendingBytes.IsEmpty())
			{
				PendingBytes = MoveTemp(Data);
			}
			else
			{
				PendingBytes.Append(Data);
			}
		}
		ScheduleDrainLocked(bShouldSchedule);
	}

	if (bShouldSchedule)
	{
		TSharedRef<FBertaSerialDispatcher, ESPMode::ThreadSafe> Self = AsShared();
		AsyncTask(ENamedThreads::GameThread, [Self]()
		{
			Self->DrainOnGameThread();
		});
	}
	return bAccepted;
}

void FBertaSerialDispatcher::PublishTerminal(
	const EBertaSerialCloseReason Reason,
	FString ErrorMessage)
{
	bool bShouldSchedule = false;
	{
		FScopeLock Lock(&Mutex);
		if (bSuppressed || bHasTerminal)
		{
			return;
		}
		bHasTerminal = true;
		TerminalReason = Reason;
		TerminalErrorMessage = MoveTemp(ErrorMessage);
		ScheduleDrainLocked(bShouldSchedule);
	}

	if (bShouldSchedule)
	{
		TSharedRef<FBertaSerialDispatcher, ESPMode::ThreadSafe> Self = AsShared();
		AsyncTask(ENamedThreads::GameThread, [Self]()
		{
			Self->DrainOnGameThread();
		});
	}
}

void FBertaSerialDispatcher::FinalizeOnGameThread(
	const EBertaSerialCloseReason Reason,
	FString ErrorMessage)
{
	check(IsInGameThread());
	PublishTerminal(Reason, MoveTemp(ErrorMessage));
	DrainOnGameThread();
}

void FBertaSerialDispatcher::Suppress()
{
	FScopeLock Lock(&Mutex);
	bSuppressed = true;
	PendingBytes.Reset();
	TerminalErrorMessage.Reset();
	bHasTerminal = false;
	Port.Reset();
}

void FBertaSerialDispatcher::ScheduleDrainLocked(bool& bOutShouldSchedule)
{
	if (!bDrainScheduled)
	{
		bDrainScheduled = true;
		bOutShouldSchedule = true;
	}
}

void FBertaSerialDispatcher::DrainOnGameThread()
{
	check(IsInGameThread());

	for (;;)
	{
		TArray<uint8> Bytes;
		TWeakObjectPtr<UBertaSerialPort> PortToNotify;
		FString ErrorMessage;
		EBertaSerialCloseReason Reason = EBertaSerialCloseReason::IOFailure;
		bool bTerminal = false;
		{
			FScopeLock Lock(&Mutex);
			if (bSuppressed)
			{
				bDrainScheduled = false;
				return;
			}

			Bytes = MoveTemp(PendingBytes);
			PendingBytes.Reset();
			PortToNotify = Port;
			bTerminal = bHasTerminal;
			if (bTerminal)
			{
				Reason = TerminalReason;
				ErrorMessage = MoveTemp(TerminalErrorMessage);
				bHasTerminal = false;
			}
			bDrainScheduled = false;
		}

		UBertaSerialPort* PortObject = PortToNotify.Get();
		if (PortObject == nullptr)
		{
			Suppress();
			return;
		}
		if (!Bytes.IsEmpty())
		{
			PortObject->HandleNativeBytes(MoveTemp(Bytes));
		}
		if (bTerminal)
		{
			PortObject->HandleNativeClosed(Reason, MoveTemp(ErrorMessage));
			return;
		}

		FScopeLock Lock(&Mutex);
		if (PendingBytes.IsEmpty() && !bHasTerminal)
		{
			return;
		}
		if (!bDrainScheduled)
		{
			bDrainScheduled = true;
		}
	}
}
