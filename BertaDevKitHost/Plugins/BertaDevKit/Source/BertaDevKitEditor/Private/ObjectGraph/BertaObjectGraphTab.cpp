#include "ObjectGraph/BertaObjectGraphTab.h"

#include "EdGraphUtilities.h"
#include "Framework/Docking/TabManager.h"
#include "ObjectGraph/BertaObjectGraphEdGraph.h"
#include "ObjectGraph/BertaObjectGraphPanel.h"
#include "SGraphNodeDefault.h"
#include "Templates/Casts.h"
#include "Widgets/Docking/SDockTab.h"

namespace BertaObjectGraph
{
	const FName TabId(TEXT("BertaObjectGraph"));

	class FNodeFactory final : public FGraphPanelNodeFactory
	{
	public:
		virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* Node) const override
		{
			if (Cast<UBertaObjectGraphEdNode>(Node))
			{
				return SNew(SGraphNodeDefault).GraphNodeObj(Node);
			}
			return nullptr;
		}
	};

	void FTabOwner::Register()
	{
		if (bRegistered) { return; }
		NodeFactory = MakeShared<FNodeFactory>();
		FEdGraphUtilities::RegisterVisualNodeFactory(NodeFactory);
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabId, FOnSpawnTab::CreateRaw(this, &FTabOwner::SpawnTab))
			.SetDisplayName(FText::FromString(TEXT("Berta Object Graph")))
			.SetTooltipText(FText::FromString(TEXT("Inspect a detached PIE Actor GC root reference graph.")));
		bRegistered = true;
	}

	void FTabOwner::Unregister()
	{
		if (!bRegistered) { return; }
		if (TSharedPtr<SDockTab> OpenTab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(TabId)))
		{
			OpenTab->RequestCloseTab();
		}
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);
		FEdGraphUtilities::UnregisterVisualNodeFactory(NodeFactory);
		NodeFactory.Reset();
		bRegistered = false;
	}

	void FTabOwner::Open()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(TabId);
	}

	TSharedRef<SDockTab> FTabOwner::SpawnTab(const FSpawnTabArgs& Args)
	{
		return SNew(SDockTab).TabRole(ETabRole::NomadTab)
			[ SNew(SBertaObjectGraphPanel) ];
	}
}
