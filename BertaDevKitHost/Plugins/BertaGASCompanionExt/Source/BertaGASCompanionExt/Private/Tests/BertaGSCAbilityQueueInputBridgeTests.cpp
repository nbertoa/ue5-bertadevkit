#include "Components/BertaGSCAbilityQueueInputBridgeInternal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Abilities/GameplayAbility.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGSCAbilityQueueNativeEndContractTest,
	"BertaDevKit.GASCompanionExt.AbilityQueueInputBridge.NativeEndContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGSCAbilityQueueNativeEndContractTest::RunTest(const FString& Parameters)
{
	// GSC owns one queue, without an opener spec/activation identity. The first
	// ended ability consumes it, independent of the order of concurrent abilities.
	auto CheckFirstEndConsumesQueue = [this](const TCHAR* Scenario, bool bReverseOrder)
	{
		UGSCAbilityQueueComponent* Queue = NewObject<UGSCAbilityQueueComponent>();
		UGameplayAbility* Queued = NewObject<UGameplayAbility>();
		UGameplayAbility* First = NewObject<UGameplayAbility>();
		UGameplayAbility* Second = NewObject<UGameplayAbility>();
		Queue->bAbilityQueueEnabled = true;
		Queue->OpenAbilityQueue();
		Queue->SetAllowAllAbilitiesForAbilityQueue(true);
		Queue->OnAbilityFailed(Queued, FGameplayTagContainer());
		TestTrue(FString::Printf(TEXT("%s: ability queued"), Scenario), Queue->GetCurrentQueuedAbility() == Queued);
		Queue->OnAbilityEnded(bReverseOrder ? Second : First);
		TestNull(FString::Printf(TEXT("%s: first end consumes native queue"), Scenario), Queue->GetCurrentQueuedAbility());
		Queue->OnAbilityEnded(bReverseOrder ? First : Second);
		TestNull(FString::Printf(TEXT("%s: later end does not revive queue"), Scenario), Queue->GetCurrentQueuedAbility());
	};

	CheckFirstEndConsumesQueue(TEXT("expected order"), false);
	CheckFirstEndConsumesQueue(TEXT("reverse order / unrelated ability first"), true);
	return true;
}

#endif
