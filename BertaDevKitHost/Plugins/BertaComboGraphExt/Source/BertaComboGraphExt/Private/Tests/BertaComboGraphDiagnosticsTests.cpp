#if WITH_DEV_AUTOMATION_TESTS

#include "Diagnostics/BertaComboGraphEffectContextCompatibility.h"
#include "Diagnostics/BertaComboGraphTraceComponent.h"
#include "Diagnostics/BertaComboGraphTraceRules.h"
#include "Input/BertaComboGraphInputBufferComponent.h"
#include "Input/BertaComboGraphInputBufferRules.h"
#include "Targeting/BertaComboGraphTargetingRules.h"

#include "Abilities/ComboGraphAbilitySystemGlobals.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemGlobals.h"
#include "ComboGraphAbilityTypes.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComboGraphCompatibilityClassificationTest,
	"Berta.ComboGraph.Compatibility.Classification",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComboGraphCompatibilityClassificationTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Combo Graph globals are compatible"),
		UBertaComboGraphEffectContextCompatibilityLibrary::ClassifyGlobalsClass(UComboGraphAbilitySystemGlobals::StaticClass()),
		EBertaComboGraphEffectContextCompatibility::Compatible);
	TestEqual(TEXT("Parallel base globals are incompatible"),
		UBertaComboGraphEffectContextCompatibilityLibrary::ClassifyGlobalsClass(UAbilitySystemGlobals::StaticClass()),
		EBertaComboGraphEffectContextCompatibility::Incompatible);
	TestEqual(TEXT("Missing class remains unknown"),
		UBertaComboGraphEffectContextCompatibilityLibrary::ClassifyGlobalsClass(nullptr),
		EBertaComboGraphEffectContextCompatibility::Unknown);
	TestEqual(TEXT("Combo Graph context is compatible"),
		UBertaComboGraphEffectContextCompatibilityLibrary::ClassifyContextStruct(FComboGraphGameplayEffectContext::StaticStruct()),
		EBertaComboGraphEffectContextCompatibility::Compatible);
	TestEqual(TEXT("Base context is incompatible"),
		UBertaComboGraphEffectContextCompatibilityLibrary::ClassifyContextStruct(FGameplayEffectContext::StaticStruct()),
		EBertaComboGraphEffectContextCompatibility::Incompatible);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComboGraphTraceFormattingTest,
	"Berta.ComboGraph.Trace.Formatting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComboGraphTraceFormattingTest::RunTest(const FString& Parameters)
{
	FBertaComboGraphTraceEvent Event;
	Event.Type = EBertaComboGraphTraceEventType::TransitionObserved;
	Event.Observation = EBertaComboGraphObservationKind::Observed;
	Event.bInferred = false;
	Event.ExecutionId = TEXT("Task|Graph");
	Event.PreviousNodePath = TEXT("Old");
	Event.CurrentNodePath = TEXT("New");
	Event.ResolvedEdgePath = TEXT("Edge");
	const FString Text = UBertaComboGraphTraceComponent::FormatTraceEvent(Event);
	TestTrue(TEXT("Observation confidence is explicit"), Text.Contains(TEXT("[Observed]")));
	TestTrue(TEXT("Transition endpoints are deterministic"), Text.Contains(TEXT("Previous=Old Current=New")));
	TestTrue(TEXT("Resolved topology edge is explicit"), Text.Contains(TEXT("ResolvedEdge=Edge")));
	FBertaComboGraphStateSnapshot Snapshot;
	Snapshot.ActorPath = TEXT("Actor");
	Snapshot.GraphPath = TEXT("Graph");
	Snapshot.ActiveGraphCount = 2;
	TestTrue(TEXT("Snapshot formatting includes execution count"), UBertaComboGraphTraceComponent::FormatStateSnapshot(Snapshot).Contains(TEXT("ActiveGraphs=2")));

	TArray<FBertaComboGraphTraceEvent> BoundedEvents;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FBertaComboGraphTraceEvent BoundedEvent;
		BoundedEvent.Message = FString::FromInt(Index);
		BertaComboGraphTraceRules::AppendBounded(BoundedEvents, MoveTemp(BoundedEvent), 2);
	}
	TestEqual(TEXT("Trace capacity is bounded"), BoundedEvents.Num(), 2);
	TestEqual(TEXT("Bounded trace evicts oldest first"), BoundedEvents[0].Message, FString(TEXT("1")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComboGraphInputBufferPolicyTest,
	"Berta.ComboGraph.InputBuffer.Policy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComboGraphInputBufferPolicyTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Triggered is supported"), UBertaComboGraphInputBufferComponent::CanConsumeWithStockInputPath(ETriggerEvent::Triggered));
	TestFalse(TEXT("Started is not reconstructed"), UBertaComboGraphInputBufferComponent::CanConsumeWithStockInputPath(ETriggerEvent::Started));
	TestFalse(TEXT("Canceled is not buffered"), UBertaComboGraphInputBufferComponent::CanConsumeWithStockInputPath(ETriggerEvent::Canceled));
	TestFalse(TEXT("Boundary is still valid"), UBertaComboGraphInputBufferComponent::IsExpired(1.0f, 1.2f, 0.2f));
	TestTrue(TEXT("Past duration expires"), UBertaComboGraphInputBufferComponent::IsExpired(1.0f, 1.201f, 0.2f));
	TestEqual(TEXT("Oldest unexpired input is selected deterministically"), BertaComboGraphInputBufferRules::SelectOldestUnexpired({0.7f, 0.9f}, 1.0f, 0.2f), 1);
	TestEqual(TEXT("Expired buffer has no candidate"), BertaComboGraphInputBufferRules::SelectOldestUnexpired({0.7f}, 1.0f, 0.2f), INDEX_NONE);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComboGraphTargetingDedupeTest,
	"Berta.ComboGraph.Targeting.StableDedupe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComboGraphTargetingDedupeTest::RunTest(const FString& Parameters)
{
	AActor* FirstActor = NewObject<AActor>();
	AActor* SecondActor = NewObject<AActor>();
	FHitResult FirstHit(FirstActor, nullptr, FVector(1.0, 0.0, 0.0), FVector::UpVector);
	FHitResult DuplicateHit(FirstActor, nullptr, FVector(2.0, 0.0, 0.0), FVector::UpVector);
	FHitResult SecondHit(SecondActor, nullptr, FVector(3.0, 0.0, 0.0), FVector::UpVector);
	TArray<FHitResult> UniqueHits;
	BertaComboGraphTargetingRules::StableUniqueActorHits({FirstHit, DuplicateHit, SecondHit}, UniqueHits);
	TestEqual(TEXT("Each actor is emitted once"), UniqueHits.Num(), 2);
	TestEqual(TEXT("First result wins for duplicate actor"), UniqueHits[0].ImpactPoint, FirstHit.ImpactPoint);
	TestEqual(TEXT("Preset result order is retained"), UniqueHits[1].GetActor(), SecondActor);
	return true;
}

#endif
