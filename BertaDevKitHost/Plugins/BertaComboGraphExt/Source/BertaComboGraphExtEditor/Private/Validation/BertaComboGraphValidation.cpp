#include "Validation/BertaComboGraphValidation.h"
#include "Validation/BertaComboGraphValidationRules.h"

#include "AnimNotifies/ComboGraphANS_ComboWindow.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimCompositeBase.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/Skeleton.h"
#include "Diagnostics/BertaComboGraphEffectContextCompatibility.h"
#include "GameplayEffect.h"
#include "Graph/ComboGraph.h"
#include "Graph/ComboGraphEdge.h"
#include "Graph/ComboGraphNodeAnimBase.h"
#include "Graph/ComboGraphNodeConduit.h"
#include "Graph/ComboGraphNodeEntry.h"
#include "Graph/ComboGraphNodeMontage.h"
#include "Graph/ComboGraphNodeSequence.h"
#include "Settings/ComboGraphProjectSettings.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

namespace
{
	void AddFinding(FBertaComboGraphValidationReport& Report, const EBertaComboGraphValidationSeverity Severity,
		const TCHAR* Code, const UObject* Node, const UObject* Edge, const TCHAR* Property, const FString& Message, const FString& Evidence)
	{
		FBertaComboGraphValidationFinding& Finding = Report.Findings.AddDefaulted_GetRef();
		Finding.Severity = Severity;
		Finding.AssetPath = Report.AssetPath;
		Finding.NodePath = GetPathNameSafe(Node);
		Finding.EdgePath = GetPathNameSafe(Edge);
		Finding.PropertyPath = Property;
		Finding.Code = Code;
		Finding.Message = Message;
		Finding.Evidence = Evidence;
	}

	FString TransitionSignature(const UComboGraphEdge* Edge)
	{
		return Edge ? FString::Printf(TEXT("%s|%d"), *GetPathNameSafe(Edge->TransitionInput), static_cast<int32>(Edge->TriggerEvent)) : TEXT("None");
	}

	bool FindEffectiveAutoWindow(const UComboGraphNodeAnimBase* Node, FComboGraphNotifyStateAutoSetup& Out)
	{
		auto FindInMap = [&Out](const TMap<TSoftClassPtr<UAnimNotifyState>, FComboGraphNotifyStateAutoSetup>& Map)
		{
			const TSoftClassPtr<UAnimNotifyState> ComboWindowClass(UComboGraphANS_ComboWindow::StaticClass());
			if (const FComboGraphNotifyStateAutoSetup* Setup = Map.Find(ComboWindowClass))
			{
				Out = *Setup;
				return true;
			}
			return false;
		};
		return Node && (FindInMap(Node->NotifyStatesOverrides) || FindInMap(UComboGraphProjectSettings::Get().NotifyStates));
	}

	FAnimNotifyEvent* FindAuthoredComboWindow(UAnimSequenceBase* Animation, TSet<UAnimSequenceBase*>& Visited)
	{
		if (!Animation || Visited.Contains(Animation)) return nullptr;
		Visited.Add(Animation);
		for (FAnimNotifyEvent& Event : Animation->Notifies)
		{
			if (Event.NotifyStateClass && Event.NotifyStateClass->IsA<UComboGraphANS_ComboWindow>()) return &Event;
		}
		if (UAnimMontage* Montage = Cast<UAnimMontage>(Animation))
		{
			for (FSlotAnimationTrack& Slot : Montage->SlotAnimTracks)
			{
				for (FAnimSegment& Segment : Slot.AnimTrack.AnimSegments)
				{
					if (FAnimNotifyEvent* Event = FindAuthoredComboWindow(Segment.GetAnimReference(), Visited)) return Event;
				}
			}
		}
		return nullptr;
	}

	FAnimNotifyEvent* FindAuthoredComboWindow(UAnimSequenceBase* Animation)
	{
		TSet<UAnimSequenceBase*> Visited;
		return FindAuthoredComboWindow(Animation, Visited);
	}

