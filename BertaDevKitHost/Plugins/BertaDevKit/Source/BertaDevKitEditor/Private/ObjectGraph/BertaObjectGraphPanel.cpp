#include "ObjectGraph/BertaObjectGraphPanel.h"

#include "Editor.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/Selection.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "ObjectGraph/BertaObjectGraphAnalyzer.h"
#include "ObjectGraph/BertaObjectGraphEdGraph.h"
#include "Selection.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SBertaObjectGraphPanel::Construct(const FArguments& InArgs)
{
	Graph.Reset(NewObject<UBertaObjectGraphEdGraph>(GetTransientPackage(), NAME_None, RF_Transient));
	Graph->Schema = UBertaObjectGraphSchema::StaticClass();
	Status = TEXT("Select exactly one Actor in a PIE world, then capture it.");
	BeginPIEHandle = FEditorDelegates::BeginPIE.AddSP(this, &SBertaObjectGraphPanel::OnBeginPIE);
	EndPIEHandle = FEditorDelegates::EndPIE.AddSP(this, &SBertaObjectGraphPanel::OnEndPIE);

	SGraphEditor::FGraphEditorEvents Events;
	Events.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &SBertaObjectGraphPanel::OnGraphSelectionChanged);
	FGraphAppearanceInfo Appearance;
	Appearance.CornerText = FText::FromString(TEXT("BERTA OBJECT GRAPH"));
	Appearance.ReadOnlyText = FText::FromString(TEXT("Read-only GC snapshot"));
	GraphEditor = SNew(SGraphEditor)
		.GraphToEdit(Graph.Get())
		.GraphEvents(Events)
		.Appearance(Appearance)
		.IsEditable(false)
		.AllowConnectionSlicing(false)
		.ShowGraphStateOverlay(false);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[ SNew(SButton).Text(FText::FromString(TEXT("Use Selected Actor"))).OnClicked(this, &SBertaObjectGraphPanel::UseSelectedActor) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[ SNew(SButton).Text(FText::FromString(TEXT("Analyze"))).OnClicked(this, &SBertaObjectGraphPanel::AnalyzeCapturedActor) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[ SNew(SButton).Text(FText::FromString(TEXT("Clear"))).OnClicked(this, &SBertaObjectGraphPanel::Clear) ]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 2)
		[ SNew(STextBlock).Text(this, &SBertaObjectGraphPanel::StatusText) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 2)
		[ SNew(STextBlock).Text(this, &SBertaObjectGraphPanel::CapturedText) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 2)
		[ SNew(STextBlock).Text(this, &SBertaObjectGraphPanel::CountsText) ]
		+ SVerticalBox::Slot().FillHeight(1)
		[ GraphEditor.ToSharedRef() ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1)
			[ SNew(STextBlock).Text(this, &SBertaObjectGraphPanel::SelectedText).AutoWrapText(true) ]
			+ SHorizontalBox::Slot().AutoWidth()
			[ SNew(SButton).Text(FText::FromString(TEXT("Select Live Object")))
				.IsEnabled(this, &SBertaObjectGraphPanel::CanSelectLiveObject)
				.OnClicked(this, &SBertaObjectGraphPanel::SelectLiveObject) ]
		]
	];
}

SBertaObjectGraphPanel::~SBertaObjectGraphPanel()
{
	FEditorDelegates::BeginPIE.Remove(BeginPIEHandle);
	FEditorDelegates::EndPIE.Remove(EndPIEHandle);
}

FReply SBertaObjectGraphPanel::UseSelectedActor()
{
	if (!GEditor || !GEditor->IsPlayingSessionInEditor())
	{
		Status = TEXT("No active PIE/SIE session.");
		return FReply::Handled();
	}
	USelection* Selection = GEditor->GetSelectedActors();
	if (Selection->CountSelections(AActor::StaticClass()) != 1)
	{
		Status = TEXT("Select exactly one Actor; no or multiple selected Actors are rejected.");
		return FReply::Handled();
	}
	AActor* Actor = Selection->GetTop<AActor>();
	if (!IsValid(Actor) || !Actor->GetWorld() || Actor->GetWorld()->WorldType != EWorldType::PIE)
	{
		Status = TEXT("The selected Actor must belong to an actual PIE world.");
		return FReply::Handled();
	}
	CapturedActor = Actor;
	CapturedSummary = FString::Printf(TEXT("%s | %s | %s"), *Actor->GetName(), *Actor->GetClass()->GetName(), *Actor->GetPathName());
	CaptureGeneration = SessionGeneration;
	Status = TEXT("PIE Actor captured weakly. Press Analyze.");
	bHasAnalysis = false;
	bStale = false;
	Snapshot = {};
	RebuildGraph();
	return FReply::Handled();
}

