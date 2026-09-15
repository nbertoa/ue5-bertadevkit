#include "DelegateInspector/BertaDelegateInspectorPanel.h"

#include "DelegateInspector/BertaDelegateInspector.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Selection.h"
#include "UObject/Object.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SExpanderArrow.h"
#include "Widgets/Views/SHeaderRow.h"

namespace
{
	using FRow = BertaDelegateInspector::FRow;
	class SBertaDelegateRow final : public SMultiColumnTableRow<TSharedPtr<FRow>>
	{
	public:
		SLATE_BEGIN_ARGS(SBertaDelegateRow) {}
			SLATE_ARGUMENT(TSharedPtr<FRow>, Item)
		SLATE_END_ARGS()

		void Construct(const FArguments& Args, const TSharedRef<STableViewBase>& OwnerTable)
		{
			Item = Args._Item;
			SMultiColumnTableRow<TSharedPtr<FRow>>::Construct(FSuperRowType::FArguments(), OwnerTable);
		}

		virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& Column) override
		{
			if (Column == TEXT("Name"))
			{
				return SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[ SNew(SExpanderArrow, SharedThis(this)) ]
					+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
					[ SNew(STextBlock).Text(FText::FromString(Item->Name))
						.ColorAndOpacity(Item->Kind == BertaDelegateInspector::ERowKind::Source
							? FAppStyle::Get().GetSlateColor("Colors.AccentBlue")
							: Item->Kind == BertaDelegateInspector::ERowKind::Delegate
								? FAppStyle::Get().GetSlateColor("Colors.AccentGreen")
								: FSlateColor::UseForeground()) ];
			}
			const FString& Value = Column == TEXT("Type") ? Item->Type
				: Column == TEXT("Function") ? Item->FunctionOrCount : Item->Status;
			return SNew(STextBlock).Text(FText::FromString(Value));
		}
	private:
		TSharedPtr<FRow> Item;
	};
}

