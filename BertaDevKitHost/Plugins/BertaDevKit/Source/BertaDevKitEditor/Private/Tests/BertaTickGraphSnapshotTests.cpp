#include "TickGraph/BertaTickGraphSnapshot.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace
{
	BertaTickGraph::FNode Node(const TCHAR* Path, BertaTickGraph::ERole Role = BertaTickGraph::ERole::ExternalPrerequisite)
	{
		BertaTickGraph::FNode Result;
		Result.Name = Path;
		Result.OwnerName = Path;
		Result.OwnerPath = Path;
		Result.Role = Role;
		return Result;
	}

	bool HasEdge(const BertaTickGraph::FSnapshot& Snapshot, int32 From, int32 To)
	{
		for (const BertaTickGraph::FEdge& Edge : Snapshot.Edges)
		{
			if (Edge.Prerequisite == From && Edge.Dependent == To) { return true; }
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaTickGraphTopologyTest, "BertaDevKit.TickGraph.Topology", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaTickGraphTopologyTest::RunTest(const FString& Parameters)
{
	using namespace BertaTickGraph;
	FBuilder Builder;
	const int32 A = Builder.AddNode(Node(TEXT("ExternalA")));
	const int32 B = Builder.AddNode(Node(TEXT("ExternalB")));
	const int32 C = Builder.AddNode(Node(TEXT("ActorSeed"), ERole::TargetActorPrimary));
	const int32 D = Builder.AddNode(Node(TEXT("ComponentSeed"), ERole::TargetComponentPrimary));
	Builder.Data().SeedCount = 2;
	Builder.Data().ExternalCount = 2;
	Builder.AddEdge(A, B);
	Builder.AddEdge(B, C); // Transitive chain.
	Builder.AddEdge(A, C); // Shared prerequisite and multiple prerequisites.
	Builder.AddEdge(A, D); // Shared prerequisite across several selected seeds.
	Builder.AddEdge(C, D); // A selected seed may precede another selected seed.
	const FSnapshot Snapshot = Builder.Finish();
	TestEqual(TEXT("Shared Tick Function appears once"), Snapshot.Nodes.Num(), 4);
	TestEqual(TEXT("All distinct prerequisite relations remain"), Snapshot.Edges.Num(), 5);
	TestTrue(TEXT("Single and transitive arrows point prerequisite to dependent"), HasEdge(Snapshot, A, B) && HasEdge(Snapshot, B, C));
	TestTrue(TEXT("Multiple prerequisites reach one dependent"), HasEdge(Snapshot, A, C) && HasEdge(Snapshot, B, C));
	TestTrue(TEXT("Several seeds and seed-to-seed relation are retained"), HasEdge(Snapshot, A, D) && HasEdge(Snapshot, C, D));
	TestEqual(TEXT("Two primary seeds"), Snapshot.SeedCount, 2);
	TestTrue(TEXT("External prerequisites classified"), Snapshot.Nodes[A].Role == ERole::ExternalPrerequisite);
	for (const BertaTickGraph::FEdge& Edge : Snapshot.Edges)
	{
		TestTrue(TEXT("DAG prerequisites lay left of dependents"), Snapshot.Nodes[Edge.Prerequisite].Layer < Snapshot.Nodes[Edge.Dependent].Layer);
	}
	FBuilder Repeat;
	for (const FNode& Original : Snapshot.Nodes) { Repeat.AddNode(Node(*Original.OwnerPath, Original.Role)); }
	for (const BertaTickGraph::FEdge& Edge : Snapshot.Edges) { Repeat.AddEdge(Edge.Prerequisite, Edge.Dependent); }
	const FSnapshot Again = Repeat.Finish();
	for (int32 Index = 0; Index < Snapshot.Nodes.Num(); ++Index)
	{
		TestEqual(TEXT("Stable layer"), Snapshot.Nodes[Index].Layer, Again.Nodes[Index].Layer);
		TestEqual(TEXT("Stable row"), Snapshot.Nodes[Index].Row, Again.Nodes[Index].Row);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaTickGraphSafetyTest, "BertaDevKit.TickGraph.SafetyLimitsAndStale", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaTickGraphSafetyTest::RunTest(const FString& Parameters)
{
	using namespace BertaTickGraph;
	FBuilder Builder({2, 1, 1});
	const int32 A = Builder.AddNode(Node(TEXT("A")));
	const int32 B = Builder.AddNode(Node(TEXT("B")));
	TestEqual(TEXT("Node cap rejects extra node"), Builder.AddNode(Node(TEXT("C"))), INDEX_NONE);
	TestTrue(TEXT("First edge fits"), Builder.AddEdge(A, B));
	TestFalse(TEXT("Edge cap rejects second edge"), Builder.AddEdge(B, A));
	TestTrue(TEXT("First visit enters"), Builder.Enter(A, 0));
	TestTrue(TEXT("Next visit enters"), Builder.Enter(B, 1));
	TestFalse(TEXT("Active recursion cycle terminates"), Builder.Enter(A, 1));
	TestFalse(TEXT("Depth cap terminates"), Builder.Enter(B, 2));
	Builder.Leave(B);
	Builder.Leave(A);
	FSnapshot Snapshot = Builder.Finish();
	TestTrue(TEXT("Cycle surfaced"), Snapshot.bCycle);
	TestTrue(TEXT("Limits surfaced"), Snapshot.bTruncated);
	TestEqual(TEXT("Partial graph retained"), Snapshot.Nodes.Num(), 2);
	TestEqual(TEXT("Partial edge retained"), Snapshot.Edges.Num(), 1);
	const FString CopiedName = Snapshot.Nodes[0].Name;
	Snapshot.bStale = true;
	TestEqual(TEXT("Stale marking retains copied details"), Snapshot.Nodes[0].Name, CopiedName);
	TestEqual(TEXT("Stale marking retains structural edges"), Snapshot.Edges.Num(), 1);
	FBuilder CycleBuilder;
	const int32 CycleA = CycleBuilder.AddNode(Node(TEXT("CycleA")));
	const int32 CycleB = CycleBuilder.AddNode(Node(TEXT("CycleB")));
	const int32 CycleC = CycleBuilder.AddNode(Node(TEXT("CycleC")));
	CycleBuilder.AddEdge(CycleA, CycleB);
	CycleBuilder.AddEdge(CycleB, CycleC);
	CycleBuilder.AddEdge(CycleC, CycleA);
	TestTrue(TEXT("A enters"), CycleBuilder.Enter(CycleA, 0));
	TestTrue(TEXT("B enters"), CycleBuilder.Enter(CycleB, 1));
	TestTrue(TEXT("C enters"), CycleBuilder.Enter(CycleC, 2));
	TestFalse(TEXT("A → B → C → A stops recursion"), CycleBuilder.Enter(CycleA, 3));
	CycleBuilder.Leave(CycleC);
	CycleBuilder.Leave(CycleB);
	CycleBuilder.Leave(CycleA);
	const FSnapshot Cycle = CycleBuilder.Finish();
	TestTrue(TEXT("Cyclic topology is reported"), Cycle.bCycle);
	TestEqual(TEXT("Cycle edges remain visible"), Cycle.Edges.Num(), 3);
	return true;
}

#endif
