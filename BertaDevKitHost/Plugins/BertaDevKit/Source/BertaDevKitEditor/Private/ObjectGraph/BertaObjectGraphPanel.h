#pragma once

#include "GraphEditor.h"
#include "ObjectGraph/BertaObjectGraphSnapshot.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"

class AActor;
class SGraphEditor;
class UBertaObjectGraphEdGraph;

class SBertaObjectGraphPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBertaObjectGraphPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SBertaObjectGraphPanel() override;

private:
	FReply UseSelectedActor();
	FReply AnalyzeCapturedActor();
	FReply Clear();
	FReply SelectLiveObject();
	void OnBeginPIE(bool bSimulating);
	void OnEndPIE(bool bSimulating);
	void OnGraphSelectionChanged(const FGraphPanelSelectionSet& Selection);
	void RebuildGraph();
	FText StatusText() const;
	FText CapturedText() const;
	FText CountsText() const;
	FText SelectedText() const;
	bool CanSelectLiveObject() const;

	TStrongObjectPtr<UBertaObjectGraphEdGraph> Graph;
	TSharedPtr<SGraphEditor> GraphEditor;
	BertaObjectGraph::FSnapshot Snapshot;
	TWeakObjectPtr<AActor> CapturedActor;
	FString CapturedSummary;
	FString Status;
	int32 SelectedSnapshotId = INDEX_NONE;
	uint64 SessionGeneration = 0;
	uint64 CaptureGeneration = 0;
	uint64 AnalysisGeneration = 0;
	bool bHasAnalysis = false;
	bool bStale = false;
	FDelegateHandle BeginPIEHandle;
	FDelegateHandle EndPIEHandle;
};
