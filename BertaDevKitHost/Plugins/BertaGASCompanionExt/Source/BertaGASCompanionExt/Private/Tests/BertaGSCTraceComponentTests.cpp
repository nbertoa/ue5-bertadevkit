#include "Components/BertaGSCTraceComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGSCTraceFormattingTest,
	"BertaDevKit.GASCompanionExt.Trace.Formatting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGSCTraceFormattingTest::RunTest(const FString& Parameters)
{
	FBertaGSCTraceEvent Event;
	Event.Type = EBertaGSCTraceEventType::AttributeChanged;
	Event.RelativeTimeSeconds = 1.25f;
	Event.ActorPath = TEXT("/Game/TestActor");
	Event.AttributeName = TEXT("Health");
	Event.bOldValueKnown = true;
	Event.OldValue = 100.0f;
	Event.NewValue = 75.0f;
	Event.DeltaValue = -25.0f;

	const FString Text = UBertaGSCTraceComponent::FormatTraceEvent(Event);
	TestTrue(TEXT("Includes event type"), Text.Contains(TEXT("AttributeChanged")));
	TestTrue(TEXT("Includes actor"), Text.Contains(TEXT("Actor=/Game/TestActor")));
	TestTrue(TEXT("Includes deterministic values"), Text.Contains(TEXT("Old=100.000 New=75.000 Delta=-25.000")));

	Event.bOldValueKnown = false;
	Event.OldValue = -999.0f;
	const FString UnknownText = UBertaGSCTraceComponent::FormatTraceEvent(Event);
	TestTrue(TEXT("Unknown old value is explicit"), UnknownText.Contains(TEXT("Old=unknown New=75.000 Delta=-25.000")));
	TestFalse(TEXT("Unknown old value is never formatted as exact"), UnknownText.Contains(TEXT("Old=-999.000")));
	return true;
}

#endif
