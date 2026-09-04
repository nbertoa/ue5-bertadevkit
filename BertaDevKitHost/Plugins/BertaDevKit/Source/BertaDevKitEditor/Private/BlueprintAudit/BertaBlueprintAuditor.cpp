#include "BlueprintAudit/BertaBlueprintAuditor.h"

#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EdGraph/EdGraph.h"
#include "Engine/Blueprint.h"
#include "Engine/Level.h"
#include "Engine/LevelScriptBlueprint.h"
#include "Engine/World.h"
#include "Framework/Notifications/NotificationManager.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CreateDelegate.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_Variable.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/UnrealType.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace
{
	struct FMemberUse
	{
		int32 InternalReads = 0;
		int32 InternalWrites = 0;
		int32 ExternalUses = 0;
	};

	void Notify(const FText& Text, const SNotificationItem::ECompletionState State)
	{
		FNotificationInfo Info(Text);
		Info.bFireAndForget = true;
		Info.ExpireDuration = 5.0f;
		if (const TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info))
		{
			Item->SetCompletionState(State);
		}
	}

	bool IsBlueprintAuditProjectAsset(const FAssetData& Asset)
	{
		const FString Path = Asset.PackagePath.ToString();
		return Path == TEXT("/Game") || Path.StartsWith(TEXT("/Game/"));
	}

	bool IsSpecialVariable(const FBPVariableDescription& Variable)
	{
		const uint64 UnsafeFlags = CPF_Edit | CPF_Net | CPF_RepNotify | CPF_ExposeOnSpawn;
		return (Variable.PropertyFlags & UnsafeFlags) != 0
			|| Variable.RepNotifyFunc != NAME_None
			|| Variable.HasMetaData(TEXT("ExposeOnSpawn"))
			|| Variable.HasMetaData(TEXT("BindWidget"));
	}

	bool IsDeclaredFunction(const UBlueprint& Blueprint, const UEdGraph& Graph, UFunction*& OutFunction)
	{
		OutFunction = Blueprint.SkeletonGeneratedClass ? Blueprint.SkeletonGeneratedClass->FindFunctionByName(Graph.GetFName()) : nullptr;
		if (!OutFunction || !Blueprint.ParentClass || !Blueprint.SkeletonGeneratedClass)
		{
			return false;
		}

		// A parent declaration, interfaces, events, and libraries have semantics V1 cannot safely reduce.
		if (Blueprint.ParentClass->FindFunctionByName(Graph.GetFName()) || OutFunction->HasAnyFunctionFlags(FUNC_Event))
		{
			return false;
		}
		for (const FBPInterfaceDescription& Interface : Blueprint.ImplementedInterfaces)
		{
			if (Interface.Interface && Interface.Interface->FindFunctionByName(Graph.GetFName()))
			{
				return false;
			}
		}
		return !Blueprint.ParentClass->IsChildOf(UBlueprintFunctionLibrary::StaticClass());
	}

	bool IsSelfReference(const FMemberReference& Reference, const UBlueprint& Blueprint)
	{
		return Reference.IsSelfContext() || Reference.GetMemberParentClass(Blueprint.SkeletonGeneratedClass) == Blueprint.SkeletonGeneratedClass;
	}

	bool HasMeaningfulOutput(const UFunction& Function)
	{
		for (TFieldIterator<FProperty> It(&Function); It; ++It)
		{
			if (It->HasAnyPropertyFlags(CPF_ReturnParm) || (It->HasAnyPropertyFlags(CPF_OutParm) && !It->HasAnyPropertyFlags(CPF_ConstParm)))
			{
				return true;
			}
		}
		return false;
	}

	bool HasOnlyConservativeNodes(const UEdGraph& Graph, const UBlueprint& Blueprint, bool& bWritesSelf, bool& bImpureSelfCall)
	{
		bWritesSelf = false;
		bImpureSelfCall = false;
		for (const UEdGraphNode* Node : Graph.Nodes)
		{
			if (!Node || Node->IsA<UK2Node_FunctionEntry>() || Node->IsA<UK2Node_FunctionResult>() || Node->IsA<UK2Node_VariableGet>())
			{
				continue;
			}
			if (const UK2Node_VariableSet* Set = Cast<UK2Node_VariableSet>(Node))
			{
				// Any variable-set node is a write path. Treat it as uncertain rather
				// than claiming a const or pure recommendation for V1.
				bWritesSelf = true;
				if (FProperty* Property = Set->VariableReference.ResolveMember<FProperty>(Blueprint.SkeletonGeneratedClass))
				{
					if (Property->GetOwner<UClass>() == Blueprint.SkeletonGeneratedClass)
					{
						bWritesSelf = true;
					}
				}
				continue;
			}
			if (const UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node))
			{
				UFunction* Called = Call->FunctionReference.ResolveMember<UFunction>(Blueprint.SkeletonGeneratedClass);
				if (!Called)
				{
					return false;
				}
				if (IsSelfReference(Call->FunctionReference, Blueprint) && !Called->HasAnyFunctionFlags(FUNC_Const))
				{
					bImpureSelfCall = true;
				}
				// A non-self call can carry side effects; V1 cannot prove otherwise.
				if (!Called->HasAnyFunctionFlags(FUNC_Const | FUNC_BlueprintPure))
				{
					return false;
				}
				continue;
			}
			return false;
		}
		return true;
	}

	void GatherProjectBlueprints(TArray<UBlueprint*>& OutBlueprints, bool& bOutComplete)
	{
		bOutComplete = true;
		IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		FARFilter Filter;
		Filter.bRecursivePaths = true;
		Filter.bRecursiveClasses = true;
		Filter.PackagePaths.Add(TEXT("/Game"));
		Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
		TArray<FAssetData> Assets;
		Registry.GetAssets(Filter, Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset()))
			{
				OutBlueprints.AddUnique(Blueprint);
			}
			else
			{
				bOutComplete = false;
				UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[BlueprintAudit] Unable to load Blueprint reference consumer: %s"), *Asset.GetSoftObjectPath().ToString());
			}
		}
		// Level script Blueprints are embedded in map packages, not standalone Blueprint assets.
		FARFilter WorldFilter;
		WorldFilter.bRecursivePaths = true;
		WorldFilter.PackagePaths.Add(TEXT("/Game"));
		WorldFilter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
		Assets.Reset();
		Registry.GetAssets(WorldFilter, Assets);
		for (const FAssetData& Asset : Assets)
		{
			UWorld* World = Cast<UWorld>(Asset.GetAsset());
			if (!World || !World->PersistentLevel)
			{
				bOutComplete = false;
				continue;
			}
			if (ULevelScriptBlueprint* LevelBlueprint = World->PersistentLevel->GetLevelScriptBlueprint(true))
			{
				OutBlueprints.AddUnique(LevelBlueprint);
			}
		}
	}
}

