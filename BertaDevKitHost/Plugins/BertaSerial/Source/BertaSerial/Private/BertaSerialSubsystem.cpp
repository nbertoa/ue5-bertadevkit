#include "BertaSerialSubsystem.h"

#include "BertaSerial.h"
#include "BertaSerialPort.h"
#include "BertaSerialUtils.h"

void UBertaSerialSubsystem::Deinitialize()
{
	bIsShuttingDown = true;

	TArray<TObjectPtr<UBertaSerialPort>> PortsToClose = MoveTemp(ActivePorts);
	ActivePorts.Reset();
	for (UBertaSerialPort* Port : PortsToClose)
	{
		if (Port != nullptr)
		{
			Port->ShutdownForOwner();
		}
	}

	Super::Deinitialize();
}

bool UBertaSerialSubsystem::OpenPort(
	const FBertaSerialOpenOptions& Options,
	UBertaSerialPort*& OutPort,
	EBertaSerialOpenError& OutError,
	FString& OutErrorMessage)
{
	OutPort = nullptr;
	OutError = EBertaSerialOpenError::None;
	OutErrorMessage.Reset();
	if (!IsInGameThread() || bIsShuttingDown)
	{
		OutError = EBertaSerialOpenError::SubsystemUnavailable;
		OutErrorMessage = TEXT("The BertaSerial GameInstance subsystem is unavailable or shutting down.");
		return false;
	}

	FString NormalizedPortName;
	if (!BertaSerial::Private::ValidateOpenOptions(
			Options,
			NormalizedPortName,
			OutError,
			OutErrorMessage))
	{
		return false;
	}

	UBertaSerialPort* Port = NewObject<UBertaSerialPort>(this);
	if (!Port->InitializeInternal(Options, this, OutError, OutErrorMessage))
	{
		UE_LOG(LogBertaSerial, Warning, TEXT("Serial port open failed: %s"), *OutErrorMessage);
		return false;
	}

	ActivePorts.Add(Port);
	OutPort = Port;
	return true;
}

void UBertaSerialSubsystem::NotifyPortClosed(UBertaSerialPort* Port)
{
	check(IsInGameThread());
	ActivePorts.RemoveSingleSwap(Port);
}
