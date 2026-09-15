#include "TickGraph/BertaTickGraphPanel.h"

#include "Editor.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/Selection.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Selection.h"
#include "TickGraph/BertaTickGraphAnalyzer.h"
#include "TickGraph/BertaTickGraphEdGraph.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const TCHAR* RoleName(BertaTickGraph::ERole Role)
	{
		switch (Role)
		{
		case BertaTickGraph::ERole::TargetActorPrimary: return TEXT("Selected Actor Primary Tick");
		case BertaTickGraph::ERole::TargetComponentPrimary: return TEXT("Selected Component Primary Tick");
		case BertaTickGraph::ERole::CustomPrerequisite: return TEXT("Custom Prerequisite Tick");
		default: return TEXT("External Prerequisite Tick");
		}
	}
	const TCHAR* KindName(BertaTickGraph::EKind Kind)
	{
		switch (Kind)
		{
		case BertaTickGraph::EKind::ActorPrimary: return TEXT("Actor Primary Tick");
		case BertaTickGraph::EKind::ComponentPrimary: return TEXT("Component Primary Tick");
		default: return TEXT("Custom Tick Function");
		}
	}
}

void SBertaTickGraphPanel::Construct(const FArguments& InArgs)
{
	Graph.Reset(NewObject<UBertaTickGraphEdGraph>(GetTransientPackage(), NAME_None, RF_Transient));
	Graph->Schema = UBertaTickGraphSchema::StaticClass();
	Status = TEXT("Select exactly one PIE Actor to inspect. Primary Actor/Component ticks are seeds; reachable custom ticks are included.");
	BeginPIEHandle = FEditorDelegates::BeginPIE.AddSP(this, &SBertaTickGraphPanel::OnBeginPIE);
	EndPIEHandle = FEditorDelegates::EndPIE.AddSP(this, &SBertaTickGraphPanel::OnEndPIE);

	SGraphEditor::FGraphEditorEvents Events;
	Events.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &SBertaTickGraphPanel::OnGraphSelectionChanged);
	FGraphAppearanceInfo Appearance;
	Appearance.CornerText = FText::FromString(TEXT("BERTA TICK GRAPH"));
	Appearance.ReadOnlyText = FText::FromString(TEXT("Read-only Tick prerequisite snapshot"));
	GraphEditor = SNew(SGraphEditor)
		.GraphToEdit(Graph.Get()).GraphEvents(Events).Appearance(Appearance)
		.IsEditable(false).AllowConnectionSlicing(false).ShowGraphStateOverlay(false);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 6, 8, 0)
		[ SNew(STextBlock).Text(FText::FromString(TEXT("What Ticks Before Me?  |  Prerequisite → Dependent (configured structure, not execution timing)"))) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[ SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[ SNew(SButton).Text(FText::FromString(TEXT("Use Selected Actor"))).OnClicked(this, &SBertaTickGraphPanel::UseSelectedActor) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[ SNew(SButton).Text(FText::FromString(TEXT("Capture Tick Graph"))).OnClicked(this, &SBertaTickGraphPanel::CaptureGraph) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[ SNew(SButton).Text(FText::FromString(TEXT("Clear Snapshot")))
				.ToolTipText(FText::FromString(TEXT("Clear diagnostic data only; Tick settings and prerequisites are untouched.")))
				.OnClicked(this, &SBertaTickGraphPanel::ClearSnapshot) ] ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 2)
		[ SNew(STextBlock).Text(this, &SBertaTickGraphPanel::StatusText).AutoWrapText(true) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 2)
		[ SNew(STextBlock).Text(this, &SBertaTickGraphPanel::CapturedText) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 2)
		[ SNew(STextBlock).Text(this, &SBertaTickGraphPanel::CountsText) ]
		+ SVerticalBox::Slot().FillHeight(1).Padding(8)
		[ GraphEditor.ToSharedRef() ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[ SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1)
			[ SNew(STextBlock).Text(this, &SBertaTickGraphPanel::DetailsText).AutoWrapText(true) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4)
			[ SNew(SButton).Text(FText::FromString(TEXT("Select Live Owner")))
				.IsEnabled(this, &SBertaTickGraphPanel::CanSelectLiveOwner)
				.OnClicked(this, &SBertaTickGraphPanel::SelectLiveOwner) ] ]
	];
}

SBertaTickGraphPanel::~SBertaTickGraphPanel()
{
	FEditorDelegates::BeginPIE.Remove(BeginPIEHandle);
	FEditorDelegates::EndPIE.Remove(EndPIEHandle);
}

FReply SBertaTickGraphPanel::UseSelectedActor()
{
	if (!GEditor || !GEditor->IsPlayingSessionInEditor()) { Status = TEXT("Select exactly one PIE Actor to inspect."); return FReply::Handled(); }
	USelection* Selection = GEditor->GetSelectedActors();
	if (!Selection || Selection->CountSelections(AActor::StaticClass()) != 1)
	{
		Status = TEXT("Select exactly one PIE Actor to inspect.");
		return FReply::Handled();
	}
	AActor* Actor = Selection->GetTop<AActor>();
	if (!IsValid(Actor) || Actor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		Status = TEXT("The selected Actor is not valid.");
		return FReply::Handled();
	}
	if (!Actor->GetWorld() || Actor->GetWorld()->WorldType != EWorldType::PIE)
	{
		Status = TEXT("The selected Actor does not belong to a PIE world.");
		return FReply::Handled();
	}
	CapturedActor = Actor;
	CapturedSummary = FString::Printf(TEXT("%s | %s | %s | %s"), *Actor->GetName(), *Actor->GetClass()->GetName(),
		*Actor->GetPathName(), *Actor->GetWorld()->GetName());
	CaptureGeneration = SessionGeneration;
	Snapshot = {};
	bHasSnapshot = false;
	Status = TEXT("PIE Actor captured weakly. Press Capture Tick Graph.");
	RebuildGraph();
	return FReply::Handled();
}

FReply SBertaTickGraphPanel::CaptureGraph()
{
	AActor* Actor = CapturedActor.Get();
	if (!Actor) { Status = TEXT("The captured target is no longer valid."); return FReply::Handled(); }
	if (!GEditor || !GEditor->IsPlayingSessionInEditor() || CaptureGeneration != SessionGeneration
		|| !Actor->GetWorld() || Actor->GetWorld()->WorldType != EWorldType::PIE)
	{
		Status = TEXT("The captured target is outside the active PIE session. Capture a new PIE Actor.");
		return FReply::Handled();
	}
	BertaTickGraph::FSnapshot NewSnapshot;
	FString Error;
	if (!BertaTickGraph::Capture(Actor, NewSnapshot, Error)) { Status = Error; return FReply::Handled(); }
	Snapshot = MoveTemp(NewSnapshot);
	bHasSnapshot = true;
	SnapshotGeneration = SessionGeneration;
	Status = Snapshot.Nodes.IsEmpty() ? TEXT("No inspectable primary Tick Functions were found on the selected Actor or its components.")
		: Snapshot.Edges.IsEmpty() ? TEXT("No Tick prerequisites were found for the selected Actor or its direct Components.")
		: TEXT("Tick prerequisite graph captured. This is structural configuration, not a frame trace.");
	if (Snapshot.bTruncated) { Status += TEXT(" Tick graph capture was truncated at the diagnostic safety limits."); }
	if (Snapshot.bCycle) { Status += TEXT(" A cycle was encountered while traversing Tick prerequisites."); }
	if (Snapshot.InvalidPrerequisiteCount) { Status += TEXT(" Invalid prerequisite records were omitted."); }
	RebuildGraph();
	return FReply::Handled();
}

FReply SBertaTickGraphPanel::ClearSnapshot()
{
	CapturedActor.Reset();
	CapturedSummary.Reset();
	Snapshot = {};
	bHasSnapshot = false;
	Status = TEXT("Tick Graph snapshot cleared; runtime Tick configuration was not changed.");
	RebuildGraph();
	return FReply::Handled();
}

void SBertaTickGraphPanel::OnBeginPIE(bool bSimulating)
{
	++SessionGeneration;
	CapturedActor.Reset();
	if (bHasSnapshot) { Snapshot.bStale = true; Status = TEXT("Snapshot from an earlier PIE session; capture a new target. Live navigation disabled."); }
	else { CapturedSummary.Reset(); }
}

void SBertaTickGraphPanel::OnEndPIE(bool bSimulating)
{
	++SessionGeneration;
	CapturedActor.Reset();
	if (bHasSnapshot) { Snapshot.bStale = true; Status = TEXT("PIE has ended. This snapshot is stale; copied Tick details remain readable."); }
	else { CapturedSummary.Reset(); }
}

void SBertaTickGraphPanel::OnGraphSelectionChanged(const FGraphPanelSelectionSet& Selection)
{
	SelectedId = INDEX_NONE;
	for (UObject* Object : Selection)
	{
		if (const UBertaTickGraphEdNode* Node = Cast<UBertaTickGraphEdNode>(Object)) { SelectedId = Node->SnapshotId; break; }
	}
}

void SBertaTickGraphPanel::RebuildGraph()
{
	SelectedId = INDEX_NONE;
	Graph->Nodes.Reset();
	TArray<UBertaTickGraphEdNode*> VisualNodes;
	VisualNodes.Reserve(Snapshot.Nodes.Num());
	for (const BertaTickGraph::FNode& Source : Snapshot.Nodes)
	{
		UBertaTickGraphEdNode* Node = NewObject<UBertaTickGraphEdNode>(Graph.Get(), NAME_None, RF_Transient);
		Node->SnapshotId = Source.Id;
		Node->CopiedName = Source.Name;
		Node->CopiedKind = KindName(Source.Kind);
		Node->CopiedGroup = Source.TickGroup;
		Node->CopiedPath = Source.OwnerPath;
		Node->Role = Source.Role;
		Node->bActive = Source.bEnabled && Source.bRegistered;
		Node->NodePosX = Source.Layer * 420;
		Node->NodePosY = Source.Row * 220;
		Graph->AddNode(Node, false, false);
		VisualNodes.Add(Node);
	}
	for (int32 Index = 0; Index < Snapshot.Edges.Num(); ++Index)
	{
		const BertaTickGraph::FEdge& Edge = Snapshot.Edges[Index];
		const FName PinName(*FString::Printf(TEXT("Prerequisite%d"), Index));
		UEdGraphPin* Out = VisualNodes[Edge.Prerequisite]->CreatePin(EGPD_Output, TEXT("Tick Prerequisite"), PinName);
		UEdGraphPin* In = VisualNodes[Edge.Dependent]->CreatePin(EGPD_Input, TEXT("Tick Prerequisite"), PinName);
		const FText Reason = FText::FromString(TEXT("Configured prerequisite: source must complete before destination may tick when enabled and registered."));
		Out->PinFriendlyName = Reason;
		In->PinFriendlyName = Reason;
		Out->MakeLinkTo(In);
	}
	GraphEditor->ClearSelectionSet();
	GraphEditor->NotifyGraphChanged();
}

FText SBertaTickGraphPanel::StatusText() const { return FText::FromString(Status); }
FText SBertaTickGraphPanel::CapturedText() const
{
	if (bHasSnapshot && Snapshot.bStale)
	{
		return FText::FromString(FString::Printf(TEXT("Last target (stale): %s | %s | %s"),
			*Snapshot.TargetName, *Snapshot.TargetClass, *Snapshot.TargetPath));
	}
	return FText::FromString(CapturedSummary.IsEmpty() ? TEXT("Captured target: none") : TEXT("Captured target (weak): ") + CapturedSummary);
}
FText SBertaTickGraphPanel::CountsText() const
{
	if (!bHasSnapshot) { return FText::FromString(TEXT("No capture yet.")); }
	return FText::FromString(FString::Printf(TEXT("%d seeds | %d Tick Functions | %d prerequisites | %d external | %d invalid omitted | %.1f ms%s"),
		Snapshot.SeedCount, Snapshot.Nodes.Num(), Snapshot.Edges.Num(), Snapshot.ExternalCount,
		Snapshot.InvalidPrerequisiteCount, Snapshot.DurationMs, Snapshot.bStale ? TEXT(" | STALE") : TEXT("")));
}
FText SBertaTickGraphPanel::DetailsText() const
{
	if (!Snapshot.Nodes.IsValidIndex(SelectedId)) { return FText::FromString(TEXT("Select a graph node to inspect copied Tick details.")); }
	const BertaTickGraph::FNode& Node = Snapshot.Nodes[SelectedId];
	return FText::FromString(FString::Printf(TEXT("%s | %s | Kind: %s | Owner: %s (%s) | Path: %s\nConfigured: %s → end %s | Can Ever Tick: %s | Enabled: %s | Registered: %s | Interval: %.3f s\nHigh Priority: %s | Any Thread Allowed: %s | Tick When Paused: %s | Start Enabled: %s | Dedicated Server: %s | Manual Dispatch: %s | Direct Prerequisites: %d | Live: %s"),
		*Node.Name, RoleName(Node.Role), KindName(Node.Kind), *Node.OwnerName, *Node.OwnerClass, *Node.OwnerPath,
		*Node.TickGroup, *Node.EndTickGroup,
		Node.bCanEverTick ? TEXT("yes") : TEXT("no"), Node.bEnabled ? TEXT("yes") : TEXT("no"), Node.bRegistered ? TEXT("yes") : TEXT("no"), Node.Interval,
		Node.bHighPriority ? TEXT("yes") : TEXT("no"), Node.bAnyThread ? TEXT("yes") : TEXT("no"),
		Node.bPaused ? TEXT("yes") : TEXT("no"), Node.bStartEnabled ? TEXT("yes") : TEXT("no"),
		Node.bDedicatedServer ? TEXT("yes") : TEXT("no"), Node.bDispatchManually ? TEXT("yes") : TEXT("no"),
		Node.DirectPrerequisites, Node.LiveOwner.IsValid() && !Snapshot.bStale ? TEXT("yes") : TEXT("no")));
}

bool SBertaTickGraphPanel::CanSelectLiveOwner() const
{
	if (!bHasSnapshot || Snapshot.bStale || SnapshotGeneration != SessionGeneration || !Snapshot.Nodes.IsValidIndex(SelectedId)
		|| !GEditor || !GEditor->IsPlayingSessionInEditor()) { return false; }
	UObject* Owner = Snapshot.Nodes[SelectedId].LiveOwner.Get();
	return IsValid(Owner) && Owner->GetWorld() && Owner->GetWorld()->WorldType == EWorldType::PIE;
}
FReply SBertaTickGraphPanel::SelectLiveOwner()
{
	if (!CanSelectLiveOwner()) { return FReply::Handled(); }
	UObject* Owner = Snapshot.Nodes[SelectedId].LiveOwner.Get();
	if (AActor* Actor = Cast<AActor>(Owner))
	{
		GEditor->SelectNone(true, false, false);
		GEditor->SelectActor(Actor, true, true, true);
	}
	else
	{
		GEditor->GetSelectedObjects()->DeselectAll();
		GEditor->GetSelectedObjects()->Select(Owner);
		GEditor->GetSelectedObjects()->NoteSelectionChanged();
	}
	return FReply::Handled();
}
