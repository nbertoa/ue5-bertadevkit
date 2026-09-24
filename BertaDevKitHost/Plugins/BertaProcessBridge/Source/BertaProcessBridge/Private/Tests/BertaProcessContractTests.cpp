#include "BertaProcessBlueprintLibrary.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "BertaProcess.h"
#include "BertaProcessSubsystem.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
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

#if PLATFORM_WINDOWS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaProcessBlockedInputShutdownTest,
	"BertaProcessBridge.Lifecycle.BlockedInputOwnerShutdown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaProcessBlockedInputShutdownTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UBertaProcessSubsystem* Subsystem = NewObject<UBertaProcessSubsystem>(GameInstance);
	FBertaProcessLaunchOptions Options;
	Options.ExecutablePath = FPlatformMisc::GetEnvironmentVariable(TEXT("ComSpec"));
	Options.Arguments = TEXT("/C ping -n 30 127.0.0.1 >nul");
	UBertaProcess* Process = nullptr;
	EBertaProcessLaunchError Error = EBertaProcessLaunchError::None;
	FString ErrorMessage;
	if (!TestTrue(TEXT("Launch child that does not consume stdin"), Subsystem->LaunchProcess(Options, Process, Error, ErrorMessage)))
	{
		return false;
	}

	FString Payload;
	Payload.Reserve(512 * 1024);
	FString Chunk;
	Chunk.Reserve(1024);
	for (int32 Index = 0; Index < 1024; ++Index)
	{
		Chunk.AppendChar(TEXT('x'));
	}
	for (int32 Index = 0; Index < 512; ++Index)
	{
		Payload += Chunk;
	}
	TestFalse(TEXT("Input above queue capacity is rejected atomically"), Process->SendString(Payload + Payload + TEXT("x")));
	TestTrue(TEXT("Bounded input accepts a large message"), Process->SendString(Payload));
	FPlatformProcess::Sleep(0.1f); // Let the worker enter the synchronous pipe write.
	Subsystem->Deinitialize();
	TestFalse(TEXT("Owner teardown stops process input"), Process->SendString(TEXT("after shutdown")));
	TestEqual(TEXT("Owner teardown records cancellation"), Process->GetState(), EBertaProcessState::Canceled);
	FBertaProcessResult Result;
	TestTrue(TEXT("Owner teardown records one terminal result"), Process->TryGetResult(Result));
	TestEqual(TEXT("Owner teardown result is cancellation"), Result.Reason, EBertaProcessFinishReason::Canceled);
	return true;
}
#endif

#endif
