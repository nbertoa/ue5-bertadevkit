#pragma once

#include "BertaSerialTypes.h"
#include "Templates/SharedPointer.h"
#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"

#include "BertaSerialPort.generated.h"

class FBertaSerialDispatcher;
class FBertaWindowsSerialWorker;
class UBertaSerialPort;
class UBertaSerialSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FBertaSerialBytesReceivedDelegate,
	UBertaSerialPort*, Port,
	const TArray<uint8>&, Data);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FBertaSerialClosedDelegate,
	UBertaSerialPort*, Port,
	EBertaSerialCloseReason, Reason,
	FString, ErrorMessage);

/** One successfully opened Win64 serial connection. */
UCLASS(BlueprintType)
class BERTASERIAL_API UBertaSerialPort : public UObject
{
	GENERATED_BODY()

public:
	UBertaSerialPort();
	virtual ~UBertaSerialPort() override;
	virtual void BeginDestroy() override;

	/** Raw received bytes. Native read boundaries are arbitrary. */
	UPROPERTY(BlueprintAssignable, Category = "BertaSerial|Connection")
	FBertaSerialBytesReceivedDelegate OnBytesReceived;

	/** Broadcast exactly once when this connection reaches a terminal state. */
	UPROPERTY(BlueprintAssignable, Category = "BertaSerial|Connection")
	FBertaSerialClosedDelegate OnClosed;

	UFUNCTION(BlueprintPure, Category = "BertaSerial|Connection")
	bool IsOpen() const;

	UFUNCTION(BlueprintPure, Category = "BertaSerial|Connection")
	FString GetPortName() const;

	UFUNCTION(BlueprintPure, Category = "BertaSerial|Connection")
	FBertaSerialOpenOptions GetOpenOptions() const;

	/** Queues exact bytes without waiting for hardware transmission. */
	UFUNCTION(BlueprintCallable, Category = "BertaSerial|Write")
	bool WriteBytes(const TArray<uint8>& Data);

	/** Queues Text encoded as UTF-8, without a null byte or line ending. */
	UFUNCTION(BlueprintCallable, Category = "BertaSerial|Write", meta = (DisplayName = "Write String UTF8"))
	bool WriteStringUtf8(const FString& Text);

	/** Appends exactly the selected terminator, encodes as UTF-8, and queues it. */
	UFUNCTION(BlueprintCallable, Category = "BertaSerial|Write", meta = (DisplayName = "Write Line UTF8"))
	bool WriteLineUtf8(const FString& Text, EBertaSerialLineEnding LineEnding = EBertaSerialLineEnding::LF);

	UFUNCTION(BlueprintCallable, Category = "BertaSerial|Connection")
	bool Close();

private:
	friend class FBertaSerialDispatcher;
	friend class UBertaSerialSubsystem;

	enum class EState : uint8
	{
		Closed,
		Open,
		Closing
	};

	bool InitializeInternal(
		const FBertaSerialOpenOptions& Options,
		UBertaSerialSubsystem* InOwnerSubsystem,
		EBertaSerialOpenError& OutError,
		FString& OutErrorMessage);
	void HandleNativeBytes(TArray<uint8> Data);
	void HandleNativeClosed(EBertaSerialCloseReason Reason, FString ErrorMessage);
	void ShutdownForOwner();
	void SuppressCallbacksAndReleaseWorker();

	UPROPERTY(Transient)
	FBertaSerialOpenOptions OpenOptions;

	TWeakObjectPtr<UBertaSerialSubsystem> OwnerSubsystem;
	TSharedPtr<FBertaSerialDispatcher, ESPMode::ThreadSafe> Dispatcher;
	TSharedPtr<FBertaWindowsSerialWorker> Worker;
	EState State = EState::Closed;
};