void SBertaDelegateInspectorPanel::Construct(const FArguments& InArgs)
{
	Status = TEXT("Select exactly one PIE Actor to inspect. Only reflected dynamic multicast delegates are included.");
	BeginPIEHandle = FEditorDelegates::BeginPIE.AddSP(this, &SBertaDelegateInspectorPanel::OnBeginPIE);
	EndPIEHandle = FEditorDelegates::EndPIE.AddSP(this, &SBertaDelegateInspectorPanel::OnEndPIE);
	Tree = SNew(STreeView<TSharedPtr<FRow>>)
		.TreeItemsSource(&RootRows)
		.OnGenerateRow(this, &SBertaDelegateInspectorPanel::GenerateRow)
		.OnGetChildren(this, &SBertaDelegateInspectorPanel::GetRowChildren)
		.OnSelectionChanged(this, &SBertaDelegateInspectorPanel::OnRowSelectionChanged)
		.SelectionMode(ESelectionMode::Single)
		.HeaderRow(SNew(SHeaderRow)
			+ SHeaderRow::Column(TEXT("Name")).DefaultLabel(FText::FromString(TEXT("Name"))).FillWidth(0.36f)
			+ SHeaderRow::Column(TEXT("Type")).DefaultLabel(FText::FromString(TEXT("Type / Class"))).FillWidth(0.25f)
			+ SHeaderRow::Column(TEXT("Function")).DefaultLabel(FText::FromString(TEXT("Function / Bindings"))).FillWidth(0.25f)
			+ SHeaderRow::Column(TEXT("Status")).DefaultLabel(FText::FromString(TEXT("Status"))).FillWidth(0.14f));

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 6, 8, 0)
		[ SNew(STextBlock).Text(FText::FromString(TEXT("Who Is Listening?  |  Reflected dynamic multicast delegates on this PIE Actor and its direct components"))) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[ SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[ SNew(SButton).Text(FText::FromString(TEXT("Use Selected Actor"))).OnClicked(this, &SBertaDelegateInspectorPanel::UseSelectedActor) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[ SNew(SButton).Text(FText::FromString(TEXT("Refresh Bindings"))).OnClicked(this, &SBertaDelegateInspectorPanel::RefreshBindings) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[ SNew(SButton).Text(FText::FromString(TEXT("Clear Snapshot")))
				.ToolTipText(FText::FromString(TEXT("Clear Inspector data only; runtime delegates are untouched.")))
				.OnClicked(this, &SBertaDelegateInspectorPanel::ClearSnapshot) ] ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 2)
		[ SNew(STextBlock).Text(this, &SBertaDelegateInspectorPanel::StatusText).AutoWrapText(true) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 2)
		[ SNew(STextBlock).Text(this, &SBertaDelegateInspectorPanel::TargetText) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 2)
		[ SNew(STextBlock).Text(this, &SBertaDelegateInspectorPanel::CountsText) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 2)
		[ SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(2).VAlign(VAlign_Center)
			[ SNew(SCheckBox).IsChecked(ECheckBoxState::Checked)
				.OnCheckStateChanged(this, &SBertaDelegateInspectorPanel::OnBoundOnlyChanged)
				[ SNew(STextBlock).Text(FText::FromString(TEXT("Bound Only"))) ] ]
			+ SHorizontalBox::Slot().FillWidth(1).Padding(6, 0)
			[ SNew(SSearchBox).HintText(FText::FromString(TEXT("Search copied names, classes, delegates, functions")))
				.OnTextChanged(this, &SBertaDelegateInspectorPanel::OnSearchChanged) ] ]
		+ SVerticalBox::Slot().FillHeight(1).Padding(8)
		[ Tree.ToSharedRef() ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[ SNew(STextBlock).Text(this, &SBertaDelegateInspectorPanel::DetailsText).AutoWrapText(true) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[ SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[ SNew(SButton).Text(FText::FromString(TEXT("Select Source Object")))
				.IsEnabled(this, &SBertaDelegateInspectorPanel::CanSelectSourceObject)
				.OnClicked(this, &SBertaDelegateInspectorPanel::SelectSourceObject) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[ SNew(SButton).Text(FText::FromString(TEXT("Select Listener Object")))
				.IsEnabled(this, &SBertaDelegateInspectorPanel::CanSelectListenerObject)
				.OnClicked(this, &SBertaDelegateInspectorPanel::SelectListenerObject) ] ]
	];
}

SBertaDelegateInspectorPanel::~SBertaDelegateInspectorPanel()
{
	FEditorDelegates::BeginPIE.Remove(BeginPIEHandle);
	FEditorDelegates::EndPIE.Remove(EndPIEHandle);
}

FReply SBertaDelegateInspectorPanel::UseSelectedActor()
{
	if (!GEditor || !GEditor->IsPlayingSessionInEditor())
	{
		Status = TEXT("Select exactly one PIE Actor to inspect.");
		return FReply::Handled();
	}
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
	Status = TEXT("PIE Actor captured weakly. Press Refresh Bindings.");
	RebuildRows();
	return FReply::Handled();
}

FReply SBertaDelegateInspectorPanel::RefreshBindings()
{
	AActor* Actor = CapturedActor.Get();
	if (!Actor)
	{
		Status = TEXT("The captured target is no longer valid.");
		return FReply::Handled();
	}
	if (!GEditor || !GEditor->IsPlayingSessionInEditor() || CaptureGeneration != SessionGeneration
		|| !Actor->GetWorld() || Actor->GetWorld()->WorldType != EWorldType::PIE)
	{
		Status = TEXT("The captured target is outside the active PIE session. Capture a new PIE Actor.");
		return FReply::Handled();
	}
	BertaDelegateInspector::FSnapshot NewSnapshot;
	FString Error;
	if (!BertaDelegateInspector::Inspect(Actor, NewSnapshot, Error))
	{
		Status = Error;
		return FReply::Handled();
	}
	Snapshot = MoveTemp(NewSnapshot);
	bHasSnapshot = true;
	InspectionGeneration = SessionGeneration;
	Status = Snapshot.DelegateCount == 0
		? TEXT("No reflected dynamic multicast delegate properties were found on the Actor or its components.")
		: Snapshot.BindingCount == 0
			? TEXT("No current dynamic multicast delegate bindings were found. Disable Bound Only to view unbound delegates.")
			: Snapshot.UnresolvedBindingCount > 0
				? TEXT("Listener found; some bound functions could not be resolved through supported UE APIs. Snapshot is read-only.")
				: TEXT("Dynamic multicast delegate bindings found. Snapshot is read-only; refresh manually for changes.");
	RebuildRows();
	return FReply::Handled();
}

FReply SBertaDelegateInspectorPanel::ClearSnapshot()
{
	CapturedActor.Reset();
	CapturedSummary.Reset();
	Snapshot = {};
	bHasSnapshot = false;
	Status = TEXT("Inspector snapshot cleared; runtime delegates were not changed.");
	RebuildRows();
	return FReply::Handled();
}

void SBertaDelegateInspectorPanel::OnBeginPIE(bool bSimulating)
{
	++SessionGeneration;
	CapturedActor.Reset();
	if (bHasSnapshot)
	{
		Snapshot.bStale = true;
		Status = TEXT("Snapshot from an earlier PIE session; capture a new target. Live navigation disabled.");
		RebuildRows();
	}
	else { CapturedSummary.Reset(); }
}

void SBertaDelegateInspectorPanel::OnEndPIE(bool bSimulating)
{
	++SessionGeneration;
	CapturedActor.Reset();
	if (bHasSnapshot)
	{
		Snapshot.bStale = true;
		Status = TEXT("PIE ended. Detached binding snapshot remains readable; live navigation disabled.");
		RebuildRows();
	}
	else { CapturedSummary.Reset(); }
}

void SBertaDelegateInspectorPanel::OnBoundOnlyChanged(ECheckBoxState NewState)
{
	bBoundOnly = NewState == ECheckBoxState::Checked;
	RebuildRows();
}

void SBertaDelegateInspectorPanel::OnSearchChanged(const FText& NewText)
{
	Search = NewText.ToString();
	RebuildRows();
}

void SBertaDelegateInspectorPanel::RebuildRows()
{
	SelectedRow.Reset();
	RootRows.Reset();
	if (bHasSnapshot)
	{
		for (const BertaDelegateInspector::FVisibleSource& VisibleSource : BertaDelegateInspector::Filter(Snapshot, bBoundOnly, Search))
		{
			const BertaDelegateInspector::FSource& Source = Snapshot.Sources[VisibleSource.SourceIndex];
			TSharedPtr<FRow> SourceRow = MakeShared<FRow>();
			SourceRow->Kind = BertaDelegateInspector::ERowKind::Source;
			SourceRow->SourceIndex = VisibleSource.SourceIndex;
			SourceRow->Name = Source.Name;
			SourceRow->Type = Source.ClassName;
			SourceRow->FunctionOrCount = Source.Kind == BertaDelegateInspector::ESourceKind::Actor ? TEXT("Actor") : TEXT("Component");
			SourceRow->Status = Source.LiveObject.IsValid() && !Snapshot.bStale ? TEXT("Live") : TEXT("Stale / expired");
			for (const BertaDelegateInspector::FVisibleDelegate& VisibleDelegate : VisibleSource.Delegates)
			{
				const BertaDelegateInspector::FDelegate& Delegate = Source.Delegates[VisibleDelegate.DelegateIndex];
				TSharedPtr<FRow> DelegateRow = MakeShared<FRow>();
				DelegateRow->Kind = BertaDelegateInspector::ERowKind::Delegate;
				DelegateRow->SourceIndex = VisibleSource.SourceIndex;
				DelegateRow->DelegateIndex = VisibleDelegate.DelegateIndex;
				DelegateRow->Name = Delegate.Name;
				DelegateRow->Type = Delegate.Kind;
				DelegateRow->FunctionOrCount = FString::Printf(TEXT("%d binding(s)"), Delegate.Bindings.Num());
				for (int32 BindingIndex : VisibleDelegate.BindingIndices)
				{
					const BertaDelegateInspector::FBinding& Binding = Delegate.Bindings[BindingIndex];
					TSharedPtr<FRow> BindingRow = MakeShared<FRow>();
					BindingRow->Kind = BertaDelegateInspector::ERowKind::Binding;
					BindingRow->SourceIndex = VisibleSource.SourceIndex;
					BindingRow->DelegateIndex = VisibleDelegate.DelegateIndex;
					BindingRow->BindingIndex = BindingIndex;
					BindingRow->Name = Binding.Name;
					BindingRow->Type = Binding.ClassName;
					BindingRow->FunctionOrCount = Binding.bFunctionResolved ? Binding.FunctionName.ToString() : TEXT("Unresolved");
					BindingRow->Status = Binding.LiveObject.IsValid() && !Snapshot.bStale ? TEXT("Live") : TEXT("Stale / expired");
					DelegateRow->Children.Add(MoveTemp(BindingRow));
				}
				SourceRow->Children.Add(MoveTemp(DelegateRow));
			}
			RootRows.Add(MoveTemp(SourceRow));
		}
	}
	if (Tree.IsValid())
	{
		Tree->ClearSelection();
		Tree->RequestTreeRefresh();
		for (const TSharedPtr<FRow>& SourceRow : RootRows)
		{
			Tree->SetItemExpansion(SourceRow, true);
			for (const TSharedPtr<FRow>& DelegateRow : SourceRow->Children) { Tree->SetItemExpansion(DelegateRow, true); }
		}
	}
}

TSharedRef<ITableRow> SBertaDelegateInspectorPanel::GenerateRow(TSharedPtr<FRow> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SBertaDelegateRow, OwnerTable).Item(Item);
}

void SBertaDelegateInspectorPanel::GetRowChildren(TSharedPtr<FRow> Item, TArray<TSharedPtr<FRow>>& OutChildren) const
{
	OutChildren = Item->Children;
}

void SBertaDelegateInspectorPanel::OnRowSelectionChanged(TSharedPtr<FRow> Item, ESelectInfo::Type SelectInfo)
{
	SelectedRow = Item; // Diagnostic selection only: no Editor Actor/Object selection side effect.
}

FText SBertaDelegateInspectorPanel::StatusText() const { return FText::FromString(Status); }
FText SBertaDelegateInspectorPanel::TargetText() const
{
	if (bHasSnapshot && Snapshot.bStale)
	{
		return FText::FromString(FString::Printf(TEXT("Last inspected target (stale): %s | %s | %s"),
			*Snapshot.TargetName, *Snapshot.TargetClass, *Snapshot.TargetPath));
	}
	return FText::FromString(CapturedSummary.IsEmpty() ? TEXT("Captured target: none")
		: TEXT("Captured target (weak): ") + CapturedSummary);
}
FText SBertaDelegateInspectorPanel::CountsText() const
{
	if (!bHasSnapshot) { return FText::FromString(TEXT("No inspection yet.")); }
	return FText::FromString(FString::Printf(TEXT("%d delegates | %d bound | %d bindings (%d unresolved) | %.1f ms%s"),
		Snapshot.DelegateCount, Snapshot.BoundDelegateCount, Snapshot.BindingCount, Snapshot.UnresolvedBindingCount, Snapshot.DurationMs,
		Snapshot.bStale ? TEXT(" | STALE") : TEXT("")));
}
FText SBertaDelegateInspectorPanel::DetailsText() const
{
	if (!SelectedRow.IsValid()) { return FText::FromString(TEXT("Select a row for copied diagnostic details.")); }
	const BertaDelegateInspector::FSource& Source = Snapshot.Sources[SelectedRow->SourceIndex];
	if (SelectedRow->Kind == BertaDelegateInspector::ERowKind::Source)
	{
		return FText::FromString(FString::Printf(TEXT("Source: %s | Class: %s | Path: %s | %s | Live: %s"),
			*Source.Name, *Source.ClassName, *Source.Path,
			Source.Kind == BertaDelegateInspector::ESourceKind::Actor ? TEXT("Actor") : TEXT("Component"),
			Source.LiveObject.IsValid() && !Snapshot.bStale ? TEXT("yes") : TEXT("no")));
	}
	const BertaDelegateInspector::FDelegate& Delegate = Source.Delegates[SelectedRow->DelegateIndex];
	if (SelectedRow->Kind == BertaDelegateInspector::ERowKind::Delegate)
	{
		return FText::FromString(FString::Printf(TEXT("Source: %s | Delegate: %s | %s | Signature: %s | Bindings: %d"),
			*Source.Name, *Delegate.Name, *Delegate.Kind, *Delegate.SignatureName, Delegate.Bindings.Num()));
	}
	const BertaDelegateInspector::FBinding& Binding = Delegate.Bindings[SelectedRow->BindingIndex];
	return FText::FromString(FString::Printf(TEXT("Listener: %s | Class: %s | Path: %s | Bound Function: %s | Live: %s"),
		*Binding.Name, *Binding.ClassName, *Binding.Path,
		Binding.bFunctionResolved ? *Binding.FunctionName.ToString() : TEXT("Unresolved through supported UE APIs"),
		Binding.LiveObject.IsValid() && !Snapshot.bStale ? TEXT("yes") : TEXT("no")));
}

bool SBertaDelegateInspectorPanel::CanNavigate(UObject* Object) const
{
	return Object && IsValid(Object) && bHasSnapshot && !Snapshot.bStale
		&& InspectionGeneration == SessionGeneration && GEditor && GEditor->IsPlayingSessionInEditor()
		&& Object->GetWorld() && Object->GetWorld()->WorldType == EWorldType::PIE;
}
bool SBertaDelegateInspectorPanel::CanSelectSourceObject() const
{
	return SelectedRow.IsValid() && CanNavigate(Snapshot.Sources[SelectedRow->SourceIndex].LiveObject.Get());
}
bool SBertaDelegateInspectorPanel::CanSelectListenerObject() const
{
	if (!SelectedRow.IsValid() || SelectedRow->Kind != BertaDelegateInspector::ERowKind::Binding) { return false; }
	return CanNavigate(Snapshot.Sources[SelectedRow->SourceIndex].Delegates[SelectedRow->DelegateIndex]
		.Bindings[SelectedRow->BindingIndex].LiveObject.Get());
}
FReply SBertaDelegateInspectorPanel::SelectObject(UObject* Object)
{
	if (!CanNavigate(Object)) { return FReply::Handled(); }
	if (AActor* Actor = Cast<AActor>(Object))
	{
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
FReply SBertaDelegateInspectorPanel::SelectSourceObject()
{
	return CanSelectSourceObject() ? SelectObject(Snapshot.Sources[SelectedRow->SourceIndex].LiveObject.Get()) : FReply::Handled();
}
FReply SBertaDelegateInspectorPanel::SelectListenerObject()
{
	return CanSelectListenerObject() ? SelectObject(Snapshot.Sources[SelectedRow->SourceIndex].Delegates[SelectedRow->DelegateIndex]
		.Bindings[SelectedRow->BindingIndex].LiveObject.Get()) : FReply::Handled();
}
