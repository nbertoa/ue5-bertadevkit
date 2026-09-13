#include "BertaWindowGeometry.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

using namespace UE::BertaWindowTools::Private;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaWindowCenteringTest,
	"BertaWindowTools.Geometry.Centering",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaWindowCenteringTest::RunTest(const FString& Parameters)
{
	TestEqual(
		TEXT("Secondary display"),
		CalculateCenteredPosition(FPlatformRect(1920, 0, 3840, 1080), FIntPoint(800, 600)),
		FIntPoint(2480, 240));
	TestEqual(
		TEXT("Negative display origin"),
		CalculateCenteredPosition(FPlatformRect(-1920, 0, 0, 1080), FIntPoint(800, 600)),
		FIntPoint(-1360, 240));
	TestEqual(
		TEXT("Oversized window"),
		CalculateCenteredPosition(FPlatformRect(0, 0, 1280, 720), FIntPoint(1920, 1080)),
		FIntPoint(0, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaWindowDisplaySelectionTest,
	"BertaWindowTools.Geometry.DisplaySelection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaWindowDisplaySelectionTest::RunTest(const FString& Parameters)
{
	TArray<FMonitorInfo> Monitors;
	FMonitorInfo& Primary = Monitors.AddDefaulted_GetRef();
	Primary.ID = TEXT("Primary");
	Primary.DisplayRect = FPlatformRect(0, 0, 1920, 1080);
	FMonitorInfo& Secondary = Monitors.AddDefaulted_GetRef();
	Secondary.ID = TEXT("Secondary");
	Secondary.DisplayRect = FPlatformRect(1920, 0, 3840, 1080);

	TestEqual(
		TEXT("Greatest intersection wins"),
		FindBestMonitorIndex(Monitors, FPlatformRect(1800, 100, 2600, 700)),
		1);
	TestEqual(
		TEXT("Nearest display wins without intersection"),
		FindBestMonitorIndex(Monitors, FPlatformRect(4000, 100, 4200, 300)),
		1);
	TestEqual(
		TEXT("First display wins an equal positive-overlap tie"),
		FindBestMonitorIndex(Monitors, FPlatformRect(1720, 100, 2120, 500)),
		0);
	TestEqual(
		TEXT("No monitor data"),
		FindBestMonitorIndex(TArray<FMonitorInfo>(), FPlatformRect(0, 0, 100, 100)),
		INDEX_NONE);
	return true;
}

#endif
