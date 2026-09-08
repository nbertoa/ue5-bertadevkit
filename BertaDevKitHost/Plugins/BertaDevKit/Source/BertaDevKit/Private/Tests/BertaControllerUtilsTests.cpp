#include "Controller/BertaControllerUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaControllerUtilsInvalidVibrationTest,
	"BertaDevKit.Controller.Feedback.InvalidVibrationHandle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaControllerUtilsInvalidVibrationTest::RunTest(const FString& Parameters)
{
	const FBertaControllerVibrationHandle Handle = UBertaControllerUtils::PlayControllerVibration(nullptr);
	TestFalse(TEXT("A null controller returns an invalid vibration handle"),
	          UBertaControllerUtils::IsControllerVibrationHandleValid(Handle));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaControllerUtilsInvalidPropertyHandleTest,
	"BertaDevKit.Controller.DeviceProperties.InvalidHandle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaControllerUtilsInvalidPropertyHandleTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("An invalid device property handle is not active"),
	          UBertaControllerUtils::IsInputDevicePropertyActive(FInputDevicePropertyHandle::InvalidHandle));
	UBertaControllerUtils::RemoveInputDeviceProperty(FInputDevicePropertyHandle::InvalidHandle);
	return true;
}

#endif
