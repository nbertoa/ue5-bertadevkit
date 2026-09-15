#include "TickGraph/BertaTickGraphSnapshot.h"

#include "Math/UnrealMathUtility.h"

namespace BertaTickGraph
{
	int32 FBuilder::AddNode(FNode Node)
	{
		if (Snapshot.Nodes.Num() >= Limits.Nodes) { MarkTruncated(); return INDEX_NONE; }
		Node.Id = Snapshot.Nodes.Num();
		return Snapshot.Nodes.Add(MoveTemp(Node));
	}

	bool FBuilder::AddEdge(int32 Prerequisite, int32 Dependent)
	{
		if (!Snapshot.Nodes.IsValidIndex(Prerequisite) || !Snapshot.Nodes.IsValidIndex(Dependent)) { return false; }
		for (const FEdge& Edge : Snapshot.Edges)
		{
			if (Edge.Prerequisite == Prerequisite && Edge.Dependent == Dependent) { return true; }
		}
		if (Snapshot.Edges.Num() >= Limits.Edges) { MarkTruncated(); return false; }
		Snapshot.Edges.Add({Prerequisite, Dependent});
		return true;
	}

	bool FBuilder::Enter(int32 NodeId, int32 Depth)
	{
		if (Depth > Limits.Depth) { MarkTruncated(); return false; }
		if (Active.Contains(NodeId)) { Snapshot.bCycle = true; return false; }
		if (Complete.Contains(NodeId)) { return false; }
		Active.Add(NodeId);
		return true;
	}

	void FBuilder::Leave(int32 NodeId)
	{
		Active.Remove(NodeId);
		Complete.Add(NodeId);
	}

	FSnapshot FBuilder::Finish()
	{
		Layout(Snapshot);
		return MoveTemp(Snapshot);
	}

	void Layout(FSnapshot& Snapshot)
	{
		const int32 Count = Snapshot.Nodes.Num();
		TArray<int32> Incoming;
		TArray<bool> Processed;
		Incoming.Init(0, Count);
		Processed.Init(false, Count);
		for (const FEdge& Edge : Snapshot.Edges)
		{
			if (Snapshot.Nodes.IsValidIndex(Edge.Prerequisite) && Snapshot.Nodes.IsValidIndex(Edge.Dependent))
			{
				++Incoming[Edge.Dependent];
			}
		}
		const auto LessNode = [&Snapshot](int32 A, int32 B)
		{
			const FNode& Left = Snapshot.Nodes[A];
			const FNode& Right = Snapshot.Nodes[B];
			const FString LeftKey = Left.OwnerPath + TEXT("|") + Left.Name + TEXT("|") + FString::FromInt(static_cast<int32>(Left.Kind));
			const FString RightKey = Right.OwnerPath + TEXT("|") + Right.Name + TEXT("|") + FString::FromInt(static_cast<int32>(Right.Kind));
			return LeftKey == RightKey ? A < B : LeftKey < RightKey;
		};
		for (int32 Done = 0; Done < Count; ++Done)
		{
			int32 Next = INDEX_NONE;
			for (int32 Id = 0; Id < Count; ++Id)
			{
				if (!Processed[Id] && Incoming[Id] == 0 && (Next == INDEX_NONE || LessNode(Id, Next))) { Next = Id; }
			}
			if (Next == INDEX_NONE)
			{
				Snapshot.bCycle = true;
				for (int32 Id = 0; Id < Count; ++Id)
				{
					if (!Processed[Id] && (Next == INDEX_NONE || LessNode(Id, Next))) { Next = Id; }
				}
			}
			Processed[Next] = true;
			for (const FEdge& Edge : Snapshot.Edges)
			{
				if (Edge.Prerequisite == Next && Snapshot.Nodes.IsValidIndex(Edge.Dependent))
				{
					Snapshot.Nodes[Edge.Dependent].Layer = FMath::Max(Snapshot.Nodes[Edge.Dependent].Layer, Snapshot.Nodes[Next].Layer + 1);
					--Incoming[Edge.Dependent];
				}
			}
		}
		TArray<int32> Order;
		for (int32 Id = 0; Id < Count; ++Id) { Order.Add(Id); }
		Order.Sort([&](int32 A, int32 B)
		{
			return Snapshot.Nodes[A].Layer == Snapshot.Nodes[B].Layer ? LessNode(A, B)
				: Snapshot.Nodes[A].Layer < Snapshot.Nodes[B].Layer;
		});
		int32 LastLayer = INDEX_NONE;
		int32 Row = 0;
		for (int32 Id : Order)
		{
			if (Snapshot.Nodes[Id].Layer != LastLayer) { LastLayer = Snapshot.Nodes[Id].Layer; Row = 0; }
			Snapshot.Nodes[Id].Row = Row++;
		}
	}
}
