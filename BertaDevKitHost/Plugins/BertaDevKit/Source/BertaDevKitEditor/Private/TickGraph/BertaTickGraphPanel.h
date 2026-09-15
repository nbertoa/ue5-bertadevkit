#pragma once

#include "GraphEditor.h"
#include "TickGraph/BertaTickGraphSnapshot.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"

class AActor;
class SGraphEditor;
class UBertaTickGraphEdGraph;

class SBertaTickGraphPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBertaTickGraphPanel) {}
	SLATE_END_ARGS()
	void Construct(const FArguments& InArgs);
	virtual ~SBertaTickGraphPanel() override;

private:
	FReply UseSelectedActor();
	FReply CaptureGraph();
	FReply ClearSnapshot();
	FReply SelectLiveOwner();
	void OnBeginPIE(bool bSimulating);
	void OnEndPIE(bool bSimulating);
	void OnGraphSelectionChanged(const FGraphPanelSelectionSet& Selection);
	void RebuildGraph();
	FText StatusText() const;
	FText CapturedText() const;
	FText CountsText() const;
	FText DetailsText() const;
	bool CanSelectLiveOwner() const;

	TStrongObjectPtr<UBertaTickGraphEdGraph> Graph;
	TSharedPtr<SGraphEditor> GraphEditor;
	BertaTickGraph::FSnapshot Snapshot;
	TWeakObjectPtr<AActor> CapturedActor;
	FString CapturedSummary;
	FString Status;
	int32 SelectedId = INDEX_NONE;
	uint64 SessionGeneration = 0;
	uint64 CaptureGeneration = 0;
	uint64 SnapshotGeneration = 0;
	bool bHasSnapshot = false;
	FDelegateHandle BeginPIEHandle;
	FDelegateHandle EndPIEHandle;
};
