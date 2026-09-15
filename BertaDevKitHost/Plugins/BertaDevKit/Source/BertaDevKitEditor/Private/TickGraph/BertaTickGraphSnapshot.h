#pragma once

#include "Containers/Array.h"
#include "Containers/Set.h"
#include "UObject/WeakObjectPtr.h"

class UObject;

namespace BertaTickGraph
{
	inline constexpr int32 MaxNodes = 256;
	inline constexpr int32 MaxEdges = 512;
	inline constexpr int32 MaxDepth = 32;

	enum class ERole : uint8 { TargetActorPrimary, TargetComponentPrimary, ExternalPrerequisite, CustomPrerequisite };
	enum class EKind : uint8 { ActorPrimary, ComponentPrimary, Custom };

	struct FNode
	{
		int32 Id = INDEX_NONE;
		FString Name;
		FString OwnerName;
		FString OwnerClass;
		FString OwnerPath;
		TWeakObjectPtr<UObject> LiveOwner; // A diagnostic snapshot must never keep a PIE object alive.
		ERole Role = ERole::ExternalPrerequisite;
		EKind Kind = EKind::Custom;
		FString TickGroup;
		FString EndTickGroup;
		float Interval = 0.0f;
		bool bCanEverTick = false;
		bool bEnabled = false;
		bool bRegistered = false;
		bool bHighPriority = false;
		bool bAnyThread = false;
		bool bPaused = false;
		bool bStartEnabled = false;
		bool bDedicatedServer = false;
		bool bDispatchManually = false;
		int32 DirectPrerequisites = 0;
		int32 Layer = 0;
		int32 Row = 0;
	};

	struct FEdge
	{
		int32 Prerequisite = INDEX_NONE;
		int32 Dependent = INDEX_NONE;
	};

	struct FSnapshot
	{
		FString TargetName;
		FString TargetClass;
		FString TargetPath;
		TArray<FNode> Nodes;
		TArray<FEdge> Edges;
		int32 SeedCount = 0;
		int32 ExternalCount = 0;
		int32 InvalidPrerequisiteCount = 0;
		bool bTruncated = false;
		bool bCycle = false;
		bool bStale = false;
		double DurationMs = 0.0;
	};

	struct FLimits { int32 Nodes = MaxNodes; int32 Edges = MaxEdges; int32 Depth = MaxDepth; };

	class FBuilder
	{
	public:
		explicit FBuilder(FLimits InLimits = {}) : Limits(InLimits) {}
		int32 AddNode(FNode Node);
		bool AddEdge(int32 Prerequisite, int32 Dependent);
		bool Enter(int32 NodeId, int32 Depth);
		void Leave(int32 NodeId);
		void MarkTruncated() { Snapshot.bTruncated = true; }
		FSnapshot& Data() { return Snapshot; }
		FSnapshot Finish();
	private:
		FLimits Limits;
		FSnapshot Snapshot;
		TSet<int32> Active;
		TSet<int32> Complete;
	};

	void Layout(FSnapshot& Snapshot);
}