FReply SBertaObjectGraphPanel::AnalyzeCapturedActor()
{
	AActor* Actor = CapturedActor.Get();
	if (!Actor)
	{
		Status = TEXT("Captured actor expired before analysis.");
		return FReply::Handled();
	}
	if (!GEditor || !GEditor->IsPlayingSessionInEditor() || CaptureGeneration != SessionGeneration
		|| !Actor->GetWorld() || Actor->GetWorld()->WorldType != EWorldType::PIE)
	{
		Status = TEXT("Captured actor is outside the active PIE session.");
		return FReply::Handled();
	}
	BertaObjectGraph::FSnapshot NewSnapshot;
	FString Error;
	if (!BertaObjectGraph::Analyze(Actor, NewSnapshot, Error))
	{
		Status = Error;
		return FReply::Handled();
	}
	Snapshot = MoveTemp(NewSnapshot);
	bHasAnalysis = true;
	bStale = false;
	AnalysisGeneration = SessionGeneration;
	Status = Snapshot.DisplayedChains == 0
		? TEXT("No GC root reference chain was found at analysis time. The object may be unreachable pending GC.")
		: Snapshot.bDirectRoot ? TEXT("Reference chain found (direct root on target).") : TEXT("Reference chain found.");
	RebuildGraph();
	return FReply::Handled();
}

FReply SBertaObjectGraphPanel::Clear()
{
	CapturedActor.Reset();
	CapturedSummary.Reset();
	Snapshot = {};
	bHasAnalysis = false;
	bStale = false;
	Status = TEXT("Snapshot cleared.");
	RebuildGraph();
	return FReply::Handled();
}

void SBertaObjectGraphPanel::OnBeginPIE(bool bSimulating)
{
	++SessionGeneration;
	CapturedActor.Reset();
	if (bHasAnalysis) { bStale = true; Status = TEXT("Snapshot from an earlier PIE session; live navigation disabled."); }
}

void SBertaObjectGraphPanel::OnEndPIE(bool bSimulating)
{
	++SessionGeneration;
	CapturedActor.Reset();
	if (bHasAnalysis) { bStale = true; Status = TEXT("PIE ended. Detached snapshot remains readable; live navigation disabled."); }
}

void SBertaObjectGraphPanel::OnGraphSelectionChanged(const FGraphPanelSelectionSet& Selection)
{
	SelectedSnapshotId = INDEX_NONE;
	for (UObject* Object : Selection)
	{
		if (const UBertaObjectGraphEdNode* Node = Cast<UBertaObjectGraphEdNode>(Object))
		{
			SelectedSnapshotId = Node->SnapshotId;
			break;
		}
	}
}

