#include "Windows/BertaWindowsSerialWorker.h"

#include "BertaSerialDispatcher.h"
#include "BertaSerialUtils.h"
#include "Containers/Queue.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"
#include "Windows/BertaWindowsSerialCommon.h"

#include "Windows/WindowsHWrapper.h"

namespace
{
	constexpr int32 ReadBufferSize = 8192;
	constexpr int64 MaxPendingWriteBytes = 4ll * 1024ll * 1024ll;

	BYTE ToWindowsParity(const EBertaSerialParity Parity)
	{
		switch (Parity)
		{
		case EBertaSerialParity::Odd:
			return ODDPARITY;
		case EBertaSerialParity::Even:
			return EVENPARITY;
		case EBertaSerialParity::Mark:
			return MARKPARITY;
		case EBertaSerialParity::Space:
			return SPACEPARITY;
		case EBertaSerialParity::None:
		default:
			return NOPARITY;
		}
	}

	BYTE ToWindowsStopBits(const EBertaSerialStopBits StopBits)
	{
		switch (StopBits)
		{
		case EBertaSerialStopBits::OnePointFive:
			return ONE5STOPBITS;
		case EBertaSerialStopBits::Two:
			return TWOSTOPBITS;
		case EBertaSerialStopBits::One:
		default:
			return ONESTOPBIT;
		}
	}

	EBertaSerialOpenError ClassifyOpenError(const uint32 ErrorCode)
	{
		if (ErrorCode == ERROR_ACCESS_DENIED || ErrorCode == ERROR_SHARING_VIOLATION)
		{
			return EBertaSerialOpenError::AccessDenied;
		}
		if (BertaSerial::Windows::IsConnectionLostError(ErrorCode))
		{
			return EBertaSerialOpenError::PortUnavailable;
		}
		return EBertaSerialOpenError::OpenFailed;
	}
}

class FBertaWindowsSerialWorker::FImplementation final : public FRunnable
{
public:
	explicit FImplementation(
		const TSharedRef<FBertaSerialDispatcher, ESPMode::ThreadSafe>& InDispatcher)
		: Dispatcher(InDispatcher)
	{
		ReadBuffer.SetNumUninitialized(ReadBufferSize);
	}

	~FImplementation()
	{
		StopAndWait();
		CloseNativeResources();
	}

