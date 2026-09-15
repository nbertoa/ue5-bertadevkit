#pragma once

#include "UObject/NameTypes.h"
#include "Templates/SharedPointer.h"

class SDockTab;
class FSpawnTabArgs;

namespace BertaDelegateInspector
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
		bool bRegistered = false;
	};
}