	void ValidateAnimation(FBertaComboGraphValidationReport& Report, UComboGraph* Graph, UComboGraphNodeAnimBase* Node)
	{
		UAnimSequenceBase* Animation = Node->GetAnimationAsset();
		if (!Animation)
		{
			AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.UnresolvedAnimation"), Node, nullptr, TEXT("Animation"), TEXT("The node animation cannot be resolved."), TEXT("Runtime synchronous loading would return null and graph execution cannot play this node."));
			return;
		}
		if (!Animation->GetSkeleton())
		{
			AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingAnimationSkeleton"), Node, nullptr, TEXT("Animation.Skeleton"), TEXT("The animation has no skeleton."), *GetPathNameSafe(Animation));
		}
		if (USkeletalMesh* PreviewMesh = Graph->GetPreviewMesh())
		{
			if (Animation->GetSkeleton() && PreviewMesh->GetSkeleton() && !Animation->GetSkeleton()->IsCompatibleForEditor(PreviewMesh->GetSkeleton()))
			{
				AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.IncompatiblePreviewSkeleton"), Node, nullptr, TEXT("Animation.Skeleton"), TEXT("The animation skeleton is incompatible with the graph preview mesh skeleton."), *GetPathNameSafe(Animation));
			}
		}
		if (const UComboGraphNodeMontage* MontageNode = Cast<UComboGraphNodeMontage>(Node))
		{
			const UAnimMontage* Montage = MontageNode->Montage.LoadSynchronous();
			if (Montage && !MontageNode->StartSection.IsNone() && Montage->GetSectionIndex(MontageNode->StartSection) == INDEX_NONE)
			{
				AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InvalidMontageSection"), Node, nullptr, TEXT("StartSection"), TEXT("Configured montage start section does not exist."), MontageNode->StartSection.ToString());
			}
		}
		else if (Cast<UComboGraphNodeSequence>(Node) && UComboGraphProjectSettings::Get().DynamicMontageSlotName.IsNone())
		{
			AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingDynamicMontageSlot"), Node, nullptr, TEXT("DynamicMontageSlotName"), TEXT("Sequence nodes require a non-empty dynamic montage slot."), TEXT("Combo Graph project setting DynamicMontageSlotName is None."));
		}
	}

