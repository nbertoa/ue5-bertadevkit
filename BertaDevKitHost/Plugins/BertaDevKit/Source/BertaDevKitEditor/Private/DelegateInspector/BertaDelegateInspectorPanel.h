#pragma once

#include "DelegateInspector/BertaDelegateInspectorSnapshot.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"

class AActor;

namespace BertaDelegateInspector
{
	enum class ERowKind : uint8 { Source, Delegate, Binding };
	struct FRow
	{
		ERowKind Kind = ERowKind::Source;
		int32 SourceIndex = INDEX_NONE;
		int32 DelegateIndex = INDEX_NONE;
		int32 BindingIndex = INDEX_NONE;
		FString Name;
		FString Type;
		FString FunctionOrCount;
		FString Status;
		TArray<TSharedPtr<FRow>> Children;
	};
}

class SBertaDelegateInspectorPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBertaDelegateInspectorPanel) {}
	SLATE_END_ARGS()
	void Construct(const FArguments& InArgs);
	virtual ~SBertaDelegateInspectorPanel() override;

private:
	using FRow = BertaDelegateInspector::FRow;
	FReply UseSelectedActor();
	FReply RefreshBindings();
	FReply ClearSnapshot();
	FReply SelectSourceObject();
	FReply SelectListenerObject();
	void OnBeginPIE(bool bSimulating);
	void OnEndPIE(bool bSimulating);
	void OnBoundOnlyChanged(ECheckBoxState NewState);
	void OnSearchChanged(const FText& NewText);
	void RebuildRows();
	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FRow> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void GetRowChildren(TSharedPtr<FRow> Item, TArray<TSharedPtr<FRow>>& OutChildren) const;
	void OnRowSelectionChanged(TSharedPtr<FRow> Item, ESelectInfo::Type SelectInfo);
	FText StatusText() const;
	FText TargetText() const;
	FText CountsText() const;
	FText DetailsText() const;
	bool CanSelectSourceObject() const;
	bool CanSelectListenerObject() const;
	bool CanNavigate(UObject* Object) const;
	FReply SelectObject(UObject* Object);

	TWeakObjectPtr<AActor> CapturedActor;
	FString CapturedSummary;
	BertaDelegateInspector::FSnapshot Snapshot;
	FString Status;
	FString Search;
	TArray<TSharedPtr<FRow>> RootRows;
	TSharedPtr<FRow> SelectedRow;
	TSharedPtr<STreeView<TSharedPtr<FRow>>> Tree;
	uint64 SessionGeneration = 0;
	uint64 CaptureGeneration = 0;
	uint64 InspectionGeneration = 0;
	bool bHasSnapshot = false;
	bool bBoundOnly = true;
	FDelegateHandle BeginPIEHandle;
	FDelegateHandle EndPIEHandle;
};
