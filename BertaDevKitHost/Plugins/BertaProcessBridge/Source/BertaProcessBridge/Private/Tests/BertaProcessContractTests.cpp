#include "BertaProcessBlueprintLibrary.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "BertaProcess.h"
#include "BertaProcessSubsystem.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaProcessInvalidContextTest,
	"BertaProcessBridge.Launch.InvalidContextClearsOutputs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaProcessInvalidContextTest::RunTest(const FString& Parameters)
{
	UBertaProcess* OutProcess = NewObject<UBertaProcess>();
	EBertaProcessLaunchError OutError = EBertaProcessLaunchError::LaunchFailed;
	FString OutMessage = TEXT("Previous error");
	TestFalse(TEXT("Launch fails without context"),
		UBertaProcessBlueprintLibrary::LaunchProcess(nullptr, {}, OutProcess, OutError, OutMessage));
	TestNull(TEXT("Failure clears the process output"), OutProcess);
	TestTrue(TEXT("Failure reports unavailable subsystem"), OutError == EBertaProcessLaunchError::SubsystemUnavailable);
	TestEqual(TEXT("Failure replaces the previous message"), OutMessage,
		FString(TEXT("A valid WorldContextObject and engine are required.")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaProcessSubsystemShutdownTest,
	"BertaProcessBridge.Lifecycle.InvalidOptionsAndShutdownRejectLaunch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaProcessSubsystemShutdownTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UBertaProcessSubsystem* Subsystem = NewObject<UBertaProcessSubsystem>(GameInstance);
	UBertaProcess* OutProcess = NewObject<UBertaProcess>(Subsystem);
	EBertaProcessLaunchError OutError = EBertaProcessLaunchError::LaunchFailed;
	FString OutMessage = TEXT("Previous error");
	TestFalse(TEXT("Empty executable fails before native launch"),
		Subsystem->LaunchProcess({}, OutProcess, OutError, OutMessage));
	TestNull(TEXT("Invalid options clear the process output"), OutProcess);
	TestTrue(TEXT("Empty executable has an explicit error"), OutError == EBertaProcessLaunchError::InvalidExecutable);
	TestEqual(TEXT("Invalid options replace the previous message"), OutMessage,
		FString(TEXT("ExecutablePath must not be empty.")));

	Subsystem->Deinitialize();
	OutProcess = NewObject<UBertaProcess>(Subsystem);
	OutError = EBertaProcessLaunchError::None;
	OutMessage = TEXT("Previous error");
	TestFalse(TEXT("Launch is rejected after deinitialization"),
		Subsystem->LaunchProcess({}, OutProcess, OutError, OutMessage));
	TestNull(TEXT("Shutdown clears the process output"), OutProcess);
	TestTrue(TEXT("Shutdown is checked before executable validation"), OutError == EBertaProcessLaunchError::SubsystemUnavailable);
	TestEqual(TEXT("Shutdown replaces the previous message"), OutMessage,
		FString(TEXT("The BertaProcessBridge GameInstance subsystem is unavailable or shutting down.")));
	return true;
}

#endif