	void ValidateContainers(FBertaComboGraphValidationReport& Report, UComboGraphNodeAnimBase* Node, bool& bUsesCues)
	{
		if (Node->CostGameplayEffect)
		{
			const UGameplayEffect* Cost = Node->CostGameplayEffect->GetDefaultObject<UGameplayEffect>();
			for (int32 Index = 0; Cost && Index < Cost->Modifiers.Num(); ++Index)
			{
				const FGameplayModifierInfo& Modifier = Cost->Modifiers[Index];
				if (!Modifier.Attribute.IsValid()) AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InvalidCostAttribute"), Node, nullptr, TEXT("CostGameplayEffect.Modifiers"), TEXT("Cost modifier has an invalid attribute."), FString::Printf(TEXT("Modifier index %d"), Index));
				if (Modifier.ModifierOp != EGameplayModOp::Additive) AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.UnsupportedCostModifierOperation"), Node, nullptr, TEXT("CostGameplayEffect.Modifiers"), TEXT("Combo Graph cost preflight only evaluates additive modifiers."), FString::Printf(TEXT("Modifier index %d uses %s"), Index, *StaticEnum<EGameplayModOp::Type>()->GetNameStringByValue(Modifier.ModifierOp)));
			}
		}
		for (const TPair<FGameplayTag, FComboGraphGameplayEffectContainer>& Pair : Node->EffectsContainerMap)
		{
			if (!Pair.Key.IsValid()) AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InvalidEffectContainerTag"), Node, nullptr, TEXT("EffectsContainerMap"), TEXT("Effect container has an invalid map key."), TEXT("Gameplay event dispatch cannot address this entry."));
			if (Pair.Value.bUseSetByCallerMagnitude && !Pair.Value.SetByCallerDataTag.IsValid()) AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InvalidSetByCallerTag"), Node, nullptr, TEXT("EffectsContainerMap.SetByCallerDataTag"), TEXT("SetByCaller is enabled without a valid data tag."), Pair.Key.ToString());
			if (!FMath::IsFinite(Pair.Value.SetByCallerMagnitude)) AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.NonFiniteSetByCallerMagnitude"), Node, nullptr, TEXT("EffectsContainerMap.SetByCallerMagnitude"), TEXT("SetByCaller magnitude must be finite."), Pair.Key.ToString());
			for (int32 Index = 0; Index < Pair.Value.TargetGameplayEffectClasses.Num(); ++Index) if (!Pair.Value.TargetGameplayEffectClasses[Index]) AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingEffectClass"), Node, nullptr, TEXT("EffectsContainerMap.TargetGameplayEffectClasses"), TEXT("Effect container contains an empty GameplayEffect class."), FString::Printf(TEXT("%s index %d"), *Pair.Key.ToString(), Index));
		}
		for (const TPair<FGameplayTag, FComboGraphDamageSystemContainer>& Pair : Node->DamagesContainerMap)
		{
			if (!Pair.Key.IsValid()) AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InvalidDamageContainerTag"), Node, nullptr, TEXT("DamagesContainerMap"), TEXT("Damage container has an invalid map key."), TEXT("Gameplay event dispatch cannot address this entry."));
			if (!FMath::IsFinite(Pair.Value.BaseDamage)) AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.NonFiniteDamage"), Node, nullptr, TEXT("DamagesContainerMap.BaseDamage"), TEXT("Damage magnitude must be finite."), Pair.Key.ToString());
		}
		bUsesCues |= !Node->CuesContainerMap.IsEmpty();
		for (const TPair<FGameplayTag, FComboGraphCueContainer>& Pair : Node->CuesContainerMap)
		{
			if (!Pair.Key.IsValid()) AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InvalidCueContainerTag"), Node, nullptr, TEXT("CuesContainerMap"), TEXT("Cue container has an invalid map key."), TEXT("Gameplay event dispatch cannot address this entry."));
			for (int32 Index = 0; Index < Pair.Value.Definitions.Num(); ++Index)
			{
				const FComboGraphCueContainerDefinition& Definition = Pair.Value.Definitions[Index];
				if (Definition.GameplayCueTags.IsEmpty()) AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingCueTag"), Node, nullptr, TEXT("CuesContainerMap.Definitions.GameplayCueTags"), TEXT("Cue definition has no valid GameplayCue tag."), FString::Printf(TEXT("%s definition %d"), *Pair.Key.ToString(), Index));
				bool bResolvable = true;
				switch (Definition.CueSourceObjectType)
				{
				case EComboGraphCueSourceObjectType::Niagara: bResolvable = !Definition.NiagaraSystem.IsNull() && Definition.NiagaraSystem.LoadSynchronous() != nullptr; break;
				case EComboGraphCueSourceObjectType::Cascade: bResolvable = !Definition.CascadeSystem.IsNull() && Definition.CascadeSystem.LoadSynchronous() != nullptr; break;
				case EComboGraphCueSourceObjectType::Sound: bResolvable = !Definition.SoundEffect.IsNull() && Definition.SoundEffect.LoadSynchronous() != nullptr; break;
				default: break;
				}
				if (!bResolvable) AddFinding(Report, EBertaComboGraphValidationSeverity::Error, TEXT("CG.UnresolvedCueSource"), Node, nullptr, TEXT("CuesContainerMap.Definitions"), TEXT("Selected cue source type has no resolvable matching asset."), FString::Printf(TEXT("%s definition %d"), *Pair.Key.ToString(), Index));
			}
		}
	}
}

namespace BertaComboGraphValidationRules
{
	TSet<FString> ReachableFrom(const TMap<FString, TArray<FString>>& Adjacency, const FString& Root)
	{
		TSet<FString> Result;
		TArray<FString> Pending { Root };
		while (!Pending.IsEmpty())
		{
			const FString Node = Pending.Pop(EAllowShrinking::No);
			if (Result.Contains(Node)) continue;
			Result.Add(Node);
			if (const TArray<FString>* Children = Adjacency.Find(Node)) Pending.Append(*Children);
		}
		return Result;
	}

	bool HasCycle(const TMap<FString, TArray<FString>>& Adjacency)
	{
		TSet<FString> Visited;
		TSet<FString> Stack;
		TFunction<bool(const FString&)> Visit = [&](const FString& Node)
		{
			if (Stack.Contains(Node)) return true;
			if (Visited.Contains(Node)) return false;
			Visited.Add(Node);
			Stack.Add(Node);
			if (const TArray<FString>* Children = Adjacency.Find(Node)) for (const FString& Child : *Children) if (Visit(Child)) return true;
			Stack.Remove(Node);
			return false;
		};
		for (const TPair<FString, TArray<FString>>& Pair : Adjacency) if (Visit(Pair.Key)) return true;
		return false;
	}

	bool HasDuplicateExactSignatures(const TArray<FString>& Signatures)
	{
		TSet<FString> Seen;
		for (const FString& Signature : Signatures)
		{
			if (Seen.Contains(Signature)) return true;
			Seen.Add(Signature);
		}
		return false;
	}
}

