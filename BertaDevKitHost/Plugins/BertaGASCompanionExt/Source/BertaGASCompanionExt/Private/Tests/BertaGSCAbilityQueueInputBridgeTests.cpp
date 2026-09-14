#include "Components/BertaGSCAbilityQueueInputBridgeInternal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGSCAbilityQueueInputBridgeDecisionTest,
	"BertaDevKit.GASCompanionExt.AbilityQueueInputBridge.Decision",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGSCAbilityQueueInputBridgeDecisionTest::RunTest(const FString& Parameters)
{
	using namespace BertaGSCAbilityQueueInputBridgePrivate;

	FInputFailureContext Context;
	Context.bBridgeActive = true;
	Context.bRequestValid = true;
	Context.bQueueEnabledAndOpen = true;
	Context.bRuntimeBindingMatches = true;
	Context.bAbilityAllowed = true;
	TestEqual(TEXT("Explicit matching input failure is accepted"), EvaluateInputFailure(Context), EInputFailureDecision::Accept);

	Context.bRuntimeBindingMatches = false;
	TestEqual(TEXT("Non-input or mismatched input failure is rejected"), EvaluateInputFailure(Context), EInputFailureDecision::BindingMismatch);
	Context.bRuntimeBindingMatches = true;
	Context.bQueueEnabledAndOpen = false;
	TestEqual(TEXT("Closed queue rejects forwarding"), EvaluateInputFailure(Context), EInputFailureDecision::QueueUnavailable);
	Context.bQueueEnabledAndOpen = true;
	Context.bAbilityAllowed = false;
	TestEqual(TEXT("Disallowed ability rejects forwarding"), EvaluateInputFailure(Context), EInputFailureDecision::AbilityNotAllowed);
	return true;
}

#endif
