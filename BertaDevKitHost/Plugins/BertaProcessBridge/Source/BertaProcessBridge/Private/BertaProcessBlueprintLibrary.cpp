#include "BertaProcessBlueprintLibrary.h"

#include "BertaProcess.h"
#include "BertaProcessSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

bool UBertaProcessBlueprintLibrary::LaunchProcess(
	const UObject* WorldContextObject,
	const FBertaProcessLaunchOptions& Options,
	UBertaProcess*& OutProcess,
	EBertaProcessLaunchError& OutError,
	FString& OutErrorMessage)
{
	OutProcess = nullptr;
	OutError = EBertaProcessLaunchError::None;
	OutErrorMessage.Reset();

	if (GEngine == nullptr || WorldContextObject == nullptr)
	{
		OutError = EBertaProcessLaunchError::SubsystemUnavailable;
		OutErrorMessage = TEXT("A valid WorldContextObject and engine are required.");
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(
		WorldContextObject,
		EGetWorldErrorMode::ReturnNull);
	UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
	UBertaProcessSubsystem* Subsystem = GameInstance != nullptr
		? GameInstance->GetSubsystem<UBertaProcessSubsystem>()
		: nullptr;
	if (Subsystem == nullptr)
	{
		OutError = EBertaProcessLaunchError::SubsystemUnavailable;
		OutErrorMessage = TEXT("No BertaProcessBridge subsystem is available for the supplied world context.");
		return false;
	}

	return Subsystem->LaunchProcess(
		Options,
		OutProcess,
		OutError,
		OutErrorMessage);
}
