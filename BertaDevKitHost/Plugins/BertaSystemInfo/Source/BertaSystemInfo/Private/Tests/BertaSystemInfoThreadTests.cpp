#include "BertaSystemInfoThreadTestContext.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AudioThread.h"
#include "BertaSystemInfoBlueprintLibrary.h"
#include "Engine/Engine.h"
#include "HAL/Event.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaSystemInfoAudioOutputThreadTest,
	"BertaSystemInfo.Audio.OutputDevicesRejectAudioThreadBeforeWorldLookup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaSystemInfoAudioOutputThreadTest::RunTest(const FString& Parameters)
{
	if (!TestNotNull(TEXT("Engine is available"), GEngine))
	{
		return false;
	}

	UBertaSystemInfoThreadTestContext* Context = NewObject<UBertaSystemInfoThreadTestContext>();
	TArray<FBertaAudioOutputDeviceInfo> GameThreadDevices;
	TestFalse(TEXT("A context without a world is rejected on the Game Thread"),
		UBertaSystemInfoBlueprintLibrary::GetAudioOutputDevices(Context, GameThreadDevices));
	const int32 GameThreadLookups = Context->WorldLookupCount.GetValue();
	TestEqual(TEXT("The context spy observes the Game Thread world lookup"), GameThreadLookups, 1);

	const bool bWasUsingThreadedAudio = FAudioThread::IsUsingThreadedAudio();
	const bool bStartedAudioThreadForTest = !IsAudioThreadRunning();
	if (bStartedAudioThreadForTest)
	{
		FAudioThread::SetUseThreadedAudio(true);
		FAudioThread::StartAudioThread();
	}

	TestTrue(TEXT("A separate Audio Thread context is running"), IsAudioThreadRunning());
	TestFalse(TEXT("The Game Thread is not the Audio Thread"), IsInAudioThread());

	bool bExecutedOnAudioThread = false;
	bool bAudioThreadResult = true;
	bool bOutputReset = false;
	FEventRef AudioCommandDone;
	FAudioThread::RunCommandOnAudioThread([&]()
	{
		bExecutedOnAudioThread = IsInAudioThread() && !IsInGameThread();
		TArray<FBertaAudioOutputDeviceInfo> AudioThreadDevices;
		AudioThreadDevices.AddDefaulted();
		bAudioThreadResult = UBertaSystemInfoBlueprintLibrary::GetAudioOutputDevices(Context, AudioThreadDevices);
		bOutputReset = AudioThreadDevices.IsEmpty();
		AudioCommandDone->Trigger();
	});
	AudioCommandDone->Wait();
	if (bStartedAudioThreadForTest)
	{
		FAudioThread::StopAudioThread();
		FAudioThread::SetUseThreadedAudio(bWasUsingThreadedAudio);
	}

	TestTrue(TEXT("The call ran on the Audio Thread"), bExecutedOnAudioThread);
	TestFalse(TEXT("The Audio Thread call is rejected"), bAudioThreadResult);
	TestTrue(TEXT("The rejected call clears output"), bOutputReset);
	TestEqual(TEXT("The Audio Thread never resolves the world context"),
		Context->WorldLookupCount.GetValue(), GameThreadLookups);
	return true;
}

#endif
