#include "BertaSerialBlueprintLibrary.h"

#include "BertaSerialPort.h"
#include "BertaSerialSubsystem.h"
#include "BertaSerialUtils.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Windows/BertaWindowsSerialEnumeration.h"

bool UBertaSerialBlueprintLibrary::GetSerialPorts(
	TArray<FBertaSerialPortInfo>& OutPorts,
	FString& OutErrorMessage)
{
	return FBertaWindowsSerialEnumeration::GetPorts(OutPorts, OutErrorMessage);
}

bool UBertaSerialBlueprintLibrary::OpenSerialPort(
	const UObject* WorldContextObject,
	const FBertaSerialOpenOptions& Options,
	UBertaSerialPort*& OutPort,
	EBertaSerialOpenError& OutError,
	FString& OutErrorMessage)
{
	OutPort = nullptr;
	OutError = EBertaSerialOpenError::None;
	OutErrorMessage.Reset();

	if (!IsInGameThread() || GEngine == nullptr || WorldContextObject == nullptr)
	{
		OutError = EBertaSerialOpenError::InvalidWorldContext;
		OutErrorMessage = TEXT("A valid WorldContextObject on the Game Thread is required.");
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
	if (GameInstance == nullptr)
	{
		OutError = EBertaSerialOpenError::InvalidWorldContext;
		OutErrorMessage = TEXT("No GameInstance is available for the supplied world context.");
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

	UBertaSerialSubsystem* Subsystem = GameInstance->GetSubsystem<UBertaSerialSubsystem>();
	if (Subsystem == nullptr)
	{
		OutError = EBertaSerialOpenError::SubsystemUnavailable;
		OutErrorMessage = TEXT("No BertaSerial subsystem is available for the supplied GameInstance.");
		return false;
	}
	return Subsystem->OpenPort(Options, OutPort, OutError, OutErrorMessage);
}
