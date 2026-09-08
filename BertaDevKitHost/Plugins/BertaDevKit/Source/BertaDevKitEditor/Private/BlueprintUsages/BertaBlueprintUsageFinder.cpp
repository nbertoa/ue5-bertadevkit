#include "BlueprintUsages/BertaBlueprintUsageFinder.h"
#include "Log/BertaDevKitEditorLog.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/Blueprint.h"
#include "Engine/LevelScriptBlueprint.h"
#include "Engine/World.h"
#include "Logging/MessageLog.h"
#include "Misc/PackageName.h"
#include "Misc/UObjectToken.h"
#include "Modules/ModuleManager.h"

namespace { const FName LogName(TEXT("BertaDevKitBlueprintUsages"));
bool Matches(const UEdGraphNode& Node,const TSet<FString>& Paths){TArray<UObject*> Refs;FReferenceFinder Finder(Refs);Finder.FindReferences(const_cast<UEdGraphNode*>(&Node));for(UObject* O:Refs)if(O&&Paths.Contains(O->GetPathName()))return true;for(UEdGraphPin* P:Node.Pins)if(P){FString V=FPackageName::ExportTextPathToObjectPath(P->DefaultValue);if(Paths.Contains(V)||Paths.Contains(P->DefaultObject?P->DefaultObject->GetPathName():FString()))return true;}return false;}
void AddBlueprint(UBlueprint* B,TArray<UBlueprint*>& Out){if(B)Out.AddUnique(B);}
}
void FBertaBlueprintUsageFinder::FindUsages(const FAssetData& TargetAsset){FMessageLog Log(LogName);Log.NewPage(FText::FromString(TEXT("Find Blueprint Usages")));IAssetRegistry& R=FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();if(R.IsLoadingAssets()){Log.Error(FText::FromString(TEXT("Asset Registry is still gathering; the Blueprint universe is incomplete.")));Log.Notify(FText::FromString(TEXT("Blueprint usage search could not run.")));return;}TSet<FString> Paths;Paths.Add(TargetAsset.GetObjectPathString());if(UBlueprint* T=Cast<UBlueprint>(TargetAsset.GetAsset())){if(T->GeneratedClass)Paths.Add(T->GeneratedClass->GetPathName());if(T->SkeletonGeneratedClass)Paths.Add(T->SkeletonGeneratedClass->GetPathName());}FARFilter F;F.bRecursivePaths=true;F.bRecursiveClasses=true;F.PackagePaths.Add(TEXT("/Game"));F.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());TArray<FAssetData>A;R.GetAssets(F,A);TArray<UBlueprint*> Bs;bool Complete=true;for(const FAssetData& D:A){UBlueprint* B=Cast<UBlueprint>(D.GetAsset());if(B)AddBlueprint(B,Bs);else Complete=false;}FARFilter W;W.bRecursivePaths=true;W.PackagePaths.Add(TEXT("/Game"));W.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());A.Reset();R.GetAssets(W,A);for(const FAssetData&D:A){UWorld* World=Cast<UWorld>(D.GetAsset());if(!World||!World->PersistentLevel){Complete=false;continue;}AddBlueprint(World->PersistentLevel->GetLevelScriptBlueprint(true),Bs);}if(!Complete){Log.Error(FText::FromString(TEXT("One or more Blueprint consumers could not be loaded; results are incomplete.")));Log.Notify(FText::FromString(TEXT("Blueprint usage search is incomplete.")));return;}int32 Hits=0;for(UBlueprint* B:Bs){TArray<UEdGraph*> G;B->GetAllGraphs(G);for(UEdGraph* Graph:G)if(Graph)for(UEdGraphNode* N:Graph->Nodes)if(N&&Matches(*N,Paths)){TSharedRef<FTokenizedMessage>M=FTokenizedMessage::Create(EMessageSeverity::Info);M->AddToken(FTextToken::Create(FText::FromString(FString::Printf(TEXT("%s > %s > %s"),*B->GetPathName(),*Graph->GetName(),*N->GetNodeTitle(ENodeTitleType::ListView).ToString()))));M->AddToken(FUObjectToken::Create(N,FText::FromString(TEXT("Open node"))));Log.AddMessage(M);++Hits;}}Log.Info(FText::FromString(FString::Printf(TEXT("Target %s: inspected %d Blueprint consumers; found %d graph nodes; universe complete."),*TargetAsset.GetObjectPathString(),Bs.Num(),Hits)));Log.Notify(FText::FromString(TEXT("Blueprint usage search complete.")));}
