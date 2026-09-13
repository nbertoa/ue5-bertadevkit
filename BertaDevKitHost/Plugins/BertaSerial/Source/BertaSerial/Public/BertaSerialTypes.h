#pragma once

#include "CoreMinimal.h"

#include "BertaSerialTypes.generated.h"

UENUM(BlueprintType)
enum class EBertaSerialParity : uint8
{
	None,
	Odd,
	Even,
	Mark,
	Space
};

UENUM(BlueprintType)
enum class EBertaSerialStopBits : uint8
{
	One,
	OnePointFive,
	Two
};

UENUM(BlueprintType)
enum class EBertaSerialFlowControl : uint8
{
	None,
	RtsCts,
	XOnXOff
};

UENUM(BlueprintType)
enum class EBertaSerialLineEnding : uint8
{
	LF,
	CRLF,
	CR
};

UENUM(BlueprintType)
enum class EBertaSerialOpenError : uint8
{
	None,
	InvalidWorldContext,
	SubsystemUnavailable,
	InvalidPortName,
	InvalidSettings,
	PortUnavailable,
	AccessDenied,
	ConfigurationFailed,
	WorkerStartFailed,
	OpenFailed
};

UENUM(BlueprintType)
enum class EBertaSerialCloseReason : uint8
{
	UserClosed,
	ConnectionLost,
	IOFailure,
	ReceiveBufferOverflow
};

USTRUCT(BlueprintType)
struct BERTASERIAL_API FBertaSerialPortInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Serial Port")
	FString PortName;

	UPROPERTY(BlueprintReadOnly, Category = "Serial Port")
	FString FriendlyName;

	UPROPERTY(BlueprintReadOnly, Category = "Serial Port")
	FString Manufacturer;

	UPROPERTY(BlueprintReadOnly, Category = "Serial Port")
	FString DeviceInstanceId;

	UPROPERTY(BlueprintReadOnly, Category = "Serial Port")
	TArray<FString> HardwareIds;
};

USTRUCT(BlueprintType)
struct BERTASERIAL_API FBertaSerialOpenOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Serial Port")
	FString PortName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Serial Port", meta = (ClampMin = "1"))
	int32 BaudRate = 115200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Serial Port", meta = (ClampMin = "5", ClampMax = "8"))
	int32 DataBits = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Serial Port")
	EBertaSerialParity Parity = EBertaSerialParity::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Serial Port")
	EBertaSerialStopBits StopBits = EBertaSerialStopBits::One;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Serial Port")
	EBertaSerialFlowControl FlowControl = EBertaSerialFlowControl::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Serial Port")
	bool bDtrEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Serial Port")
	bool bRtsEnabled = true;
};
