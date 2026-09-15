#include "DelegateInspector/BertaDelegateInspectorSnapshot.h"
#include "DelegateInspector/BertaDelegateInspector.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

namespace
{
	BertaDelegateInspector::FBinding Binding(const TCHAR* Name, const TCHAR* Class, const TCHAR* Function)
	{
		BertaDelegateInspector::FBinding Result;
		Result.Name = Name;
		Result.ClassName = Class;
		Result.Path = Name;
		Result.FunctionName = Function;
		Result.bFunctionResolved = true;
		return Result;
	}

	BertaDelegateInspector::FSnapshot MakeSnapshot()
	{
		using namespace BertaDelegateInspector;
		FSnapshot Snapshot;
		FSource& Component = Snapshot.Sources.AddDefaulted_GetRef();
		Component.Kind = ESourceKind::Component;
		Component.Name = TEXT("BPC_Interaction");
		Component.ClassName = TEXT("BPC_Interaction_C");
		Component.Path = TEXT("/PIE/Door/BPC_Interaction");
		FDelegate& Interaction = Component.Delegates.AddDefaulted_GetRef();
		Interaction.Name = TEXT("OnInteractionStarted");
		Interaction.Bindings.Add(Binding(TEXT("BP_PlayerController_C_0"), TEXT("BP_PlayerController_C"), TEXT("HandleInteractionStarted")));

		FSource& Actor = Snapshot.Sources.AddDefaulted_GetRef();
		Actor.Kind = ESourceKind::Actor;
		Actor.Name = TEXT("BP_Door_C_2");
		Actor.ClassName = TEXT("BP_Door_C");
		Actor.Path = TEXT("/PIE/Door");
		FDelegate& Empty = Actor.Delegates.AddDefaulted_GetRef();
		Empty.Name = TEXT("OnClosed");
		FDelegate& Opened = Actor.Delegates.AddDefaulted_GetRef();
		Opened.Name = TEXT("OnOpened");
		Opened.Bindings.Add(Binding(TEXT("BP_HUD_C_0"), TEXT("BP_HUD_C"), TEXT("OnDoorOpenedB")));
		Opened.Bindings.Add(Binding(TEXT("BP_HUD_C_0"), TEXT("BP_HUD_C"), TEXT("OnDoorOpenedA")));
		FSource& EmptyComponent = Snapshot.Sources.AddDefaulted_GetRef();
		EmptyComponent.Kind = ESourceKind::Component;
		EmptyComponent.Name = TEXT("BPC_Unbound");
		EmptyComponent.ClassName = TEXT("BPC_Unbound_C");
		EmptyComponent.Path = TEXT("/PIE/Door/BPC_Unbound");
		EmptyComponent.Delegates.AddDefaulted_GetRef().Name = TEXT("OnUnused");
		return Snapshot;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaDelegateInspectorModelTest,
	"BertaDevKit.DelegateInspector.OrderCountsAndFilter", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaDelegateInspectorModelTest::RunTest(const FString& Parameters)
{
	using namespace BertaDelegateInspector;
	FSnapshot Snapshot = MakeSnapshot();
	SortAndCount(Snapshot);
	TestEqual(TEXT("Actor sorts before components"), Snapshot.Sources[0].Name, FString(TEXT("BP_Door_C_2")));
	TestEqual(TEXT("Delegate lexical order"), Snapshot.Sources[0].Delegates[0].Name, FString(TEXT("OnClosed")));
	TestEqual(TEXT("Function lexical order"), Snapshot.Sources[0].Delegates[1].Bindings[0].FunctionName, FName(TEXT("OnDoorOpenedA")));
	TestEqual(TEXT("Two functions on one listener remain two bindings"), Snapshot.Sources[0].Delegates[1].Bindings.Num(), 2);
	TestEqual(TEXT("Delegate count"), Snapshot.DelegateCount, 4);
	TestEqual(TEXT("Bound delegate count"), Snapshot.BoundDelegateCount, 2);
	TestEqual(TEXT("Binding count"), Snapshot.BindingCount, 3);
	const TArray<FName> TwoFunctions = AssignFunctionOccurrences(2, {FName(TEXT("FunctionB")), FName(TEXT("FunctionA"))});
	TestEqual(TEXT("Two proven listener functions have separate occurrences"), TwoFunctions.Num(), 2);
	TestEqual(TEXT("Function assignment is lexical"), TwoFunctions[0], FName(TEXT("FunctionA")));
	const TArray<FName> Repeated = AssignFunctionOccurrences(3, {FName(TEXT("FunctionA"))});
	TestEqual(TEXT("One proven function preserves duplicate invocation count"), Repeated[2], FName(TEXT("FunctionA")));
	const TArray<FName> Ambiguous = AssignFunctionOccurrences(3, {FName(TEXT("FunctionB")), FName(TEXT("FunctionA"))});
	TestEqual(TEXT("Ambiguous extra invocation is explicitly unresolved"), Ambiguous[2], NAME_None);

	const TArray<FVisibleSource> Bound = Filter(Snapshot, true, TEXT(""));
	TestEqual(TEXT("Bound Only has both active sources"), Bound.Num(), 2);
	TestEqual(TEXT("Bound Only removes the empty component source"), Bound[1].SourceIndex, 1);
	TestEqual(TEXT("Bound Only removes empty delegate"), Bound[0].Delegates.Num(), 1);
	const TArray<FVisibleSource> All = Filter(Snapshot, false, TEXT(""));
	TestEqual(TEXT("Unbound delegate appears when disabled"), All[0].Delegates.Num(), 2);
	TestEqual(TEXT("Source-name search"), Filter(Snapshot, true, TEXT("door")).Num(), 1);
	TestEqual(TEXT("Delegate-name search"), Filter(Snapshot, true, TEXT("interactionstarted")).Num(), 1);
	TestEqual(TEXT("Listener-class search"), Filter(Snapshot, true, TEXT("BP_HUD_C")).Num(), 1);
	TestEqual(TEXT("Function-name search"), Filter(Snapshot, true, TEXT("OnDoorOpenedB"))[0].Delegates[0].BindingIndices.Num(), 1);
	TestEqual(TEXT("Listener-name search"), Filter(Snapshot, true, TEXT("BP_PlayerController_C_0")).Num(), 1);
	TestEqual(TEXT("Source-class search"), Filter(Snapshot, true, TEXT("BPC_Interaction_C")).Num(), 1);
	TestEqual(TEXT("Filtering does not mutate delegate count"), Snapshot.DelegateCount, 4);
	Snapshot.bStale = true;
	TestEqual(TEXT("Stale marking preserves copied binding"), Snapshot.Sources[0].Delegates[1].Bindings[0].FunctionName,
		FName(TEXT("OnDoorOpenedA")));
	return true;
}

#endif
