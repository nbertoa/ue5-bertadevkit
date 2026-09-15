#include "TickGraph/BertaTickGraphTab.h"

#include "EdGraphUtilities.h"
#include "Framework/Docking/TabManager.h"
#include "SGraphNodeDefault.h"
#include "Templates/Casts.h"
#include "TickGraph/BertaTickGraphEdGraph.h"
#include "TickGraph/BertaTickGraphPanel.h"
#include "Widgets/Docking/SDockTab.h"

namespace BertaTickGraph
{
	const FName TabId(TEXT("BertaTickGraph"));

	class FNodeFactory final : public FGraphPanelNodeFactory
	{
	public:
		virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* Node) const override
		{
			if (Cast<UBertaTickGraphEdNode>(Node)) { return SNew(SGraphNodeDefault).GraphNodeObj(Node); }
			return nullptr;
		}
	};

	void FTabOwner::Register()
	{
		if (bRegistered) { return; }
		NodeFactory = MakeShared<FNodeFactory>();
		FEdGraphUtilities::RegisterVisualNodeFactory(NodeFactory);
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabId, FOnSpawnTab::CreateRaw(this, &FTabOwner::SpawnTab))
			.SetDisplayName(FText::FromString(TEXT("Berta Tick Graph")))
			.SetTooltipText(FText::FromString(TEXT("Visualize Tick prerequisites for a selected PIE Actor and its components.")));
		bRegistered = true;
	}

	void FTabOwner::Unregister()
	{
		if (!bRegistered) { return; }
		if (TSharedPtr<SDockTab> OpenTab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(TabId))) { OpenTab->RequestCloseTab(); }
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);
		FEdGraphUtilities::UnregisterVisualNodeFactory(NodeFactory);
		NodeFactory.Reset();
		bRegistered = false;
	}

	void FTabOwner::Open() { FGlobalTabmanager::Get()->TryInvokeTab(TabId); }
	TSharedRef<SDockTab> FTabOwner::SpawnTab(const FSpawnTabArgs& Args)
	{
		return SNew(SDockTab).TabRole(ETabRole::NomadTab)[ SNew(SBertaTickGraphPanel) ];
	}
}
