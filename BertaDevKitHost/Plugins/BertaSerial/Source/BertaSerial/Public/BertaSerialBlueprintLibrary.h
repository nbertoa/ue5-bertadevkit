#pragma once

#include "BertaSerialTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaSerialBlueprintLibrary.generated.h"

class UBertaSerialPort;

UCLASS()
class BERTASERIAL_API UBertaSerialBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BertaSerial|Ports", meta = (ReturnDisplayName = "Success"))
	static bool GetSerialPorts(TArray<FBertaSerialPortInfo>& OutPorts, FString& OutErrorMessage);

	UFUNCTION(
		BlueprintCallable,
		Category = "BertaSerial|Connection",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Open Serial Port", ReturnDisplayName = "Success"))
	static bool OpenSerialPort(
		const UObject* WorldContextObject,
		const FBertaSerialOpenOptions& Options,
		UBertaSerialPort*& OutPort,
		EBertaSerialOpenError& OutError,
		FString& OutErrorMessage);
};
