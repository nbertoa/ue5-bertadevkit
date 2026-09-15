#pragma once

#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"

struct FGraphPanelNodeFactory;
class SDockTab;
class FSpawnTabArgs;

namespace BertaTickGraph
{
	extern const FName TabId;
	class FTabOwner
	{
	public:
		void Register();
		void Unregister();
		static void Open();
	private:
		TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
		TSharedPtr<FGraphPanelNodeFactory> NodeFactory;
		bool bRegistered = false;
	};
}
