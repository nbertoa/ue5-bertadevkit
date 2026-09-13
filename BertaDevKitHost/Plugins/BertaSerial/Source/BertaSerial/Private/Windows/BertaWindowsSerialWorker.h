#pragma once

#include "BertaSerialTypes.h"
#include "Templates/SharedPointer.h"
#include "Templates/UniquePtr.h"

class FBertaSerialDispatcher;

/** Owns one exclusive Win32 COM handle and its overlapped I/O thread. */
class FBertaWindowsSerialWorker final
{
public:
	~FBertaWindowsSerialWorker();

	FBertaWindowsSerialWorker(const FBertaWindowsSerialWorker&) = delete;
	FBertaWindowsSerialWorker& operator=(const FBertaWindowsSerialWorker&) = delete;

	static TSharedPtr<FBertaWindowsSerialWorker> Open(
		const FBertaSerialOpenOptions& Options,
		const TSharedRef<FBertaSerialDispatcher, ESPMode::ThreadSafe>& Dispatcher,
		EBertaSerialOpenError& OutError,
		FString& OutErrorMessage);

	bool EnqueueWrite(const TArray<uint8>& Data);
	void StopAndWait();

private:
	FBertaWindowsSerialWorker();

	class FImplementation;
	TUniquePtr<FImplementation> Implementation;
};
