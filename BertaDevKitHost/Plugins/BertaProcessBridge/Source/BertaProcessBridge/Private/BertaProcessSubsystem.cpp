#include "BertaProcessSubsystem.h"

#include "BertaProcess.h"
#include "BertaProcessBridge.h"
#include "HAL/FileManager.h"

void UBertaProcessSubsystem::Deinitialize()
{
	bIsShuttingDown = true;

	TArray<TObjectPtr<UBertaProcess>> ProcessesToStop = MoveTemp(ActiveProcesses);
	ActiveProcesses.Reset();
	for (UBertaProcess* Process : ProcessesToStop)
	{
		if (Process != nullptr)
		{
			Process->ShutdownForOwner();
		}
	}

	Super::Deinitialize();
}

bool UBertaProcessSubsystem::LaunchProcess(
	const FBertaProcessLaunchOptions& Options,
	UBertaProcess*& OutProcess,
	EBertaProcessLaunchError& OutError,
	FString& OutErrorMessage)
{
	OutProcess = nullptr;
	OutError = EBertaProcessLaunchError::None;
	OutErrorMessage.Reset();

	if (!IsInGameThread() || bIsShuttingDown)
	{
		OutError = EBertaProcessLaunchError::SubsystemUnavailable;
		OutErrorMessage = TEXT("The BertaProcessBridge GameInstance subsystem is unavailable or shutting down.");
		return false;
	}

	if (Options.ExecutablePath.IsEmpty())
	{
		OutError = EBertaProcessLaunchError::InvalidExecutable;
		OutErrorMessage = TEXT("ExecutablePath must not be empty.");
		return false;
	}

	if (!Options.WorkingDirectory.IsEmpty()
		&& !IFileManager::Get().DirectoryExists(*Options.WorkingDirectory))
	{
		OutError = EBertaProcessLaunchError::InvalidWorkingDirectory;
		OutErrorMessage = FString::Printf(
			TEXT("Working directory does not exist: '%s'."),
			*Options.WorkingDirectory);
		return false;
	}

	UBertaProcess* Process = NewObject<UBertaProcess>(this);
	if (!Process->LaunchInternal(Options, this))
	{
		OutError = EBertaProcessLaunchError::LaunchFailed;
		OutErrorMessage = Options.WorkingDirectory.IsEmpty()
			? FString::Printf(TEXT("Failed to launch executable '%s'."), *Options.ExecutablePath)
			: FString::Printf(
				TEXT("Failed to launch executable '%s' in working directory '%s'."),
				*Options.ExecutablePath,
				*Options.WorkingDirectory);
		UE_LOG(
			LogBertaProcessBridge,
			Warning,
			TEXT("%s"),
			*OutErrorMessage);
		return false;
	}

	ActiveProcesses.Add(Process);
	OutProcess = Process;
	return true;
}

void UBertaProcessSubsystem::NotifyProcessFinished(UBertaProcess* Process)
{
	check(IsInGameThread());
	ActiveProcesses.RemoveSingleSwap(Process);
}