	bool Initialize(
		const FBertaSerialOpenOptions& Options,
		EBertaSerialOpenError& OutError,
		FString& OutErrorMessage)
	{
		FString NormalizedPortName;
		if (!BertaSerial::Private::ValidateOpenOptions(
				Options,
				NormalizedPortName,
				OutError,
				OutErrorMessage))
		{
			return false;
		}

		const FString DevicePath = FString::Printf(TEXT("\\\\.\\%s"), *NormalizedPortName);
		PortHandle = ::CreateFileW(
			*DevicePath,
			GENERIC_READ | GENERIC_WRITE,
			0,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			nullptr);
		if (PortHandle == INVALID_HANDLE_VALUE)
		{
			const uint32 ErrorCode = ::GetLastError();
			OutError = ClassifyOpenError(ErrorCode);
			OutErrorMessage = FString::Printf(
				TEXT("Failed to open %s: %s."),
				*NormalizedPortName,
				*BertaSerial::Windows::FormatWindowsError(ErrorCode));
			return false;
		}

		DCB Configuration{};
		Configuration.DCBlength = sizeof(Configuration);
		if (!::GetCommState(PortHandle, &Configuration))
		{
			return FailConfiguration(NormalizedPortName, Options.BaudRate, OutError, OutErrorMessage);
		}
		Configuration.BaudRate = static_cast<DWORD>(Options.BaudRate);
		Configuration.ByteSize = static_cast<BYTE>(Options.DataBits);
		Configuration.Parity = ToWindowsParity(Options.Parity);
		Configuration.StopBits = ToWindowsStopBits(Options.StopBits);
		Configuration.fBinary = 1;
		Configuration.fParity = Options.Parity == EBertaSerialParity::None ? 0 : 1;
		Configuration.fOutxCtsFlow = Options.FlowControl == EBertaSerialFlowControl::RtsCts ? 1 : 0;
		Configuration.fOutxDsrFlow = 0;
		Configuration.fDtrControl = Options.bDtrEnabled ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;
		Configuration.fDsrSensitivity = 0;
		Configuration.fTXContinueOnXoff = 1;
		Configuration.fOutX = Options.FlowControl == EBertaSerialFlowControl::XOnXOff ? 1 : 0;
		Configuration.fInX = Options.FlowControl == EBertaSerialFlowControl::XOnXOff ? 1 : 0;
		Configuration.fErrorChar = 0;
		Configuration.fNull = 0;
		Configuration.fRtsControl = Options.FlowControl == EBertaSerialFlowControl::RtsCts
			? RTS_CONTROL_HANDSHAKE
			: (Options.bRtsEnabled ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE);
		Configuration.fAbortOnError = 0;
		if (!::SetCommState(PortHandle, &Configuration))
		{
			return FailConfiguration(NormalizedPortName, Options.BaudRate, OutError, OutErrorMessage);
		}

		// This documented special case returns available bytes immediately, otherwise
		// waits for the first byte or the constant. It avoids waiting for all 8192 bytes.
		COMMTIMEOUTS Timeouts{};
		Timeouts.ReadIntervalTimeout = MAXDWORD;
		Timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
		Timeouts.ReadTotalTimeoutConstant = 1000;
		if (!::SetCommTimeouts(PortHandle, &Timeouts))
		{
			return FailConfiguration(NormalizedPortName, Options.BaudRate, OutError, OutErrorMessage);
		}

		StopEvent = ::CreateEventW(nullptr, 1, 0, nullptr);
		ReadEvent = ::CreateEventW(nullptr, 1, 0, nullptr);
		WriteEvent = ::CreateEventW(nullptr, 1, 0, nullptr);
		WriteQueuedEvent = ::CreateEventW(nullptr, 0, 0, nullptr);
		if (StopEvent == nullptr || ReadEvent == nullptr || WriteEvent == nullptr || WriteQueuedEvent == nullptr)
		{
			const uint32 ErrorCode = ::GetLastError();
			OutError = EBertaSerialOpenError::WorkerStartFailed;
			OutErrorMessage = FString::Printf(
				TEXT("Failed to create overlapped I/O events for %s: %s."),
				*NormalizedPortName,
				*BertaSerial::Windows::FormatWindowsError(ErrorCode));
			return false;
		}

		ReadOverlapped.hEvent = ReadEvent;
		WriteOverlapped.hEvent = WriteEvent;
		{
			FScopeLock Lock(&QueueMutex);
			bAcceptWrites = true;
		}
		const FString ThreadName = FString::Printf(TEXT("BertaSerial_%s_%p"), *NormalizedPortName, this);
		Thread = FRunnableThread::Create(this, *ThreadName);
		if (Thread == nullptr)
		{
			FScopeLock Lock(&QueueMutex);
			bAcceptWrites = false;
			OutError = EBertaSerialOpenError::WorkerStartFailed;
			OutErrorMessage = FString::Printf(TEXT("Failed to start the serial I/O worker for %s."), *NormalizedPortName);
			return false;
		}

		OutError = EBertaSerialOpenError::None;
		OutErrorMessage.Reset();
		return true;
	}

	bool EnqueueWrite(const TArray<uint8>& Data)
	{
		FScopeLock Lock(&QueueMutex);
		if (!bAcceptWrites)
		{
			return false;
		}
		if (Data.IsEmpty())
		{
			return true;
		}
		if (Data.Num() > MaxPendingWriteBytes - PendingWriteBytes)
		{
			return false;
		}

		PendingWriteBytes += Data.Num();
		WriteQueue.Enqueue(Data);
		::SetEvent(WriteQueuedEvent);
		return true;
	}

	void StopAndWait()
	{
		{
			FScopeLock Lock(&QueueMutex);
			bAcceptWrites = false;
		}
		if (StopEvent != nullptr)
		{
			::SetEvent(StopEvent);
		}
		if (PortHandle != INVALID_HANDLE_VALUE)
		{
			::CancelIoEx(PortHandle, nullptr);
		}
		if (Thread != nullptr)
		{
			Thread->WaitForCompletion();
			delete Thread;
			Thread = nullptr;
		}
		DiscardPendingWrites();
	}

