#include "DelegateInspector/BertaDelegateInspectorTab.h"

#include "DelegateInspector/BertaDelegateInspectorPanel.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

namespace BertaDelegateInspector
{
	const FName TabId(TEXT("BertaDelegateInspector"));

	void FTabOwner::Register()
	{
		if (bRegistered) { return; }
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabId, FOnSpawnTab::CreateRaw(this, &FTabOwner::SpawnTab))
			.SetDisplayName(FText::FromString(TEXT("Berta Delegate Inspector")))
			.SetTooltipText(FText::FromString(TEXT("Inspect live reflected dynamic multicast delegate bindings on a selected PIE Actor and its components.")));
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
		bRegistered = false;
	}

	void FTabOwner::Open()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(TabId);
	}

	TSharedRef<SDockTab> FTabOwner::SpawnTab(const FSpawnTabArgs& Args)
	{
		return SNew(SDockTab).TabRole(ETabRole::NomadTab)
			[ SNew(SBertaDelegateInspectorPanel) ];
	}
}