void FBertaBlueprintAuditor::Audit(const TArray<FAssetData>& Assets)
{
	TArray<UBlueprint*> Targets;
	for (const FAssetData& Asset : Assets)
	{
		if (IsBlueprintAuditProjectAsset(Asset))
		{
			if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset()))
			{
				Targets.AddUnique(Blueprint);
			}
		}
	}
	if (Targets.IsEmpty())
	{
		Notify(NSLOCTEXT("BertaDevKit", "BlueprintAuditNoTargets", "Blueprint Audit: no Blueprint assets selected."), SNotificationItem::CS_None);
		return;
	}

	TArray<UBlueprint*> Consumers;
	bool bReferenceUniverseComplete = false;
	GatherProjectBlueprints(Consumers, bReferenceUniverseComplete);
	int32 UnusedVariables = 0, UnusedFunctions = 0, PrivateCandidates = 0, ConstCandidates = 0, PureCandidates = 0, Skipped = 0;
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[BlueprintAudit] Started: %d target Blueprint(s), %d reference consumer(s), universe complete: %s."), Targets.Num(), Consumers.Num(), bReferenceUniverseComplete ? TEXT("true") : TEXT("false"));

	for (UBlueprint* Target : Targets)
	{
		if (!Target || !Target->SkeletonGeneratedClass)
		{
			++Skipped;
			continue;
		}
		TMap<FProperty*, FMemberUse> Variables;
		TMap<UFunction*, FMemberUse> Functions;
		for (const FBPVariableDescription& Variable : Target->NewVariables)
		{
			if (FProperty* Property = FindFProperty<FProperty>(Target->SkeletonGeneratedClass, Variable.VarName))
			{
				if (Property->GetOwner<UClass>() == Target->SkeletonGeneratedClass)
				{
					Variables.Add(Property);
				}
			}
		}
		for (UEdGraph* Graph : Target->FunctionGraphs)
		{
			UFunction* Function = nullptr;
			if (Graph && IsDeclaredFunction(*Target, *Graph, Function))
			{
				Functions.Add(Function);
			}
		}

		for (UBlueprint* Consumer : Consumers)
		{
			if (!Consumer) { bReferenceUniverseComplete = false; continue; }
			// A descendant override is an external extension point even when no call node exists.
			if (Consumer != Target && Consumer->ParentClass && Consumer->ParentClass->IsChildOf(Target->SkeletonGeneratedClass))
			{
				for (UEdGraph* FunctionGraph : Consumer->FunctionGraphs)
				{
					if (FunctionGraph)
					{
						if (FMemberUse* Use = Functions.Find(Target->SkeletonGeneratedClass->FindFunctionByName(FunctionGraph->GetFName())))
						{
							++Use->ExternalUses;
						}
					}
				}
			}
			TArray<UEdGraph*> Graphs;
			Consumer->GetAllGraphs(Graphs);
			for (UEdGraph* Graph : Graphs)
			{
				if (!Graph) { continue; }
				for (UEdGraphNode* Node : Graph->Nodes)
				{
					if (UK2Node_Variable* VariableNode = Cast<UK2Node_Variable>(Node))
					{
						if (FProperty* Property = VariableNode->VariableReference.ResolveMember<FProperty>(Consumer))
						{
							if (FMemberUse* Use = Variables.Find(Property))
							{
								if (Consumer == Target)
								{
									if (VariableNode->IsA<UK2Node_VariableSet>()) { ++Use->InternalWrites; } else { ++Use->InternalReads; }
								}
								else { ++Use->ExternalUses; }
							}
						}
					}
					if (UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node))
					{
						if (UFunction* Function = Call->FunctionReference.ResolveMember<UFunction>(Consumer))
						{
							if (FMemberUse* Use = Functions.Find(Function))
							{
								if (Consumer == Target) { ++Use->InternalReads; } else { ++Use->ExternalUses; }
							}
						}
					}
					if (UK2Node_CreateDelegate* Delegate = Cast<UK2Node_CreateDelegate>(Node))
					{
						if (UClass* Scope = Delegate->GetScopeClass())
						{
							if (UFunction* Function = Scope->FindFunctionByName(Delegate->GetFunctionName()))
							{
								if (FMemberUse* Use = Functions.Find(Function))
								{
									if (Consumer == Target) { ++Use->InternalReads; } else { ++Use->ExternalUses; }
								}
							}
						}
					}
				}
			}
		}

		UE_LOG(LogBertaDevKitEditor, Log, TEXT("[BlueprintAudit] %s"), *Target->GetPathName());
		for (const FBPVariableDescription& Variable : Target->NewVariables)
		{
			FProperty* Property = FindFProperty<FProperty>(Target->SkeletonGeneratedClass, Variable.VarName);
			const FMemberUse* Use = Variables.Find(Property);
			if (!Use || IsSpecialVariable(Variable)) { continue; }
			if (bReferenceUniverseComplete && Use->InternalReads == 0 && Use->InternalWrites == 0 && Use->ExternalUses == 0)
			{
				++UnusedVariables;
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Info] Variable \"%s\" -> Unused Variable (reads: 0, writes: 0, external static Blueprint references: 0)."), *Variable.VarName.ToString());
			}
			if (bReferenceUniverseComplete && !Variable.HasMetaData(TEXT("BlueprintPrivate")) && !Property->HasMetaData(TEXT("BlueprintPrivate")) && Use->ExternalUses == 0)
			{
				++PrivateCandidates;
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Recommendation] Variable \"%s\" -> Candidate Private (internal usages: %d, 0 external static Blueprint references found)."), *Variable.VarName.ToString(), Use->InternalReads + Use->InternalWrites);
			}
		}
		for (UEdGraph* Graph : Target->FunctionGraphs)
		{
			UFunction* Function = nullptr;
			if (!Graph || !IsDeclaredFunction(*Target, *Graph, Function)) { continue; }
			const FMemberUse* Use = Functions.Find(Function);
			if (!Use) { continue; }
			if (bReferenceUniverseComplete && Use->InternalReads == 0 && Use->ExternalUses == 0)
			{
				++UnusedFunctions;
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Info] Function \"%s\" -> Unused Function (internal calls: 0, external static Blueprint calls: 0)."), *Function->GetName());
			}
			if (bReferenceUniverseComplete && !Function->HasAnyFunctionFlags(FUNC_Private) && Use->ExternalUses == 0)
			{
				++PrivateCandidates;
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Recommendation] Function \"%s\" -> Candidate Private (0 external static Blueprint calls found)."), *Function->GetName());
			}
			bool bWritesSelf = false, bImpureSelfCall = false;
			if (!Function->HasAnyFunctionFlags(FUNC_Const) && HasOnlyConservativeNodes(*Graph, *Target, bWritesSelf, bImpureSelfCall) && !bWritesSelf && !bImpureSelfCall)
			{
				++ConstCandidates;
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Recommendation] Function \"%s\" -> Candidate Const (no detected mutation of Self)."), *Function->GetName());
				if (!Function->HasAnyFunctionFlags(FUNC_BlueprintPure) && HasMeaningfulOutput(*Function))
				{
					++PureCandidates;
					UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Advisory] Function \"%s\" -> Advisory: Candidate Pure (no detected side effects; changing to Pure can change evaluation timing/count. Review manually)."), *Function->GetName());
				}
			}
		}
	}
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[BlueprintAudit] Complete: targets=%d consumers=%d unused variables=%d unused functions=%d private candidates=%d const candidates=%d pure advisories=%d skipped=%d."), Targets.Num(), Consumers.Num(), UnusedVariables, UnusedFunctions, PrivateCandidates, ConstCandidates, PureCandidates, Skipped);
	Notify(FText::Format(NSLOCTEXT("BertaDevKit", "BlueprintAuditComplete", "Blueprint Audit: {0} target(s), {1} finding(s). See Output Log."), FText::AsNumber(Targets.Num()), FText::AsNumber(UnusedVariables + UnusedFunctions + PrivateCandidates + ConstCandidates + PureCandidates)), SNotificationItem::CS_Success);
}