	virtual uint32 Run() override
	{
		while (!IsStopRequested())
		{
			if (bReadPending && ::WaitForSingleObject(ReadEvent, 0) == WAIT_OBJECT_0 && !CompleteRead())
			{
				break;
			}
			if (bWritePending && ::WaitForSingleObject(WriteEvent, 0) == WAIT_OBJECT_0 && !CompleteWrite())
			{
				break;
			}
			if (!bWritePending && !IssueNextWrite())
			{
				break;
			}
			if (!bReadPending)
			{
				if (!IssueRead())
				{
					break;
				}
				if (!bReadPending)
				{
					continue;
				}
			}
			if (!bWritePending && HasPendingWrites())
			{
				continue;
			}

			const HANDLE WaitHandles[] = { StopEvent, ReadEvent, WriteEvent, WriteQueuedEvent };
			const DWORD WaitResult = ::WaitForMultipleObjects(UE_ARRAY_COUNT(WaitHandles), WaitHandles, 0, INFINITE);
			if (WaitResult == WAIT_OBJECT_0)
			{
				break;
			}
			if (WaitResult == WAIT_OBJECT_0 + 1 && bReadPending)
			{
				if (!CompleteRead())
				{
					break;
				}
			}
			else if (WaitResult == WAIT_OBJECT_0 + 2 && bWritePending)
			{
				if (!CompleteWrite())
				{
					break;
				}
			}
			else if (WaitResult != WAIT_OBJECT_0 + 3)
			{
				SetFailure(::GetLastError(), TEXT("WaitForMultipleObjects failed for serial overlapped I/O"));
				break;
			}
		}

		CancelAndCompleteOutstandingOperations();
		{
			FScopeLock Lock(&QueueMutex);
			bAcceptWrites = false;
		}
		DiscardPendingWrites();

		if (!IsStopRequested() && !bDispatcherTerminalAlreadySet)
		{
			const EBertaSerialCloseReason Reason = BertaSerial::Windows::IsConnectionLostError(FailureErrorCode)
				? EBertaSerialCloseReason::ConnectionLost
				: EBertaSerialCloseReason::IOFailure;
			Dispatcher->PublishTerminal(Reason, FailureMessage);
		}
		return 0;
	}

	virtual void Stop() override
	{
		if (StopEvent != nullptr)
		{
			::SetEvent(StopEvent);
		}
		if (PortHandle != INVALID_HANDLE_VALUE)
		{
			::CancelIoEx(PortHandle, nullptr);
		}
	}

private:
	bool FailConfiguration(
		const FString& PortName,
		const int32 BaudRate,
		EBertaSerialOpenError& OutError,
		FString& OutErrorMessage)
	{
		const uint32 ErrorCode = ::GetLastError();
		OutError = EBertaSerialOpenError::ConfigurationFailed;
		OutErrorMessage = FString::Printf(
			TEXT("Failed to configure %s at %d baud: %s."),
			*PortName,
			BaudRate,
			*BertaSerial::Windows::FormatWindowsError(ErrorCode));
		return false;
	}

	bool IssueRead()
	{
		ReadOverlapped = {};
		ReadOverlapped.hEvent = ReadEvent;
		::ResetEvent(ReadEvent);
		DWORD BytesRead = 0;
		if (::ReadFile(
				PortHandle,
				ReadBuffer.GetData(),
				static_cast<DWORD>(ReadBuffer.Num()),
				&BytesRead,
				&ReadOverlapped))
		{
			::ResetEvent(ReadEvent);
			return HandleReadBytes(BytesRead);
		}
		const uint32 ErrorCode = ::GetLastError();
		if (ErrorCode == ERROR_IO_PENDING)
		{
			bReadPending = true;
			return true;
		}
		SetFailure(ErrorCode, TEXT("ReadFile failed"));
		return false;
	}

