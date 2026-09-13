#include "BertaSerialPort.h"

#include "BertaSerial.h"
#include "BertaSerialDispatcher.h"
#include "BertaSerialSubsystem.h"
#include "BertaSerialUtils.h"
#include "Windows/BertaWindowsSerialWorker.h"

UBertaSerialPort::UBertaSerialPort() = default;

UBertaSerialPort::~UBertaSerialPort()
{
	SuppressCallbacksAndReleaseWorker();
}

void UBertaSerialPort::BeginDestroy()
{
	SuppressCallbacksAndReleaseWorker();
	Super::BeginDestroy();
}

bool UBertaSerialPort::IsOpen() const
{
	return State == EState::Open && Worker.IsValid();
}

FString UBertaSerialPort::GetPortName() const
{
	return OpenOptions.PortName;
}

FBertaSerialOpenOptions UBertaSerialPort::GetOpenOptions() const
{
	return OpenOptions;
}

bool UBertaSerialPort::WriteBytes(const TArray<uint8>& Data)
{
	return IsInGameThread() && State == EState::Open && Worker && Worker->EnqueueWrite(Data);
}

bool UBertaSerialPort::WriteStringUtf8(const FString& Text)
{
	return WriteBytes(BertaSerial::Private::EncodeUtf8(Text));
}

bool UBertaSerialPort::WriteLineUtf8(
	const FString& Text,
	const EBertaSerialLineEnding LineEnding)
{
	return WriteStringUtf8(Text + BertaSerial::Private::GetLineEnding(LineEnding));
}

bool UBertaSerialPort::Close()
{
	if (!IsInGameThread() || State != EState::Open || !Worker || !Dispatcher)
	{
		return false;
	}

	State = EState::Closing;
	Worker->StopAndWait();
	Dispatcher->FinalizeOnGameThread(EBertaSerialCloseReason::UserClosed, FString());
	return true;
}

bool UBertaSerialPort::InitializeInternal(
	const FBertaSerialOpenOptions& Options,
	UBertaSerialSubsystem* InOwnerSubsystem,
	EBertaSerialOpenError& OutError,
	FString& OutErrorMessage)
{
	check(IsInGameThread());
	check(InOwnerSubsystem != nullptr);

	FString NormalizedPortName;
	if (!BertaSerial::Private::ValidateOpenOptions(
			Options,
			NormalizedPortName,
			OutError,
			OutErrorMessage))
	{
		return false;
	}

	OpenOptions = Options;
	OpenOptions.PortName = NormalizedPortName;
	OwnerSubsystem = InOwnerSubsystem;
	Dispatcher = MakeShared<FBertaSerialDispatcher, ESPMode::ThreadSafe>(this);
	Worker = FBertaWindowsSerialWorker::Open(OpenOptions, Dispatcher.ToSharedRef(), OutError, OutErrorMessage);
	if (!Worker)
	{
		Dispatcher->Suppress();
		Dispatcher.Reset();
		OwnerSubsystem.Reset();
		return false;
	}

	State = EState::Open;
	return true;
}

void UBertaSerialPort::HandleNativeBytes(TArray<uint8> Data)
{
	check(IsInGameThread());
	if (State != EState::Closed && !Data.IsEmpty())
	{
		OnBytesReceived.Broadcast(this, Data);
	}
}

void UBertaSerialPort::HandleNativeClosed(
	const EBertaSerialCloseReason Reason,
	FString ErrorMessage)
{
	check(IsInGameThread());
	if (State == EState::Closed)
	{
		return;
	}

	State = EState::Closed;
	if (Worker)
	{
		Worker->StopAndWait();
		Worker.Reset();
	}
	if (Reason != EBertaSerialCloseReason::UserClosed)
	{
		UE_LOG(
			LogBertaSerial,
			Warning,
			TEXT("Serial connection %s closed: %s"),
			*OpenOptions.PortName,
			ErrorMessage.IsEmpty() ? TEXT("no additional error information") : *ErrorMessage);
	}

	OnClosed.Broadcast(this, Reason, MoveTemp(ErrorMessage));
	if (UBertaSerialSubsystem* Subsystem = OwnerSubsystem.Get())
	{
		Subsystem->NotifyPortClosed(this);
	}
	OwnerSubsystem.Reset();

	if (Dispatcher)
	{
		Dispatcher->Suppress();
	}
	Dispatcher.Reset();
}

void UBertaSerialPort::ShutdownForOwner()
{
	check(IsInGameThread());
	if (Dispatcher)
	{
		Dispatcher->Suppress();
	}
	State = EState::Closed;
	if (Worker)
	{
		Worker->StopAndWait();
		Worker.Reset();
	}
	Dispatcher.Reset();
	OwnerSubsystem.Reset();
}

void UBertaSerialPort::SuppressCallbacksAndReleaseWorker()
{
	if (Dispatcher)
	{
		Dispatcher->Suppress();
	}
	if (Worker)
	{
		Worker->StopAndWait();
		Worker.Reset();
	}
	Dispatcher.Reset();
	OwnerSubsystem.Reset();
	State = EState::Closed;
}
