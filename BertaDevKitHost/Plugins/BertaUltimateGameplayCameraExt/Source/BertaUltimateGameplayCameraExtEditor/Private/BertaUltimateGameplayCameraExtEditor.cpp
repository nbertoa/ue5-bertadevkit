#include "Audit/BertaUGCAudit.h"

#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "ToolMenuEntry.h"
#include "ToolMenuSection.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaUGCExtEditor, Log, All);

#define LOCTEXT_NAMESPACE "BertaUltimateGameplayCameraExtEditor"

class FBertaUltimateGameplayCameraExtEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FCoreDelegates::GetOnPostEngineInit().AddRaw(this, &FBertaUltimateGameplayCameraExtEditorModule::RegisterMenus);
	}

	virtual void ShutdownModule() override
	{
		FCoreDelegates::GetOnPostEngineInit().RemoveAll(this);
		if (UToolMenus::IsToolMenuUIEnabled())
		{
			UToolMenus::Get()->UnregisterOwner(MenuOwner);
		}
	}

private:
	void RegisterMenus()
	{
		if (!UToolMenus::IsToolMenuUIEnabled())
		{
			UE_LOG(LogBertaUGCExtEditor, Warning, TEXT("UGC Editor tools menu is unavailable after engine initialization."));
			return;
		}
		UToolMenu* Tools = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
		if (!Tools)
		{
			UE_LOG(LogBertaUGCExtEditor, Warning, TEXT("UGC Editor tools could not extend the Tools menu."));
			return;
		}
		FToolMenuSection& Section = Tools->FindOrAddSection(TEXT("BertaUltimateGameplayCameraExt"));
		FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
			TEXT("BertaAuditUltimateGameplayCamera"), LOCTEXT("AuditUGC", "Audit Ultimate Gameplay Camera"),
			LOCTEXT("AuditUGCTooltip", "Read-only audit of UGC setup, presets and explicit Blueprint camera writes. See Output Log."),
			FSlateIcon(), FUIAction(FExecuteAction::CreateStatic(&BertaUGCAudit::Run)));
		Entry.Owner = FToolMenuOwner(MenuOwner);
		Section.AddEntry(Entry);
	}

	const FName MenuOwner = TEXT("BertaUltimateGameplayCameraExtEditor");
};

IMPLEMENT_MODULE(FBertaUltimateGameplayCameraExtEditorModule, BertaUltimateGameplayCameraExtEditor)

#undef LOCTEXT_NAMESPACE
