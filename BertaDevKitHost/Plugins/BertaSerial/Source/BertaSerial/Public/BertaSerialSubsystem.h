#pragma once

#include "BertaSerialTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "BertaSerialSubsystem.generated.h"

class UBertaSerialPort;

/** Owns open serial ports for exactly one GameInstance lifetime. */
UCLASS()
class BERTASERIAL_API UBertaSerialSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	bool OpenPort(
		const FBertaSerialOpenOptions& Options,
		UBertaSerialPort*& OutPort,
		EBertaSerialOpenError& OutError,
		FString& OutErrorMessage);

private:
	friend class UBertaSerialPort;

	void NotifyPortClosed(UBertaSerialPort* Port);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBertaSerialPort>> ActivePorts;

	bool bIsShuttingDown = false;
};
