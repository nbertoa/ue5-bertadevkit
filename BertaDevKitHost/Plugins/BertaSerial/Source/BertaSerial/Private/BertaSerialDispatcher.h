#pragma once

#include "BertaSerialTypes.h"
#include "Templates/SharedPointer.h"
#include "UObject/WeakObjectPtr.h"

class UBertaSerialPort;

/** Bounded, ordered worker-to-Game-Thread bridge for one serial port. */
class FBertaSerialDispatcher final
	: public TSharedFromThis<FBertaSerialDispatcher, ESPMode::ThreadSafe>
{
public:
	explicit FBertaSerialDispatcher(UBertaSerialPort* InPort);

	bool PublishBytes(TArray<uint8> Data);
	void PublishTerminal(EBertaSerialCloseReason Reason, FString ErrorMessage);
	void FinalizeOnGameThread(EBertaSerialCloseReason Reason, FString ErrorMessage);
	void Suppress();

private:
	void ScheduleDrainLocked(bool& bOutShouldSchedule);
	void DrainOnGameThread();

	static constexpr int32 MaxPendingReceiveBytes = 4 * 1024 * 1024;

	FCriticalSection Mutex;
	TWeakObjectPtr<UBertaSerialPort> Port;
	TArray<uint8> PendingBytes;
	FString TerminalErrorMessage;
	EBertaSerialCloseReason TerminalReason = EBertaSerialCloseReason::IOFailure;
	bool bHasTerminal = false;
	bool bDrainScheduled = false;
	bool bSuppressed = false;
};
