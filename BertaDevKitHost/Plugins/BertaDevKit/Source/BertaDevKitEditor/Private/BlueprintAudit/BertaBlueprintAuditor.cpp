#include "BlueprintAudit/BertaBlueprintAuditor.h"
#include "BlueprintAudit/BertaBlueprintAuditMetrics.h"

#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
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
#include "EdGraphSchema_K2.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"
#include "Misc/UObjectToken.h"
#include "UObject/UnrealType.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace
{
	struct FMemberUse
	{
		int32 InternalReads = 0;
		int32 InternalWrites = 0;
		int32 ExternalUses = 0;
		int32 ExternalDescendantUses = 0;
		int32 ExternalNonDescendantUses = 0;
		TSet<const UEdGraph*> InternalGraphs;
	};

	const FName BlueprintAuditLogName(TEXT("BertaDevKitBlueprintAudit"));

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

	bool HasNonGraphVariableSemantics(const FBPVariableDescription& Variable)
	{
		const uint64 UnsafeFlags = CPF_Edit | CPF_Net | CPF_RepNotify | CPF_ExposeOnSpawn | CPF_Config | CPF_SaveGame | CPF_Interp;
		return (Variable.PropertyFlags & UnsafeFlags) != 0
			|| Variable.RepNotifyFunc != NAME_None
			|| Variable.HasMetaData(TEXT("ExposeOnSpawn"))
			|| Variable.HasMetaData(TEXT("BindWidget"))
			|| Variable.HasMetaData(TEXT("FieldNotify"));
	}

	bool IsUnsafePrivateVariableCandidate(const FBPVariableDescription& Variable, const FProperty& Property)
	{
		const uint64 AccessUnsafeFlags = CPF_Edit | CPF_Net | CPF_RepNotify | CPF_ExposeOnSpawn;
		return (Variable.PropertyFlags & AccessUnsafeFlags) != 0
			|| Variable.RepNotifyFunc != NAME_None
			|| Variable.HasMetaData(TEXT("ExposeOnSpawn"))
			|| Variable.HasMetaData(TEXT("BindWidget"))
			|| Variable.HasMetaData(TEXT("BlueprintPrivate"))
			|| Property.HasMetaData(TEXT("BlueprintPrivate"));
	}

	void AddFinding(FMessageLog& MessageLog, UBlueprint& Blueprint, const FText& Text, EMessageSeverity::Type Severity = EMessageSeverity::Info)
	{
		TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(Severity);
		Message->AddToken(FUObjectToken::Create(&Blueprint, FText::FromString(Blueprint.GetName())));
		Message->AddToken(FTextToken::Create(FText::FromString(TEXT("  "))));
		Message->AddToken(FTextToken::Create(Text));
		MessageLog.AddMessage(Message);
	}

	void AddMetricsSummary(FMessageLog& MessageLog, UBlueprint& Blueprint, const FString& Summary)
	{
		TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
		Message->AddToken(FUObjectToken::Create(&Blueprint, FText::FromString(Blueprint.GetName())));
		Message->AddToken(FTextToken::Create(FText::FromString(TEXT("  [Metrics] ") + Summary)));
		MessageLog.AddMessage(Message);
	}

	bool IsDescendantConsumer(const UBlueprint& Consumer, const UBlueprint& Target)
	{
		return Consumer.SkeletonGeneratedClass && Target.SkeletonGeneratedClass && Consumer.SkeletonGeneratedClass != Target.SkeletonGeneratedClass && Consumer.SkeletonGeneratedClass->IsChildOf(Target.SkeletonGeneratedClass);
	}

	UK2Node_FunctionEntry* FindFunctionEntry(const UEdGraph& Graph)
	{
		for (UEdGraphNode* Node : Graph.Nodes)
		{
			if (UK2Node_FunctionEntry* Entry = Cast<UK2Node_FunctionEntry>(Node))
			{
				return Entry;
			}
		}
		return nullptr;
	}

	bool IsMeaningfulFunctionInput(const UEdGraphPin& Pin, const UFunction& Function)
	{
		if (Pin.Direction != EGPD_Output || Pin.PinType.PinCategory == UEdGraphSchema_K2::PC_Exec || Pin.bHidden || Pin.bOrphanedPin)
		{
			return false;
		}
		const FProperty* Property = FindFProperty<FProperty>(&Function, Pin.PinName);
		return Property && Property->HasAnyPropertyFlags(CPF_Parm) && !Property->HasAnyPropertyFlags(CPF_ReturnParm | CPF_OutParm);
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
	int32 UnusedVariables = 0, UnusedFunctions = 0, PrivateCandidates = 0, ProtectedCandidates = 0, LocalCandidates = 0, ConstCandidates = 0, PureCandidates = 0, UnusedInputs = 0, MaintainabilityReviews = 0, Skipped = 0;
	FMessageLog MessageLog(BlueprintAuditLogName);
	MessageLog.NewPage(FText::Format(NSLOCTEXT("BertaDevKit", "BlueprintAuditPage", "Blueprint Audit {0}"), FText::AsDateTime(FDateTime::Now())));
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[BlueprintAudit] Started: %d target Blueprint(s), %d reference consumer(s), universe complete: %s."), Targets.Num(), Consumers.Num(), bReferenceUniverseComplete ? TEXT("true") : TEXT("false"));

	for (UBlueprint* Target : Targets)
	{
		if (!Target || !Target->SkeletonGeneratedClass)
		{
			++Skipped;
			continue;
		}
		TArray<FBertaBlueprintGraphMetrics> GraphMetrics;
		BertaBlueprintAuditMetrics::Collect(*Target, GraphMetrics);
		int32 TotalMeaningfulNodes = 0;
		for (const FBertaBlueprintGraphMetrics& Metrics : GraphMetrics)
		{
			TotalMeaningfulNodes += Metrics.MeaningfulNodeCount;
			TArray<EBertaBlueprintMaintainabilityReview> Reviews;
			BertaBlueprintAuditMetrics::EvaluateReviews(Metrics, Reviews);
			for (const EBertaBlueprintMaintainabilityReview Review : Reviews)
			{
				++MaintainabilityReviews;
				const FString GraphName = Metrics.Graph ? Metrics.Graph->GetName() : TEXT("Unknown");
				const bool bDecisionReview = Review == EBertaBlueprintMaintainabilityReview::HighDecisionCount;
				const int32 ActualValue = bDecisionReview ? Metrics.ConditionalDecisionCount : Metrics.MeaningfulNodeCount;
				const int32 Threshold = bDecisionReview ? 12 : BertaBlueprintAuditMetrics::GetSizeThreshold(Metrics.Kind);
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Review] %s \"%s\" -> %s (%d; review threshold is %d)."), BertaBlueprintAuditMetrics::GetGraphKindLabel(Metrics.Kind), *GraphName, BertaBlueprintAuditMetrics::GetReviewLabel(Review), ActualValue, Threshold);
				AddFinding(MessageLog, *Target, FText::Format(NSLOCTEXT("BertaDevKit", "BlueprintMaintainabilityReview", "[Review] {0} \"{1}\" -> {2}. Actual value: {3}; review threshold is {4}. Consider whether part of this logic has a clearer reusable responsibility."), FText::FromString(BertaBlueprintAuditMetrics::GetGraphKindLabel(Metrics.Kind)), FText::FromString(GraphName), FText::FromString(BertaBlueprintAuditMetrics::GetReviewLabel(Review)), FText::AsNumber(ActualValue), FText::AsNumber(Threshold)));
			}
		}
		GraphMetrics.Sort([](const FBertaBlueprintGraphMetrics& Left, const FBertaBlueprintGraphMetrics& Right)
		{
			return Left.MeaningfulNodeCount != Right.MeaningfulNodeCount
				? Left.MeaningfulNodeCount > Right.MeaningfulNodeCount
				: Left.Graph->GetName() < Right.Graph->GetName();
		});
		FString MetricsSummary = FString::Printf(TEXT("Graphs=%d, meaningful nodes=%d. Largest:"), GraphMetrics.Num(), TotalMeaningfulNodes);
		for (int32 Index = 0; Index < FMath::Min(3, GraphMetrics.Num()); ++Index)
		{
			const FBertaBlueprintGraphMetrics& Metrics = GraphMetrics[Index];
			MetricsSummary += FString::Printf(TEXT("%s%s (%s) - %d nodes / %d decisions"), Index == 0 ? TEXT(" ") : TEXT("; "), *Metrics.Graph->GetName(), BertaBlueprintAuditMetrics::GetGraphKindLabel(Metrics.Kind), Metrics.MeaningfulNodeCount, Metrics.ConditionalDecisionCount);
		}
		UE_LOG(LogBertaDevKitEditor, Log, TEXT("[BlueprintAudit] %s [Metrics] %s"), *Target->GetPathName(), *MetricsSummary);
		AddMetricsSummary(MessageLog, *Target, MetricsSummary);
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
			if (IsDescendantConsumer(*Consumer, *Target))
			{
				for (UEdGraph* FunctionGraph : Consumer->FunctionGraphs)
				{
					if (FunctionGraph)
					{
						if (FMemberUse* Use = Functions.Find(Target->SkeletonGeneratedClass->FindFunctionByName(FunctionGraph->GetFName())))
						{
							++Use->ExternalUses;
							++Use->ExternalDescendantUses;
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
									Use->InternalGraphs.Add(Graph);
								}
								else
								{
									++Use->ExternalUses;
									if (IsDescendantConsumer(*Consumer, *Target)) { ++Use->ExternalDescendantUses; } else { ++Use->ExternalNonDescendantUses; }
								}
							}
						}
					}
					if (UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node))
					{
						if (UFunction* Function = Call->FunctionReference.ResolveMember<UFunction>(Consumer))
						{
							if (FMemberUse* Use = Functions.Find(Function))
							{
								if (Consumer == Target) { ++Use->InternalReads; }
								else { ++Use->ExternalUses; if (IsDescendantConsumer(*Consumer, *Target)) { ++Use->ExternalDescendantUses; } else { ++Use->ExternalNonDescendantUses; } }
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
									if (Consumer == Target) { ++Use->InternalReads; }
									else { ++Use->ExternalUses; if (IsDescendantConsumer(*Consumer, *Target)) { ++Use->ExternalDescendantUses; } else { ++Use->ExternalNonDescendantUses; } }
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
			if (!Use) { continue; }
			const bool bHasNonGraphSemantics = HasNonGraphVariableSemantics(Variable);
			if (!bHasNonGraphSemantics && bReferenceUniverseComplete && Use->InternalReads == 0 && Use->InternalWrites == 0 && Use->ExternalUses == 0)
			{
				++UnusedVariables;
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Info] Variable \"%s\" -> Unused Variable (reads: 0, writes: 0, external static Blueprint references: 0)."), *Variable.VarName.ToString());
				AddFinding(MessageLog, *Target, FText::Format(NSLOCTEXT("BertaDevKit", "UnusedVariable", "[Info] Variable \"{0}\" -> Unused Variable (reads: 0, writes: 0)."), FText::FromName(Variable.VarName)));
			}
			else if (!bHasNonGraphSemantics && bReferenceUniverseComplete && Use->InternalGraphs.Num() == 1 && Use->ExternalUses == 0)
			{
				const UEdGraph* OnlyGraph = *Use->InternalGraphs.CreateConstIterator();
				UFunction* OwnerFunction = OnlyGraph ? Target->SkeletonGeneratedClass->FindFunctionByName(OnlyGraph->GetFName()) : nullptr;
				if (OwnerFunction && IsDeclaredFunction(*Target, *OnlyGraph, OwnerFunction))
				{
					++LocalCandidates;
					UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Recommendation] Variable \"%s\" -> Review for Local Variable (single-function member used only inside \"%s\"; it may intentionally retain state between calls; external static Blueprint references: 0)."), *Variable.VarName.ToString(), *OwnerFunction->GetName());
					AddFinding(MessageLog, *Target, FText::Format(NSLOCTEXT("BertaDevKit", "CandidateLocal", "[Recommendation] Variable \"{0}\" -> Review for Local Variable. Used only inside function \"{1}\"; it may intentionally retain state between calls. External static Blueprint references: 0."), FText::FromName(Variable.VarName), FText::FromString(OwnerFunction->GetName())));
				}
			}
			if (bReferenceUniverseComplete && !IsUnsafePrivateVariableCandidate(Variable, *Property) && Use->ExternalUses == 0)
			{
				++PrivateCandidates;
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Recommendation] Variable \"%s\" -> Candidate Private (internal usages: %d, 0 external static Blueprint references found)."), *Variable.VarName.ToString(), Use->InternalReads + Use->InternalWrites);
				AddFinding(MessageLog, *Target, FText::Format(NSLOCTEXT("BertaDevKit", "CandidatePrivateVariable", "[Recommendation] Variable \"{0}\" -> Candidate Private. 0 external static Blueprint references found."), FText::FromName(Variable.VarName)));
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
				AddFinding(MessageLog, *Target, FText::Format(NSLOCTEXT("BertaDevKit", "UnusedFunction", "[Info] Function \"{0}\" -> Unused Function."), FText::FromString(Function->GetName())));
			}
			if (bReferenceUniverseComplete && !Function->HasAnyFunctionFlags(FUNC_Private) && Use->ExternalUses == 0)
			{
				++PrivateCandidates;
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Recommendation] Function \"%s\" -> Candidate Private (0 external static Blueprint calls found)."), *Function->GetName());
				AddFinding(MessageLog, *Target, FText::Format(NSLOCTEXT("BertaDevKit", "CandidatePrivateFunction", "[Recommendation] Function \"{0}\" -> Candidate Private. 0 external static Blueprint calls found."), FText::FromString(Function->GetName())));
			}
			else if (bReferenceUniverseComplete && !Function->HasAnyFunctionFlags(FUNC_Private | FUNC_Protected) && Use->ExternalDescendantUses > 0 && Use->ExternalNonDescendantUses == 0)
			{
				++ProtectedCandidates;
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Recommendation] Function \"%s\" -> Candidate Protected (all %d external static Blueprint callers are descendants)."), *Function->GetName(), Use->ExternalDescendantUses);
				AddFinding(MessageLog, *Target, FText::Format(NSLOCTEXT("BertaDevKit", "CandidateProtectedFunction", "[Recommendation] Function \"{0}\" -> Candidate Protected. All external static Blueprint callers are descendants ({1})."), FText::FromString(Function->GetName()), FText::AsNumber(Use->ExternalDescendantUses)));
			}
			if (UK2Node_FunctionEntry* Entry = FindFunctionEntry(*Graph))
			{
				for (UEdGraphPin* Pin : Entry->Pins)
				{
					if (Pin && IsMeaningfulFunctionInput(*Pin, *Function) && Pin->LinkedTo.IsEmpty())
					{
						++UnusedInputs;
						UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Info] Function \"%s\", Input \"%s\" -> Unused Function Input."), *Function->GetName(), *Pin->PinName.ToString());
						AddFinding(MessageLog, *Target, FText::Format(NSLOCTEXT("BertaDevKit", "UnusedFunctionInput", "[Info] Function \"{0}\", Input \"{1}\" -> Unused Function Input."), FText::FromString(Function->GetName()), FText::FromName(Pin->PinName)));
					}
				}
			}
			bool bWritesSelf = false, bImpureSelfCall = false;
			if (!Function->HasAnyFunctionFlags(FUNC_Const) && HasOnlyConservativeNodes(*Graph, *Target, bWritesSelf, bImpureSelfCall) && !bWritesSelf && !bImpureSelfCall)
			{
				++ConstCandidates;
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Recommendation] Function \"%s\" -> Candidate Const (no detected mutation of Self)."), *Function->GetName());
				AddFinding(MessageLog, *Target, FText::Format(NSLOCTEXT("BertaDevKit", "CandidateConst", "[Recommendation] Function \"{0}\" -> Candidate Const. No detected mutation of Self."), FText::FromString(Function->GetName())));
				if (!Function->HasAnyFunctionFlags(FUNC_BlueprintPure) && HasMeaningfulOutput(*Function))
				{
					++PureCandidates;
					UE_LOG(LogBertaDevKitEditor, Log, TEXT("  [Advisory] Function \"%s\" -> Advisory: Candidate Pure (no detected side effects; changing to Pure can change evaluation timing/count. Review manually)."), *Function->GetName());
					AddFinding(MessageLog, *Target, FText::Format(NSLOCTEXT("BertaDevKit", "CandidatePure", "[Advisory] Function \"{0}\" -> Candidate Pure. Changing to Pure can change evaluation timing/count. Review manually."), FText::FromString(Function->GetName())));
				}
			}
		}
	}
	const int32 Findings = UnusedVariables + UnusedFunctions + PrivateCandidates + ProtectedCandidates + LocalCandidates + ConstCandidates + PureCandidates + UnusedInputs + MaintainabilityReviews;
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[BlueprintAudit] Complete: targets=%d consumers=%d unused variables=%d unused functions=%d private candidates=%d protected candidates=%d local variable candidates=%d const candidates=%d pure advisories=%d unused function inputs=%d maintainability reviews=%d skipped=%d."), Targets.Num(), Consumers.Num(), UnusedVariables, UnusedFunctions, PrivateCandidates, ProtectedCandidates, LocalCandidates, ConstCandidates, PureCandidates, UnusedInputs, MaintainabilityReviews, Skipped);
	MessageLog.Flush();
	Notify(FText::Format(NSLOCTEXT("BertaDevKit", "BlueprintAuditComplete", "Blueprint Audit: {0} target(s), {1} finding(s). See Message Log / Output Log."), FText::AsNumber(Targets.Num()), FText::AsNumber(Findings)), SNotificationItem::CS_Success);
}