bool UBertaComboGraphValidationLibrary::IsValidNormalizedRange(const float Start, const float End)
{
	return FMath::IsFinite(Start) && FMath::IsFinite(End) && Start >= 0.0f && End <= 1.0f && Start < End;
}

void UBertaComboGraphValidationLibrary::SortAndCount(FBertaComboGraphValidationReport& Report)
{
	Report.Findings.Sort([](const FBertaComboGraphValidationFinding& A, const FBertaComboGraphValidationFinding& B)
	{
		return MakeTuple(2 - static_cast<uint8>(A.Severity), A.Code, A.NodePath, A.EdgePath, A.PropertyPath, A.Message) < MakeTuple(2 - static_cast<uint8>(B.Severity), B.Code, B.NodePath, B.EdgePath, B.PropertyPath, B.Message);
	});
	Report.ErrorCount = Report.WarningCount = Report.InfoCount = 0;
	for (const FBertaComboGraphValidationFinding& Finding : Report.Findings)
	{
		if (Finding.Severity == EBertaComboGraphValidationSeverity::Error) ++Report.ErrorCount;
		else if (Finding.Severity == EBertaComboGraphValidationSeverity::Warning) ++Report.WarningCount;
		else ++Report.InfoCount;
	}
}

bool UBertaComboGraphValidationLibrary::BuildValidationReport(UComboGraph* Graph, FBertaComboGraphValidationReport& OutReport)
{
	OutReport = FBertaComboGraphValidationReport();
	if (!Graph) return false;
	OutReport.AssetPath = Graph->GetPathName();
	if (!Graph->EntryNode) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingEntryNode"), nullptr, nullptr, TEXT("EntryNode"), TEXT("Graph has no Entry node."), TEXT("Runtime start requires a valid entry topology."));
	if (!Graph->FirstNode) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingFirstNode"), nullptr, nullptr, TEXT("FirstNode"), TEXT("Graph has no first executable node."), TEXT("Runtime start cannot advance to an animation node."));

	TSet<UComboGraphNodeBase*> Declared;
	for (UComboGraphNodeBase* Node : Graph->AllNodes) if (Node) Declared.Add(Node);
	if (Graph->EntryNode && !Declared.Contains(Graph->EntryNode)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.EntryNotDeclared"), Graph->EntryNode, nullptr, TEXT("AllNodes"), TEXT("EntryNode is absent from AllNodes."), TEXT("Serialized graph topology collections disagree."));
	if (Graph->EntryNode) Declared.Add(Graph->EntryNode);
	if (Graph->FirstNode && !Declared.Contains(Graph->FirstNode)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.FirstNodeNotDeclared"), Graph->FirstNode, nullptr, TEXT("FirstNode"), TEXT("FirstNode is absent from the declared graph nodes."), TEXT("Runtime may start from a node outside the serialized graph topology."));
	TSet<UComboGraphNodeBase*> Reachable;
	TArray<UComboGraphNodeBase*> Pending;
	if (Graph->EntryNode) Pending.Add(Graph->EntryNode);
	while (!Pending.IsEmpty())
	{
		UComboGraphNodeBase* Node = Pending.Pop(EAllowShrinking::No);
		if (!Node || Reachable.Contains(Node)) continue;
		Reachable.Add(Node);
		for (UComboGraphNodeBase* Child : Node->ChildrenNodes) Pending.Add(Child);
	}
	for (UComboGraphNodeBase* Node : Graph->AllNodes) if (Node && !Reachable.Contains(Node)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Warning, TEXT("CG.UnreachableNode"), Node, nullptr, TEXT("AllNodes"), TEXT("Node is not reachable from Entry."), TEXT("It cannot participate in runtime traversal."));

	TMap<FString, TArray<FString>> Adjacency;
	for (UComboGraphNodeBase* Node : Declared)
	{
		TArray<FString>& Children = Adjacency.FindOrAdd(GetPathNameSafe(Node));
		for (UComboGraphNodeBase* Child : Node->ChildrenNodes) if (Child) Children.Add(GetPathNameSafe(Child));
	}
	const bool bHasCycle = BertaComboGraphValidationRules::HasCycle(Adjacency);
#if WITH_EDITORONLY_DATA
	if (bHasCycle && !Graph->bCanBeCyclical) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.ForbiddenCycle"), nullptr, nullptr, TEXT("bCanBeCyclical"), TEXT("Graph contains a cycle while cycles are disabled."), TEXT("Cycle detection used an explicit recursion stack and did not call GetLevelNum()."));
#endif

	int32 ConduitCount = 0;
	bool bUsesCues = false;
	for (UComboGraphNodeBase* Node : Declared)
	{
		if (!Node) continue;
		for (UComboGraphNodeBase* Parent : Node->ParentNodes)
		{
			if (!Parent || !Declared.Contains(Parent)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.UndeclaredParent"), Node, nullptr, TEXT("ParentNodes"), TEXT("Node references a null or undeclared parent."), *GetPathNameSafe(Parent));
			else if (!Parent->ChildrenNodes.Contains(Node)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InconsistentParentChild"), Node, nullptr, TEXT("ParentNodes"), TEXT("Parent does not contain the reverse child reference."), *GetPathNameSafe(Parent));
		}
		if (Node->ChildrenNodes.Contains(Node)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.SelfLink"), Node, nullptr, TEXT("ChildrenNodes"), TEXT("Node links to itself."), TEXT("Self-links are an ambiguous one-node cycle."));
		TSet<UComboGraphNodeBase*> UniqueChildren;
		TMap<FString, UComboGraphEdge*> ExactSignatures;
		TMap<FString, UComboGraphEdge*> ActionSignatures;
		for (UComboGraphNodeBase* Child : Node->ChildrenNodes)
		{
			if (!Child) { AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.NullChild"), Node, nullptr, TEXT("ChildrenNodes"), TEXT("Node contains a null child reference."), TEXT("Runtime traversal cannot resolve it.")); continue; }
			if (!Declared.Contains(Child)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.UndeclaredChild"), Node, nullptr, TEXT("ChildrenNodes"), TEXT("Node references a child absent from the declared graph nodes."), *GetPathNameSafe(Child));
			if (UniqueChildren.Contains(Child)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.DuplicateLink"), Node, nullptr, TEXT("ChildrenNodes"), TEXT("The same exact child link appears more than once."), *GetPathNameSafe(Child));
			UniqueChildren.Add(Child);
			if (!Child->ParentNodes.Contains(Node)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InconsistentParentChild"), Node, nullptr, TEXT("ParentNodes"), TEXT("Child does not contain the reverse parent reference."), *GetPathNameSafe(Child));
			const TObjectPtr<UComboGraphEdge>* EdgePtr = Node->Edges.Find(Child);
			UComboGraphEdge* Edge = EdgePtr ? EdgePtr->Get() : nullptr;
			if (!Edge) { AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingEdge"), Node, nullptr, TEXT("Edges"), TEXT("Executable child link has no edge object."), *GetPathNameSafe(Child)); continue; }
			if (Edge->StartNode != Node || Edge->EndNode != Child) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InconsistentEdgeEndpoints"), Node, Edge, TEXT("StartNode/EndNode"), TEXT("Edge endpoints do not match its map relationship."), *GetPathNameSafe(Child));
			if (Node->IsA<UComboGraphNodeEntry>()) continue;
			if (!Edge->TransitionInput) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingTransitionInput"), Node, Edge, TEXT("TransitionInput"), TEXT("Executable transition has no Input Action."), TEXT("Stock input binding cannot register this edge."));
			if (!StaticEnum<EComboGraphTransitionBehavior>()->IsValidEnumValue(static_cast<int64>(Edge->TransitionBehavior))) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InvalidTransitionBehavior"), Node, Edge, TEXT("TransitionBehavior"), TEXT("Transition behavior contains an invalid serialized enum value."), FString::FromInt(static_cast<int32>(Edge->TransitionBehavior)));
			const FString Exact = TransitionSignature(Edge);
			const bool bExactDuplicate = ExactSignatures.Contains(Exact);
			if (UComboGraphEdge** Existing = ExactSignatures.Find(Exact)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.AmbiguousTransition"), Node, Edge, TEXT("TransitionInput/TriggerEvent"), TEXT("Sibling transitions share the same Input Action and trigger."), FString::Printf(TEXT("First-match order chooses between %s and %s."), *GetPathNameSafe(*Existing), *GetPathNameSafe(Edge)));
			else ExactSignatures.Add(Exact, Edge);
			const FString Action = GetPathNameSafe(Edge->TransitionInput);
			if (Edge->TransitionInput && ActionSignatures.Contains(Action) && !bExactDuplicate) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Warning, TEXT("CG.SharedTransitionInput"), Node, Edge, TEXT("TransitionInput"), TEXT("Sibling transitions share an Input Action with different triggers."), TEXT("This is legal but input configuration and first-match traversal should be reviewed."));
			else if (Edge->TransitionInput) ActionSignatures.Add(Action, Edge);

			if (UComboGraphNodeAnimBase* AnimNode = Cast<UComboGraphNodeAnimBase>(Node))
			{
				FAnimNotifyEvent* TransitionNotify = nullptr;
				if (Edge->TransitionBehavior == EComboGraphTransitionBehavior::OnAnimNotifyClass)
				{
					UClass* NotifyClass = Edge->TransitionAnimNotify.LoadSynchronous();
					if (!NotifyClass) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingTransitionNotifyClass"), Node, Edge, TEXT("TransitionAnimNotify"), TEXT("Notify-class transition has no resolvable UAnimNotify class."), TEXT("Runtime searches UAnimNotify frames, not UAnimNotifyState ranges."));
					else TransitionNotify = AnimNode->GetAnimNotify(TSubclassOf<UAnimNotify>(NotifyClass));
				}
				else if (Edge->TransitionBehavior == EComboGraphTransitionBehavior::OnAnimNotifyName)
				{
					if (Edge->TransitionAnimNotifyName.IsNone()) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingTransitionNotifyName"), Node, Edge, TEXT("TransitionAnimNotifyName"), TEXT("Notify-name transition has an empty notify name."), TEXT("Runtime cannot resolve a trigger time."));
					else TransitionNotify = AnimNode->GetAnimNotify(Edge->TransitionAnimNotifyName);
				}
				if ((Edge->TransitionBehavior == EComboGraphTransitionBehavior::OnAnimNotifyClass || Edge->TransitionBehavior == EComboGraphTransitionBehavior::OnAnimNotifyName) && !TransitionNotify)
					AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingTransitionNotify"), Node, Edge, TEXT("TransitionBehavior"), TEXT("Configured transition notify is absent from the resolved animation under Combo Graph runtime lookup semantics."), TEXT("The queued transition cannot receive its notify trigger time."));
				float EffectiveWindowEnd = -1.0f;
				if (FAnimNotifyEvent* DirectWindow = AnimNode->GetAnimNotify(TSubclassOf<UAnimNotifyState>(UComboGraphANS_ComboWindow::StaticClass())))
				{
					EffectiveWindowEnd = DirectWindow->GetEndTriggerTime();
				}
				else if (UAnimSequenceBase* Animation = AnimNode->GetAnimationAsset(); Animation && !FindAuthoredComboWindow(Animation))
				{
					FComboGraphNotifyStateAutoSetup AutoWindow;
					if (FindEffectiveAutoWindow(AnimNode, AutoWindow) && IsValidNormalizedRange(AutoWindow.StartPercent, AutoWindow.EndPercent))
					{
						EffectiveWindowEnd = Animation->GetPlayLength() * AutoWindow.EndPercent;
					}
				}
				if (TransitionNotify && EffectiveWindowEnd >= 0.0f && TransitionNotify->GetTriggerTime() > EffectiveWindowEnd)
				{
					AddFinding(OutReport, EBertaComboGraphValidationSeverity::Warning, TEXT("CG.TransitionNotifyAfterWindow"), Node, Edge, TEXT("TransitionAnimNotify"), TEXT("Transition notify occurs after the effective Combo Window closes."), TEXT("Combo Graph can retain the queued transition and execute it after window close; verify that timing is intended."));
				}
			}
		}
		for (const TPair<TObjectPtr<UComboGraphNodeBase>, TObjectPtr<UComboGraphEdge>>& Pair : Node->Edges) if (!Node->ChildrenNodes.Contains(Pair.Key)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.OrphanEdge"), Node, Pair.Value, TEXT("Edges"), TEXT("Edge map contains a child not present in ChildrenNodes."), *GetPathNameSafe(Pair.Key));

		if (UComboGraphNodeConduit* Conduit = Cast<UComboGraphNodeConduit>(Node))
		{
			++ConduitCount;
			if (Conduit->ParentNodes.Num() != 1 || Conduit->ParentNodes[0] != Graph->EntryNode) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InvalidConduitPlacement"), Conduit, nullptr, TEXT("ParentNodes"), TEXT("Conduit must be the single node immediately after Entry."), TEXT("Runtime only special-cases that placement."));
			if (Conduit->ChildrenNodes.IsEmpty()) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.ConduitWithoutOutputs"), Conduit, nullptr, TEXT("ChildrenNodes"), TEXT("Conduit has no output transitions."), TEXT("Initial input cannot select a first animation node."));
		}
		if (UComboGraphNodeAnimBase* AnimNode = Cast<UComboGraphNodeAnimBase>(Node); AnimNode && !Node->IsA<UComboGraphNodeEntry>())
		{
			ValidateAnimation(OutReport, Graph, AnimNode);
			FAnimNotifyEvent* AuthoredWindow = FindAuthoredComboWindow(AnimNode->GetAnimationAsset());
			FComboGraphNotifyStateAutoSetup AutoWindow;
			bool bRequiresComboWindow = false;
			for (UComboGraphNodeBase* Child : AnimNode->ChildrenNodes)
			{
				const TObjectPtr<UComboGraphEdge>* EdgePtr = AnimNode->Edges.Find(Child);
				const UComboGraphEdge* Edge = EdgePtr ? EdgePtr->Get() : nullptr;
				bRequiresComboWindow |= Edge && Edge->TransitionInput && !Edge->IsUsingCanceledTriggerEvent();
			}
			if (bRequiresComboWindow && !AuthoredWindow && !FindEffectiveAutoWindow(AnimNode, AutoWindow)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MissingComboWindow"), AnimNode, nullptr, TEXT("NotifyStatesOverrides"), TEXT("Node has no authored or effective Auto Setup Combo Window."), TEXT("Started/Triggered transition input is ignored while the window is closed."));
			if (bRequiresComboWindow && !AuthoredWindow && FindEffectiveAutoWindow(AnimNode, AutoWindow) && !IsValidNormalizedRange(AutoWindow.StartPercent, AutoWindow.EndPercent)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InvalidComboWindowRange"), AnimNode, nullptr, TEXT("NotifyStatesOverrides"), TEXT("Effective Auto Setup Combo Window range is invalid."), FString::Printf(TEXT("Start=%.6f End=%.6f"), AutoWindow.StartPercent, AutoWindow.EndPercent));
			ValidateContainers(OutReport, AnimNode, bUsesCues);
		}
	}
	if (ConduitCount > 1) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.MultipleConduits"), nullptr, nullptr, TEXT("AllNodes"), TEXT("Graph contains more than one conduit."), FString::Printf(TEXT("Count=%d; runtime supports only the Entry-adjacent conduit."), ConduitCount));

	for (UComboGraphNodeBase* Root : Graph->RootNodes) if (!Root || !Declared.Contains(Root)) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.InvalidRootReference"), Root, nullptr, TEXT("RootNodes"), TEXT("RootNodes contains a null or undeclared node."), TEXT("Serialized topology collections disagree."));
	if (bUsesCues)
	{
		const FBertaComboGraphEffectContextCompatibilityReport Compatibility = UBertaComboGraphEffectContextCompatibilityLibrary::InspectConfiguredGlobals();
		if (!Compatibility.bCueContainersSafe) AddFinding(OutReport, EBertaComboGraphValidationSeverity::Error, TEXT("CG.IncompatibleEffectContext"), nullptr, nullptr, TEXT("AllocGameplayEffectContext"), TEXT("Graph uses Cue Containers without a proven Combo Graph-compatible allocated EffectContext."), Compatibility.Evidence);
	}
	SortAndCount(OutReport);
	return true;
}

FString UBertaComboGraphValidationLibrary::FormatValidationReport(const FBertaComboGraphValidationReport& Report)
{
	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("ComboGraph=%s Errors=%d Warnings=%d Info=%d"), *Report.AssetPath, Report.ErrorCount, Report.WarningCount, Report.InfoCount));
	for (const FBertaComboGraphValidationFinding& Finding : Report.Findings)
	{
		Lines.Add(FString::Printf(TEXT("[%s] %s | Node=%s Edge=%s Property=%s | %s | %s"), *StaticEnum<EBertaComboGraphValidationSeverity>()->GetNameStringByValue(static_cast<int64>(Finding.Severity)), *Finding.Code, *Finding.NodePath, *Finding.EdgePath, *Finding.PropertyPath, *Finding.Message, *Finding.Evidence));
	}
	return FString::Join(Lines, TEXT("\n"));
}
