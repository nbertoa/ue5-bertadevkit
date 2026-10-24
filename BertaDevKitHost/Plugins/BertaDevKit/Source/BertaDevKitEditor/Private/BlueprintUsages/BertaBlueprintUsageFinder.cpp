#include "BlueprintUsages/BertaBlueprintUsageFinder.h"

#include "CoreMinimal.h"
#include "Log/BertaDevKitEditorLog.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/Blueprint.h"
#include "Engine/Level.h"
#include "Engine/LevelScriptBlueprint.h"
#include "Engine/World.h"
#include "Logging/MessageLog.h"
#include "Misc/PackageName.h"
#include "Misc/UObjectToken.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	const FName LogName(TEXT("BertaDevKitBlueprintUsages"));
	constexpr int32 MaxBlueprintAssetsPerSearch = 256;
	constexpr int32 MaxWorldAssetsPerSearch = 16;

	bool Matches(const UEdGraphNode& Node, const TSet<FString>& Paths)
	{
		TArray<UObject*> Refs;
		FReferenceFinder Finder(Refs);
		Finder.FindReferences(const_cast<UEdGraphNode*>(&Node));
		for (UObject* Object : Refs)
		{
			if (Object && Paths.Contains(Object->GetPathName()))
			{
				return true;
			}
		}
		for (UEdGraphPin* Pin : Node.Pins)
		{
			if (!Pin)
			{
				continue;
			}
			const FString DefaultPath = FPackageName::ExportTextPathToObjectPath(Pin->DefaultValue);
			if (Paths.Contains(DefaultPath) || (Pin->DefaultObject && Paths.Contains(Pin->DefaultObject->GetPathName())))
			{
				return true;
			}
		}
		return false;
	}
}

void FBertaBlueprintUsageFinder::FindUsages(const FAssetData& TargetAsset)
{
	FMessageLog Log(LogName);
	Log.NewPage(FText::FromString(TEXT("Find Blueprint Usages")));
	Log.Info(FText::FromString(TEXT("Scope: /Game Blueprint graph nodes and Level Script graph nodes only. This does not search C++, non-graph properties, dynamic lookups or all indirect/soft references.")));

	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	if (Registry.IsLoadingAssets())
	{
		Log.Error(FText::FromString(TEXT("The Asset Registry is still gathering assets. Search did not run.")));
		Log.Notify(FText::FromString(TEXT("Blueprint usage search could not run.")));
		return;
	}

	TSet<FString> TargetPaths;
	TargetPaths.Add(TargetAsset.GetObjectPathString());
	if (UBlueprint* TargetBlueprint = Cast<UBlueprint>(TargetAsset.GetAsset()))
	{
		if (TargetBlueprint->GeneratedClass)
		{
			TargetPaths.Add(TargetBlueprint->GeneratedClass->GetPathName());
		}
		if (TargetBlueprint->SkeletonGeneratedClass)
		{
			TargetPaths.Add(TargetBlueprint->SkeletonGeneratedClass->GetPathName());
		}
	}

	FARFilter BlueprintFilter;
	BlueprintFilter.bRecursivePaths = true;
	BlueprintFilter.bRecursiveClasses = true;
	BlueprintFilter.PackagePaths.Add(TEXT("/Game"));
	BlueprintFilter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	TArray<FAssetData> BlueprintAssets;
	Registry.GetAssets(BlueprintFilter, BlueprintAssets);

	FARFilter WorldFilter;
	WorldFilter.bRecursivePaths = true;
	WorldFilter.PackagePaths.Add(TEXT("/Game"));
	WorldFilter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
	TArray<FAssetData> WorldAssets;
	Registry.GetAssets(WorldFilter, WorldAssets);

	// Asset Registry enumeration order is not stable. A capped search must visit
	// the same set of assets in the same order each time.
	const auto SortByPackageName = [](const FAssetData& A, const FAssetData& B)
	{
		return A.PackageName.LexicalLess(B.PackageName);
	};
	BlueprintAssets.Sort(SortByPackageName);
	WorldAssets.Sort(SortByPackageName);

	const int32 BlueprintLimit = FMath::Min(BlueprintAssets.Num(), MaxBlueprintAssetsPerSearch);
	const int32 WorldLimit = FMath::Min(WorldAssets.Num(), MaxWorldAssetsPerSearch);
	const int32 SkippedByLimit = BlueprintAssets.Num() - BlueprintLimit + WorldAssets.Num() - WorldLimit;
	int32 LoadFailures = 0;
	int32 ConsumersInspected = 0;
	int32 MatchingNodes = 0;

	const auto InspectBlueprint = [&Log, &TargetPaths, &ConsumersInspected, &MatchingNodes](UBlueprint* Blueprint)
	{
		if (!Blueprint)
		{
			return;
		}
		++ConsumersInspected;
		TArray<UEdGraph*> Graphs;
		Blueprint->GetAllGraphs(Graphs);
		for (UEdGraph* Graph : Graphs)
		{
			if (!Graph)
			{
				continue;
			}
			for (UEdGraphNode* Node : Graph->Nodes)
			{
				if (!Node || !Matches(*Node, TargetPaths))
				{
					continue;
				}
				TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
				Message->AddToken(FTextToken::Create(FText::FromString(FString::Printf(
					TEXT("%s > %s > %s"),
					*Blueprint->GetPathName(), *Graph->GetName(), *Node->GetNodeTitle(ENodeTitleType::ListView).ToString()))));
				Message->AddToken(FUObjectToken::Create(Node, FText::FromString(TEXT("Open node"))));
				Log.AddMessage(Message);
				++MatchingNodes;
			}
		}
	};

	for (int32 Index = 0; Index < BlueprintLimit; ++Index)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAssets[Index].GetAsset()))
		{
			InspectBlueprint(Blueprint);
		}
		else
		{
			++LoadFailures;
		}
	}

	for (int32 Index = 0; Index < WorldLimit; ++Index)
	{
		UWorld* World = Cast<UWorld>(WorldAssets[Index].GetAsset());
		if (!World || !World->PersistentLevel)
		{
			++LoadFailures;
			continue;
		}
		// Passing true avoids creating a Level Script Blueprint during a read-only search.
		InspectBlueprint(World->PersistentLevel->GetLevelScriptBlueprint(true));
	}

	Log.Info(FText::FromString(FString::Printf(
		TEXT("Target %s: inspected %d Blueprint graph consumers, matched %d nodes. Candidates: %d Blueprints, %d maps; limits: %d Blueprints, %d maps."),
		*TargetAsset.GetObjectPathString(), ConsumersInspected, MatchingNodes,
		BlueprintAssets.Num(), WorldAssets.Num(), MaxBlueprintAssetsPerSearch, MaxWorldAssetsPerSearch)));

	if (SkippedByLimit > 0 || LoadFailures > 0)
	{
		Log.Warning(FText::FromString(FString::Printf(
			TEXT("PARTIAL SEARCH: skipped %d candidate assets due to scan limits; failed to load %d. A zero-match result does not mean the asset is unused."),
			SkippedByLimit, LoadFailures)));
		Log.Notify(FText::FromString(TEXT("Blueprint usage search incomplete; see Message Log.")));
		return;
	}
	Log.Info(FText::FromString(TEXT("All eligible /Game candidates were inspected. Results are limited to supported graph references, not all possible asset usages.")));
	Log.Notify(FText::FromString(TEXT("Blueprint graph reference search finished.")));
}