	bool CompleteRead()
	{
		DWORD BytesRead = 0;
		const bool bSucceeded = ::GetOverlappedResult(PortHandle, &ReadOverlapped, &BytesRead, 0) != 0;
		bReadPending = false;
		::ResetEvent(ReadEvent);
		if (!bSucceeded)
		{
			const uint32 ErrorCode = ::GetLastError();
			if (ErrorCode == ERROR_OPERATION_ABORTED && IsStopRequested())
			{
				return true;
			}
			SetFailure(ErrorCode, TEXT("An overlapped serial read failed"));
			return false;
		}
		return HandleReadBytes(BytesRead);
	}

	bool HandleReadBytes(const DWORD BytesRead)
	{
		if (BytesRead > 0)
		{
			TArray<uint8> Data;
			Data.Append(ReadBuffer.GetData(), static_cast<int32>(BytesRead));
			if (!Dispatcher->PublishBytes(MoveTemp(Data)))
			{
				bDispatcherTerminalAlreadySet = true;
				return false;
			}
		}

		DWORD CommunicationErrors = 0;
		COMSTAT Status{};
		if (!::ClearCommError(PortHandle, &CommunicationErrors, &Status))
		{
			SetFailure(::GetLastError(), TEXT("ClearCommError failed after a serial read"));
			return false;
		}
		if ((CommunicationErrors & (CE_RXOVER | CE_OVERRUN)) != 0)
		{
			FailureErrorCode = ERROR_IO_DEVICE;
			FailureMessage = TEXT("Windows reported serial receive data loss (CE_RXOVER or CE_OVERRUN).");
			return false;
		}
		return true;
	}

	bool IssueNextWrite()
	{
		if (CurrentWrite.IsEmpty())
		{
			if (!WriteQueue.Dequeue(CurrentWrite))
			{
				return true;
			}
			CurrentWriteOffset = 0;
		}

		WriteOverlapped = {};
		WriteOverlapped.hEvent = WriteEvent;
		::ResetEvent(WriteEvent);
		DWORD BytesWritten = 0;
		const int32 Remaining = CurrentWrite.Num() - CurrentWriteOffset;
		if (::WriteFile(
				PortHandle,
				CurrentWrite.GetData() + CurrentWriteOffset,
				static_cast<DWORD>(Remaining),
				&BytesWritten,
				&WriteOverlapped))
		{
			::ResetEvent(WriteEvent);
			return HandleWrittenBytes(BytesWritten);
		}
		const uint32 ErrorCode = ::GetLastError();
		if (ErrorCode == ERROR_IO_PENDING)
		{
			bWritePending = true;
			return true;
		}
		SetFailure(ErrorCode, TEXT("WriteFile failed"));
		return false;
	}

	bool CompleteWrite()
	{
		DWORD BytesWritten = 0;
		const bool bSucceeded = ::GetOverlappedResult(PortHandle, &WriteOverlapped, &BytesWritten, 0) != 0;
		bWritePending = false;
		::ResetEvent(WriteEvent);
		if (!bSucceeded)
		{
			const uint32 ErrorCode = ::GetLastError();
			if (ErrorCode == ERROR_OPERATION_ABORTED && IsStopRequested())
			{
				return true;
			}
			SetFailure(ErrorCode, TEXT("An overlapped serial write failed"));
			return false;
		}
		return HandleWrittenBytes(BytesWritten);
	}

	bool HandleWrittenBytes(const DWORD BytesWritten)
	{
		const int32 Remaining = CurrentWrite.Num() - CurrentWriteOffset;
		if (BytesWritten == 0 || BytesWritten > static_cast<DWORD>(Remaining))
		{
			SetFailure(ERROR_WRITE_FAULT, TEXT("Windows completed a serial write without valid progress"));
			return false;
		}

		CurrentWriteOffset += static_cast<int32>(BytesWritten);
		{
			FScopeLock Lock(&QueueMutex);
			PendingWriteBytes -= BytesWritten;
		}
		if (CurrentWriteOffset == CurrentWrite.Num())
		{
			CurrentWrite.Reset();
			CurrentWriteOffset = 0;
		}
		return true;
	}

	void SetFailure(const uint32 ErrorCode, const TCHAR* Context)
	{
		FailureErrorCode = ErrorCode;
		FailureMessage = FString::Printf(
			TEXT("%s: %s."),
			Context,
			*BertaSerial::Windows::FormatWindowsError(ErrorCode));
	}

