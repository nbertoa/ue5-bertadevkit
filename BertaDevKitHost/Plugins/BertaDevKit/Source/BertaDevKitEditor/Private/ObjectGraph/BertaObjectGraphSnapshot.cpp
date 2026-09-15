#include "ObjectGraph/BertaObjectGraphSnapshot.h"

#include "Algo/Sort.h"
#include "Containers/Map.h"
#include "Containers/Set.h"

namespace BertaObjectGraph
{
	FSnapshot BuildSnapshot(TArray<FChain> Chains, int32 NativeChainCount)
	{
		FSnapshot Result;
		Result.NativeChains = NativeChainCount;
		// Sort before applying caps: engine traversal and TSet iteration order
		// must not change which paths are displayed.
		Chains.Sort([](const FChain& A, const FChain& B)
		{
			if (A.Num() != B.Num()) { return A.Num() < B.Num(); }
			FString AKey, BKey;
			for (const FStep& Step : A) { AKey += Step.Object.Path + TEXT("|") + Step.ReasonToNext + TEXT("|"); }
			for (const FStep& Step : B) { BKey += Step.Object.Path + TEXT("|") + Step.ReasonToNext + TEXT("|"); }
			return AKey < BKey;
		});

		TMap<FString, int32> IdByPath;
		TSet<FString> EdgeKeys;
		TArray<int32> DisplayedChainIndices;
		for (int32 ChainIndex = 0; ChainIndex < Chains.Num(); ++ChainIndex)
		{
			const FChain& Chain = Chains[ChainIndex];
			if (Result.DisplayedChains >= MaxDisplayedChains)
			{
				Result.bTruncated = true;
				break;
			}
			int32 NewNodes = 0;
			TSet<FString> NewPaths;
			for (const FStep& Step : Chain)
			{
				if (!IdByPath.Contains(Step.Object.Path) && !NewPaths.Contains(Step.Object.Path))
				{
					NewPaths.Add(Step.Object.Path);
					++NewNodes;
				}
			}
			if (Result.Nodes.Num() + NewNodes > MaxDisplayedNodes)
			{
				Result.bTruncated = true;
				continue;
			}
			++Result.DisplayedChains;
			DisplayedChainIndices.Add(ChainIndex);
			TArray<int32> Ids;
			for (int32 Index = 0; Index < Chain.Num(); ++Index)
			{
				const FStep& Step = Chain[Index];
				int32* Existing = IdByPath.Find(Step.Object.Path);
				int32 Id;
				if (Existing)
				{
					Id = *Existing;
				}
				else
				{
					Id = Result.Nodes.Num();
					IdByPath.Add(Step.Object.Path, Id);
					FNode& Node = Result.Nodes.AddDefaulted_GetRef();
					Node.Id = Id;
					Node.Object = Step.Object;
				}
				FNode& Node = Result.Nodes[Id];
				if (Index == Chain.Num() - 1) { Node.Role = ERole::Target; }
				else if (Index == 0 && Node.Role != ERole::Target) { Node.Role = ERole::Root; }
				Ids.Add(Id);
				if (Index > 0)
				{
					const FString& Reason = Chain[Index - 1].ReasonToNext;
					const FString EdgeKey = FString::Printf(TEXT("%d|%d|%s"), Ids[Index - 1], Id, *Reason);
					if (!EdgeKeys.Contains(EdgeKey))
					{
						EdgeKeys.Add(EdgeKey);
						Result.Edges.Add({Ids[Index - 1], Id, Reason});
					}
				}
			}
		}
		Result.bTruncated |= Result.DisplayedChains < NativeChainCount;

		// Longest prefix within the displayed chains gives a stable layered
		// layout. Roots remain at x=0; the target is placed after every other
		// layer. Rows are ordered by copied path, independently of GC traversal.
		for (int32 ChainIndex : DisplayedChainIndices)
		{
			const FChain& Chain = Chains[ChainIndex];
			if (Chain.IsEmpty() || !IdByPath.Contains(Chain[0].Object.Path)) { continue; }
			for (int32 Index = 0; Index < Chain.Num(); ++Index)
			{
				if (int32* Id = IdByPath.Find(Chain[Index].Object.Path))
				{
					FNode& Node = Result.Nodes[*Id];
					if (Node.Role != ERole::Root && Node.Role != ERole::Target)
					{
						Node.Layer = FMath::Max(Node.Layer, Index);
					}
				}
			}
		}
		int32 MaxLayer = 0;
		for (const FNode& Node : Result.Nodes) { MaxLayer = FMath::Max(MaxLayer, Node.Layer); }
		for (FNode& Node : Result.Nodes)
		{
			if (Node.Role == ERole::Root) { Node.Layer = 0; }
			if (Node.Role == ERole::Target) { Node.Layer = MaxLayer + 1; }
		}
		TArray<int32> Order;
		for (const FNode& Node : Result.Nodes) { Order.Add(Node.Id); }
		Order.Sort([&Result](int32 A, int32 B)
		{
			const FNode& Left = Result.Nodes[A];
			const FNode& Right = Result.Nodes[B];
			if (Left.Layer != Right.Layer) { return Left.Layer < Right.Layer; }
			return Left.Object.Path < Right.Object.Path;
		});
		int32 PreviousLayer = INDEX_NONE, Row = 0;
		for (int32 Id : Order)
		{
			FNode& Node = Result.Nodes[Id];
			if (Node.Layer != PreviousLayer) { PreviousLayer = Node.Layer; Row = 0; }
			Node.Row = Row++;
		}
		return Result;
	}
}
