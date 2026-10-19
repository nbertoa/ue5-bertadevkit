#include "BertaGASCompanionExt.h"

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogBertaGASCompanionExt);

void FBertaGASCompanionExtModule::StartupModule()
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("BertaGASCompanionExt"));
	checkf(Plugin.IsValid(), TEXT("The loaded BertaGASCompanionExt module must have a discoverable plugin descriptor."));

	// Ordinary code plugins do not automatically import their Config/Tags directory.
	// Default startup precedes PostEngineInit, when the tag manager finalizes native tags.
	UGameplayTagsManager::Get().AddTagIniSearchPath(FPaths::Combine(Plugin->GetBaseDir(), TEXT("Config/Tags")));
}

void FBertaGASCompanionExtModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FBertaGASCompanionExtModule, BertaGASCompanionExt)