void SBertaObjectGraphPanel::RebuildGraph()
{
	SelectedSnapshotId = INDEX_NONE;
	Graph->Nodes.Reset();
	TArray<UBertaObjectGraphEdNode*> EdNodes;
	EdNodes.Reserve(Snapshot.Nodes.Num());
	for (const BertaObjectGraph::FNode& SnapshotNode : Snapshot.Nodes)
	{
		UBertaObjectGraphEdNode* Node = NewObject<UBertaObjectGraphEdNode>(Graph.Get(), NAME_None, RF_Transient);
		Node->SnapshotId = SnapshotNode.Id;
		Node->CopiedName = SnapshotNode.Object.Name;
		Node->CopiedClass = SnapshotNode.Object.ClassName;
		Node->CopiedPath = SnapshotNode.Object.Path;
		Node->Role = SnapshotNode.Role;
		Node->NodePosX = SnapshotNode.Layer * 420;
		Node->NodePosY = SnapshotNode.Row * 220;
		Graph->AddNode(Node, false, false);
		EdNodes.Add(Node);
	}
	for (int32 EdgeIndex = 0; EdgeIndex < Snapshot.Edges.Num(); ++EdgeIndex)
	{
		const BertaObjectGraph::FEdge& Edge = Snapshot.Edges[EdgeIndex];
		const FName PinName(*FString::Printf(TEXT("Reference%d"), EdgeIndex));
		UEdGraphPin* OutPin = EdNodes[Edge.From]->CreatePin(EGPD_Output, TEXT("GC Reference"), PinName);
		UEdGraphPin* InPin = EdNodes[Edge.To]->CreatePin(EGPD_Input, TEXT("GC Reference"), PinName);
		OutPin->PinFriendlyName = FText::FromString(Edge.Reason);
		InPin->PinFriendlyName = FText::FromString(Edge.Reason);
		OutPin->MakeLinkTo(InPin);
	}
	GraphEditor->ClearSelectionSet();
	GraphEditor->NotifyGraphChanged();
}

FText SBertaObjectGraphPanel::StatusText() const { return FText::FromString(Status); }
FText SBertaObjectGraphPanel::CapturedText() const
{
	return FText::FromString(CapturedSummary.IsEmpty() ? TEXT("Captured target: none") : TEXT("Captured target (weak): ") + CapturedSummary);
}
FText SBertaObjectGraphPanel::CountsText() const
{
	if (!bHasAnalysis) { return FText::FromString(TEXT("No analysis yet.")); }
	return FText::FromString(FString::Printf(TEXT("Native chains: %d | Displayed: %d%s | Nodes: %d | Edges: %d | Truncated: %s | %.1f ms%s"),
		Snapshot.NativeChains, Snapshot.DisplayedChains,
		Snapshot.bDirectRoot ? TEXT(" (direct root)") : TEXT(""), Snapshot.Nodes.Num(), Snapshot.Edges.Num(),
		Snapshot.bTruncated ? TEXT("yes") : TEXT("no"), Snapshot.DurationMs, bStale ? TEXT(" | STALE") : TEXT("")));
}
FText SBertaObjectGraphPanel::SelectedText() const
{
	if (!Snapshot.Nodes.IsValidIndex(SelectedSnapshotId)) { return FText::FromString(TEXT("Select a graph node to inspect its copied details.")); }
	const BertaObjectGraph::FNode& Node = Snapshot.Nodes[SelectedSnapshotId];
	const TCHAR* Role = Node.Role == BertaObjectGraph::ERole::Root ? TEXT("root")
		: Node.Role == BertaObjectGraph::ERole::Target ? TEXT("target") : TEXT("intermediate");
	return FText::FromString(FString::Printf(TEXT("%s | %s | %s | %s | %s | live: %s"),
		*Node.Object.Name, *Node.Object.ClassName, *Node.Object.Path, Role, *Node.Object.RootFlags,
		Node.Object.LiveObject.IsValid() ? TEXT("yes") : TEXT("no")));
}
bool SBertaObjectGraphPanel::CanSelectLiveObject() const
{
	return Snapshot.Nodes.IsValidIndex(SelectedSnapshotId) && !bStale
		&& AnalysisGeneration == SessionGeneration && GEditor && GEditor->IsPlayingSessionInEditor()
		&& Snapshot.Nodes[SelectedSnapshotId].Object.LiveObject.IsValid();
}
FReply SBertaObjectGraphPanel::SelectLiveObject()
{
	if (!CanSelectLiveObject()) { return FReply::Handled(); }
	UObject* Object = Snapshot.Nodes[SelectedSnapshotId].Object.LiveObject.Get();
	if (AActor* Actor = Cast<AActor>(Object))
	{
		if (!Actor->GetWorld() || Actor->GetWorld()->WorldType != EWorldType::PIE) { return FReply::Handled(); }
		GEditor->SelectNone(true, false, false);
		GEditor->SelectActor(Actor, true, true, true);
	}
	else
	{
		GEditor->GetSelectedObjects()->DeselectAll();
		GEditor->GetSelectedObjects()->Select(Object);
		GEditor->GetSelectedObjects()->NoteSelectionChanged();
	}
	return FReply::Handled();
}