	bool IsStopRequested() const
	{
		return StopEvent != nullptr && ::WaitForSingleObject(StopEvent, 0) == WAIT_OBJECT_0;
	}

	bool HasPendingWrites()
	{
		FScopeLock Lock(&QueueMutex);
		return PendingWriteBytes > 0;
	}

	void CancelAndCompleteOutstandingOperations()
	{
		if (PortHandle == INVALID_HANDLE_VALUE)
		{
			return;
		}
		::CancelIoEx(PortHandle, nullptr);
		DWORD IgnoredBytes = 0;
		if (bReadPending)
		{
			::GetOverlappedResult(PortHandle, &ReadOverlapped, &IgnoredBytes, 1);
			bReadPending = false;
		}
		if (bWritePending)
		{
			::GetOverlappedResult(PortHandle, &WriteOverlapped, &IgnoredBytes, 1);
			bWritePending = false;
		}
	}

	void DiscardPendingWrites()
	{
		TArray<uint8> Discarded;
		while (WriteQueue.Dequeue(Discarded))
		{
			Discarded.Reset();
		}
		CurrentWrite.Reset();
		CurrentWriteOffset = 0;
		FScopeLock Lock(&QueueMutex);
		PendingWriteBytes = 0;
	}

	void CloseNativeResources()
	{
		if (PortHandle != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(PortHandle);
			PortHandle = INVALID_HANDLE_VALUE;
		}
		for (HANDLE* Event : { &StopEvent, &ReadEvent, &WriteEvent, &WriteQueuedEvent })
		{
			if (*Event != nullptr)
			{
				::CloseHandle(*Event);
				*Event = nullptr;
			}
		}
	}

	TSharedRef<FBertaSerialDispatcher, ESPMode::ThreadSafe> Dispatcher;
	HANDLE PortHandle = INVALID_HANDLE_VALUE;
	HANDLE StopEvent = nullptr;
	HANDLE ReadEvent = nullptr;
	HANDLE WriteEvent = nullptr;
	HANDLE WriteQueuedEvent = nullptr;
	OVERLAPPED ReadOverlapped{};
	OVERLAPPED WriteOverlapped{};
	FRunnableThread* Thread = nullptr;
	TArray<uint8> ReadBuffer;
	TQueue<TArray<uint8>, EQueueMode::Mpsc> WriteQueue;
	TArray<uint8> CurrentWrite;
	int32 CurrentWriteOffset = 0;
	FCriticalSection QueueMutex;
	int64 PendingWriteBytes = 0;
	uint32 FailureErrorCode = ERROR_GEN_FAILURE;
	FString FailureMessage = TEXT("The serial I/O worker stopped unexpectedly.");
	bool bAcceptWrites = false;
	bool bReadPending = false;
	bool bWritePending = false;
	bool bDispatcherTerminalAlreadySet = false;
};

FBertaWindowsSerialWorker::FBertaWindowsSerialWorker()
	: Implementation(nullptr)
{
}

FBertaWindowsSerialWorker::~FBertaWindowsSerialWorker() = default;

TSharedPtr<FBertaWindowsSerialWorker> FBertaWindowsSerialWorker::Open(
	const FBertaSerialOpenOptions& Options,
	const TSharedRef<FBertaSerialDispatcher, ESPMode::ThreadSafe>& Dispatcher,
	EBertaSerialOpenError& OutError,
	FString& OutErrorMessage)
{
	TSharedPtr<FBertaWindowsSerialWorker> Worker(new FBertaWindowsSerialWorker());
	Worker->Implementation = MakeUnique<FImplementation>(Dispatcher);
	if (!Worker->Implementation->Initialize(Options, OutError, OutErrorMessage))
	{
		return nullptr;
	}
	return Worker;
}

bool FBertaWindowsSerialWorker::EnqueueWrite(const TArray<uint8>& Data)
{
	return Implementation && Implementation->EnqueueWrite(Data);
}

void FBertaWindowsSerialWorker::StopAndWait()
{
	if (Implementation)
	{
		Implementation->StopAndWait();
	}
}
