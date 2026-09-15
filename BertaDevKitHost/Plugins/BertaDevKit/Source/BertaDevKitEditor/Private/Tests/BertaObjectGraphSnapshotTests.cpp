#include "ObjectGraph/BertaObjectGraphSnapshot.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace
{
	BertaObjectGraph::FStep Step(const TCHAR* Path, const TCHAR* Reason = TEXT(""))
	{
		BertaObjectGraph::FStep Result;
		Result.Object.Path = Path;
		Result.Object.Name = Path;
		Result.Object.ClassName = TEXT("TestClass");
		Result.ReasonToNext = Reason;
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaObjectGraphSnapshotTest, "BertaDevKit.ObjectGraph.DetachedSnapshot", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaObjectGraphSnapshotTest::RunTest(const FString& Parameters)
{
	using namespace BertaObjectGraph;
	TArray<FChain> Chains;
	Chains.Add({Step(TEXT("RootB"), TEXT("Property: ToShared")), Step(TEXT("Shared"), TEXT("Property: Target")), Step(TEXT("Target"))});
	Chains.Add({Step(TEXT("RootA"), TEXT("AddReferencedObjects")), Step(TEXT("Shared"), TEXT("Property: Target")), Step(TEXT("Target"))});
	Chains.Add({Step(TEXT("RootA"), TEXT("Property: Other")), Step(TEXT("Shared"), TEXT("Property: Target")), Step(TEXT("Target"))});
	const FSnapshot A = BuildSnapshot(Chains, Chains.Num());
	const FSnapshot B = BuildSnapshot(Chains, Chains.Num());
	TestEqual(TEXT("Shared objects have one node"), A.Nodes.Num(), 4);
	TestEqual(TEXT("Distinct reasons between the same objects remain separate edges"), A.Edges.Num(), 4);
	TestEqual(TEXT("All chains displayed"), A.DisplayedChains, 3);
	int32 TargetCount = 0;
	for (const FNode& Node : A.Nodes)
	{
		if (Node.Role == ERole::Root) { TestEqual(TEXT("Root is left"), Node.Layer, 0); }
		if (Node.Role == ERole::Target) { ++TargetCount; TestTrue(TEXT("Target is rightmost"), Node.Layer > 0); }
		TestEqual(TEXT("Stable layer"), Node.Layer, B.Nodes[Node.Id].Layer);
		TestEqual(TEXT("Stable row"), Node.Row, B.Nodes[Node.Id].Row);
	}
	TestEqual(TEXT("One target"), TargetCount, 1);
	for (const BertaObjectGraph::FEdge& Edge : A.Edges)
	{
		TestTrue(TEXT("Arrows point toward target"), A.Nodes[Edge.From].Layer < A.Nodes[Edge.To].Layer);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaObjectGraphEmptyAndCapsTest, "BertaDevKit.ObjectGraph.EmptyAndCaps", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaObjectGraphEmptyAndCapsTest::RunTest(const FString& Parameters)
{
	using namespace BertaObjectGraph;
	const FSnapshot Empty = BuildSnapshot({}, 0);
	TestTrue(TEXT("No chain gives an empty graph"), Empty.Nodes.IsEmpty() && Empty.Edges.IsEmpty());
	TestFalse(TEXT("Empty result is not truncated"), Empty.bTruncated);
	const FSnapshot DirectRoot = BuildSnapshot({FChain{Step(TEXT("Target"))}}, 0);
	TestEqual(TEXT("Direct root is a single visible target"), DirectRoot.Nodes.Num(), 1);
	TestEqual(TEXT("Direct root does not claim a native chain"), DirectRoot.NativeChains, 0);

	TArray<FChain> Chains;
	for (int32 Index = MaxDisplayedChains + 5; Index >= 0; --Index)
	{
		FChain Chain;
		Chain.Add(Step(*FString::Printf(TEXT("Root%03d"), Index), TEXT("ref")));
		Chain.Add(Step(TEXT("Target")));
		Chains.Add(MoveTemp(Chain));
	}
	const FSnapshot Snapshot = BuildSnapshot(Chains, Chains.Num());
	TestEqual(TEXT("Visible chain cap"), Snapshot.DisplayedChains, MaxDisplayedChains);
	TestTrue(TEXT("Cap is reported"), Snapshot.bTruncated);
	TestEqual(TEXT("Stable shortest lexical path is first"), Snapshot.Nodes[0].Object.Path, FString(TEXT("Root000")));

	TArray<FChain> WideChains;
	FChain Wide;
	for (int32 Index = 0; Index < MaxDisplayedNodes + 1; ++Index)
	{
		Wide.Add(Step(*FString::Printf(TEXT("Node%03d"), Index), TEXT("ref")));
	}
	WideChains.Add(MoveTemp(Wide));
	WideChains.Add({Step(TEXT("SmallRoot"), TEXT("ref")), Step(TEXT("SmallTarget"))});
	const FSnapshot NodeCapped = BuildSnapshot(WideChains, WideChains.Num());
	TestEqual(TEXT("Oversized path is omitted"), NodeCapped.DisplayedChains, 1);
	TestTrue(TEXT("Node cap is reported"), NodeCapped.bTruncated);
	TestEqual(TEXT("Small path still fits"), NodeCapped.Nodes.Num(), 2);
	return true;
}

#endif
