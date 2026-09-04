#include "BertaDevKitEditor.h"

#include "ContentBrowser/BertaContentBrowserMenu.h"
#include "Toolbar/BertaEditorToolbar.h"
#include "Log/BertaDevKitEditorLog.h"

#include "MessageLogModule.h"
#include "Misc/CoreDelegates.h"

void FBertaDevKitEditorModule::StartupModule()
{
	// Construct the toolbar object immediately so it is ready when the delegate fires.
	// Registration itself must wait for OnPostEngineInit because UToolMenus
	// is not guaranteed to exist at this point in the startup sequence.
	EditorToolbar = MakeUnique<FBertaEditorToolbar>();
	ContentBrowserMenu = MakeUnique<FBertaContentBrowserMenu>();

	// Bind to OnPostEngineInit using a named member callback — avoids a raw lambda
	// and makes the call traceable in the debugger.
	FCoreDelegates::GetOnPostEngineInit().AddRaw(this,
	                                             &FBertaDevKitEditorModule::OnPostEngineInit);

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT( "[FBertaDevKitEditorModule::StartupModule] Module started. Toolbar registration deferred to OnPostEngineInit." ));
}

void FBertaDevKitEditorModule::ShutdownModule()
{
	// Remove the delegate binding before the toolbar is destroyed.
	// If OnPostEngineInit has not fired yet (edge case on very fast shutdown),
	// this prevents a dangling callback.
	FCoreDelegates::GetOnPostEngineInit().RemoveAll(this);

	if (EditorToolbar)
	{
		EditorToolbar->Unregister();
		EditorToolbar.Reset();
	}

	if (ContentBrowserMenu)
	{
		ContentBrowserMenu->Unregister();
		ContentBrowserMenu.Reset();
	}
	UnregisterBlueprintAuditMessageLog();

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[FBertaDevKitEditorModule::ShutdownModule] Module shut down. Toolbar unregistered."));
}

void FBertaDevKitEditorModule::OnPostEngineInit()
{
	RegisterBlueprintAuditMessageLog();
	// This delegate fires once — no need to unbind after registration.
	// The engine guarantees it is not called again after this point.
	if (EditorToolbar)
	{
		EditorToolbar->Register();
	}

	if (ContentBrowserMenu)
	{
		ContentBrowserMenu->Register();
	}
}

void FBertaDevKitEditorModule::RegisterBlueprintAuditMessageLog()
{
	FMessageLogInitializationOptions Options;
	Options.bShowPages = true;
	Options.MaxPageCount = 10;
	FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog")).RegisterLogListing(TEXT("BertaDevKitBlueprintAudit"), NSLOCTEXT("BertaDevKit", "BlueprintAuditMessageLog", "BertaDevKit Blueprint Audit"), Options);
}

void FBertaDevKitEditorModule::UnregisterBlueprintAuditMessageLog()
{
	if (FModuleManager::Get().IsModuleLoaded(TEXT("MessageLog")))
	{
		FModuleManager::GetModuleChecked<FMessageLogModule>(TEXT("MessageLog")).UnregisterLogListing(TEXT("BertaDevKitBlueprintAudit"));
	}
}

IMPLEMENT_MODULE(FBertaDevKitEditorModule,
                 BertaDevKitEditor)
