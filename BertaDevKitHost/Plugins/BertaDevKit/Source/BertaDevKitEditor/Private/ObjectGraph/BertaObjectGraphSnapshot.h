#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UObject;

namespace BertaObjectGraph
{
	constexpr int32 MaxDisplayedChains = 32;
	constexpr int32 MaxDisplayedNodes = 256;

	enum class ERole : uint8 { Root, Intermediate, Target };

	// These values are detached from the native search before its destructor runs.
	struct FObject
	{
		FString Name;
		FString ClassName;
		FString Path;
		FString RootFlags;
		TWeakObjectPtr<UObject> LiveObject;
	};

	struct FStep
	{
		FObject Object;
		FString ReasonToNext;
	};

	// Each input chain is ordered root -> target. The reason on a step describes
	// its outgoing reference. Empty reasons are preserved as unknown references.
	using FChain = TArray<FStep>;

	struct FNode
	{
		int32 Id = INDEX_NONE;
		FObject Object;
		ERole Role = ERole::Intermediate;
		int32 Layer = 0;
		int32 Row = 0;
	};

	struct FEdge
	{
		int32 From = INDEX_NONE;
		int32 To = INDEX_NONE;
		FString Reason;
	};

	struct FSnapshot
	{
		TArray<FNode> Nodes;
		TArray<FEdge> Edges;
		int32 NativeChains = 0;
		int32 DisplayedChains = 0;
		bool bDirectRoot = false;
		bool bTruncated = false;
		double DurationMs = 0.0;
	};

	FSnapshot BuildSnapshot(TArray<FChain> Chains, int32 NativeChainCount);
}
