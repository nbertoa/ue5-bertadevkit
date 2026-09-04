#include "ProjectSetup/BertaProjectSetup.h"

#include "Log/BertaDevKitEditorLog.h"

#include "BlueprintEditorSettings.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "Editor/EditorPerformanceSettings.h"
#include "Engine/RendererSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/IProjectManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Settings/EditorExperimentalSettings.h"
#include "Settings/EditorLoadingSavingSettings.h"
#include "Settings/EditorProjectSettings.h"
#include "Settings/EditorStyleSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "UObject/UnrealType.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace
{
	constexpr TCHAR ScriptStackSection[] = TEXT("Kismet");
	constexpr TCHAR ScriptStackKey[] = TEXT("ScriptStackOnWarnings");
	const TCHAR* const OptionalPluginIds[] = { TEXT("BlueprintAssist"), TEXT("ElectronicNodes"), TEXT("PCGExtendedToolkit"), TEXT("GASCompanion"), TEXT("ComboGraph") };

	FString BoolText(bool bValue) { return bValue ? TEXT("True") : TEXT("False"); }
	const TCHAR* StatusText(EBertaProjectSetupStatus Status)
	{
		switch (Status) { case EBertaProjectSetupStatus::Match: return TEXT("Match"); case EBertaProjectSetupStatus::NeedsChange: return TEXT("Needs Change"); case EBertaProjectSetupStatus::PluginMissing: return TEXT("Plugin Missing"); case EBertaProjectSetupStatus::PendingRestart: return TEXT("Pending Restart"); case EBertaProjectSetupStatus::Unavailable: return TEXT("Unavailable"); case EBertaProjectSetupStatus::Failed: return TEXT("Failed"); default: return TEXT("Unknown"); }
	}
	void AddValue(FBertaProjectSetupReport& Report, const TCHAR* Name, const FString& Current, const FString& Expected, bool bRestart = false, const FString& Detail = FString())
	{
		Report.Items.Add({ Name, Current, Expected, Detail, Current == Expected ? EBertaProjectSetupStatus::Match : EBertaProjectSetupStatus::NeedsChange, bRestart });
	}
	UObject* FindSettingsClassDefault(const TCHAR* ClassPath)
	{
		if (UClass* SettingsClass = FindObject<UClass>(nullptr, ClassPath)) return GetMutableDefault<UObject>(SettingsClass);
		return nullptr;
	}
	bool ReadBoolProperty(UObject* Object, const FName Name, bool& OutValue)
	{
		if (const FBoolProperty* Property = FindFProperty<FBoolProperty>(Object->GetClass(), Name)) { OutValue = Property->GetPropertyValue_InContainer(Object); return true; }
		return false;
	}
	bool WriteBoolProperty(UObject* Object, const FName Name, bool Value)
	{
		if (FBoolProperty* Property = FindFProperty<FBoolProperty>(Object->GetClass(), Name)) { Property->SetPropertyValue_InContainer(Object, Value); Object->PostEditChange(); Object->SaveConfig(); return true; }
		return false;
	}
	void Notify(const FText& Text)
	{
		FNotificationInfo Info(Text); Info.ExpireDuration = 5.0f; FSlateNotificationManager::Get().AddNotification(Info);
	}
	void LogReport(const FBertaProjectSetupReport& Report, const TCHAR* Prefix)
	{
		for (const FBertaProjectSetupItem& Item : Report.Items) UE_LOG(LogBertaDevKitEditor, Log, TEXT("[%s] %s | Current: %s | Expected: %s%s%s"), StatusText(Item.Status), *Item.Name, *Item.CurrentValue, *Item.ExpectedValue, Item.Detail.IsEmpty() ? TEXT("") : TEXT(" | "), *Item.Detail);
		UE_LOG(LogBertaDevKitEditor, Log, TEXT("[%s] Match=%d NeedsChange=%d Missing=%d PendingRestart=%d Unavailable=%d Failed=%d"), Prefix, Report.Count(EBertaProjectSetupStatus::Match), Report.Count(EBertaProjectSetupStatus::NeedsChange), Report.Count(EBertaProjectSetupStatus::PluginMissing), Report.Count(EBertaProjectSetupStatus::PendingRestart), Report.Count(EBertaProjectSetupStatus::Unavailable), Report.Count(EBertaProjectSetupStatus::Failed));
	}
	int32 CountPluginsToEnable(const FBertaProjectSetupReport& Report)
	{
		int32 Result = 0;
		for (const FBertaProjectSetupItem& Item : Report.Items)
		{
			for (const TCHAR* PluginId : OptionalPluginIds)
			{
				if (Item.Status == EBertaProjectSetupStatus::NeedsChange && Item.Name == PluginId)
				{
					++Result;
					break;
				}
			}
		}
		return Result;
	}
	void AddBlueprintAssistAudit(FBertaProjectSetupReport& Report)
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("BlueprintAssist"));
		if (!Plugin.IsValid()) { Report.Items.Add({ TEXT("Blueprint Assist"), TEXT("Not installed"), TEXT("Installed"), TEXT("Optional plugin is not installed."), EBertaProjectSetupStatus::PluginMissing }); return; }
		if (!Plugin->IsEnabled()) { Report.Items.Add({ TEXT("Blueprint Assist settings"), TEXT("Plugin disabled"), TEXT("Available after restart"), TEXT("Apply enables the optional plugin without loading it."), EBertaProjectSetupStatus::PendingRestart, true }); return; }
		UObject* Settings = FindSettingsClassDefault(TEXT("/Script/BlueprintAssist.BASettings"));
		if (!Settings) { Report.Items.Add({ TEXT("Blueprint Assist settings"), TEXT("Class unavailable"), TEXT("Available"), TEXT("Plugin is enabled but its settings class is unavailable."), EBertaProjectSetupStatus::Unavailable }); return; }
		const FEnumProperty* Parameter = FindFProperty<FEnumProperty>(Settings->GetClass(), TEXT("ParameterStyle"));
		const FBoolProperty* CommentPadding = FindFProperty<FBoolProperty>(Settings->GetClass(), TEXT("bApplyCommentPadding"));
		const FStructProperty* Formatter = FindFProperty<FStructProperty>(Settings->GetClass(), TEXT("BlueprintFormatterSettings"));
		const FEnumProperty* AutoFormatting = Formatter ? FindFProperty<FEnumProperty>(Formatter->Struct, TEXT("AutoFormatting")) : nullptr;
		if (!Parameter || !CommentPadding || !AutoFormatting) { Report.Items.Add({ TEXT("Blueprint Assist settings"), TEXT("Properties unavailable"), TEXT("Available"), TEXT("Blueprint Assist reflected properties were not available."), EBertaProjectSetupStatus::Unavailable }); return; }
		const int64 CurrentParameter = Parameter->GetUnderlyingProperty()->GetSignedIntPropertyValue(Parameter->ContainerPtrToValuePtr<void>(Settings));
		AddValue(Report, TEXT("Blueprint Assist Parameter Style"), Parameter->GetEnum()->GetNameStringByValue(CurrentParameter), TEXT("LeftSide"));
		AddValue(Report, TEXT("Blueprint Assist Apply Comment Padding"), BoolText(CommentPadding->GetPropertyValue_InContainer(Settings)), TEXT("False"));
		void* FormatterValue = Formatter->ContainerPtrToValuePtr<void>(Settings);
		const int64 CurrentAuto = AutoFormatting->GetUnderlyingProperty()->GetSignedIntPropertyValue(AutoFormatting->ContainerPtrToValuePtr<void>(FormatterValue));
		AddValue(Report, TEXT("Blueprint Assist Auto Formatting"), AutoFormatting->GetEnum()->GetNameStringByValue(CurrentAuto), TEXT("Never"));
	}
	bool ApplyBlueprintAssist(UObject* Settings)
	{
		FEnumProperty* Parameter = FindFProperty<FEnumProperty>(Settings->GetClass(), TEXT("ParameterStyle")); FBoolProperty* CommentPadding = FindFProperty<FBoolProperty>(Settings->GetClass(), TEXT("bApplyCommentPadding")); FStructProperty* Formatter = FindFProperty<FStructProperty>(Settings->GetClass(), TEXT("BlueprintFormatterSettings")); FEnumProperty* AutoFormatting = Formatter ? FindFProperty<FEnumProperty>(Formatter->Struct, TEXT("AutoFormatting")) : nullptr;
		if (!Parameter || !CommentPadding || !AutoFormatting) return false;
		Parameter->GetUnderlyingProperty()->SetIntPropertyValue(Parameter->ContainerPtrToValuePtr<void>(Settings), Parameter->GetEnum()->GetValueByNameString(TEXT("LeftSide"))); CommentPadding->SetPropertyValue_InContainer(Settings, false);
		void* FormatterValue = Formatter->ContainerPtrToValuePtr<void>(Settings); AutoFormatting->GetUnderlyingProperty()->SetIntPropertyValue(AutoFormatting->ContainerPtrToValuePtr<void>(FormatterValue), AutoFormatting->GetEnum()->GetValueByNameString(TEXT("Never")));
		Settings->PostEditChange(); Settings->SaveConfig(); return true;
	}
}

int32 FBertaProjectSetupReport::Count(EBertaProjectSetupStatus Status) const
{
	int32 Result = 0;
	for (const FBertaProjectSetupItem& Item : Items)
	{
		Result += Item.Status == Status ? 1 : 0;
	}
	return Result;
}
bool FBertaProjectSetupReport::RequiresRestart() const { return Items.ContainsByPredicate([](const FBertaProjectSetupItem& Item) { return Item.bRequiresRestart; }); }

FBertaProjectSetupReport FBertaProjectSetup::Audit()
{
	FBertaProjectSetupReport Report;
	const ULevelEditorPlaySettings* Play = GetDefault<ULevelEditorPlaySettings>(); const UEditorStyleSettings* Style = GetDefault<UEditorStyleSettings>(); const UEditorLoadingSavingSettings* Loading = GetDefault<UEditorLoadingSavingSettings>(); const UEditorPerformanceSettings* Performance = GetDefault<UEditorPerformanceSettings>(); const UBlueprintEditorSettings* Blueprint = GetDefault<UBlueprintEditorSettings>(); const UEditorExperimentalSettings* Experimental = GetDefault<UEditorExperimentalSettings>(); const UEditorPerProjectUserSettings* PerProject = GetDefault<UEditorPerProjectUserSettings>(); const UEditorProjectAppearanceSettings* Appearance = GetDefault<UEditorProjectAppearanceSettings>(); const URendererSettings* Renderer = GetDefault<URendererSettings>();
	AddValue(Report, TEXT("Game Gets Mouse Control"), BoolText(Play->GameGetsMouseControl), TEXT("True")); AddValue(Report, TEXT("Asset Editor Open Location"), StaticEnum<EAssetEditorOpenLocation>()->GetNameStringByValue((int64)Style->AssetEditorOpenLocation), TEXT("MainWindow")); AddValue(Report, TEXT("Load Level at Startup"), StaticEnum<ELoadLevelAtStartup::Type>()->GetNameStringByValue(Loading->LoadLevelAtStartup), TEXT("LastOpened"), true); AddValue(Report, TEXT("Restore Open Asset Tabs"), StaticEnum<ERestoreOpenAssetTabsMethod>()->GetNameStringByValue((int64)Loading->RestoreOpenAssetTabsOnRestart), TEXT("AlwaysRestore"), true); AddValue(Report, TEXT("Use Less CPU when in Background"), BoolText(Performance->bThrottleCPUWhenNotForeground), TEXT("False")); AddValue(Report, TEXT("Save On Compile"), StaticEnum<ESaveOnCompile>()->GetNameStringByValue(Blueprint->SaveOnCompile), TEXT("SoC_SuccessOnly")); AddValue(Report, TEXT("Blueprint Break on Exceptions"), BoolText(Experimental->bBreakOnExceptions), TEXT("True")); AddValue(Report, TEXT("Jump to Node Errors"), BoolText(Blueprint->bJumpToNodeErrors), TEXT("True")); AddValue(Report, TEXT("Promote Output Log Warnings During PIE"), BoolText(Play->bPromoteOutputLogWarningsDuringPIE), TEXT("True")); AddValue(Report, TEXT("Animation Reimport Warnings"), BoolText(PerProject->bAnimationReimportWarnings), TEXT("True")); AddValue(Report, TEXT("Show Hidden Properties While Playing"), BoolText(Style->bShowHiddenPropertiesWhilePlaying), TEXT("True")); AddValue(Report, TEXT("Display Units on Component Transforms"), BoolText(Appearance->bDisplayUnitsOnComponentTransforms), TEXT("True")); AddValue(Report, TEXT("Auto Exposure"), BoolText(Renderer->bDefaultFeatureAutoExposure), TEXT("False"));
	bool bScriptStack = false; const FString DefaultEngine = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("DefaultEngine.ini")); GConfig->GetBool(ScriptStackSection, ScriptStackKey, bScriptStack, DefaultEngine); AddValue(Report, TEXT("Script Stack on Warnings"), BoolText(bScriptStack), TEXT("True"), true);
	if (UObject* LiveCoding = FindSettingsClassDefault(TEXT("/Script/LiveCoding.LiveCodingSettings"))) { bool bAutoCompile = true; if (ReadBoolProperty(LiveCoding, TEXT("bAutomaticallyCompileNewClasses"), bAutoCompile)) AddValue(Report, TEXT("Automatically Compile Newly Added C++ Classes"), BoolText(bAutoCompile), TEXT("False")); else Report.Items.Add({ TEXT("Automatically Compile Newly Added C++ Classes"), TEXT("Property unavailable"), TEXT("False"), TEXT("Live Coding settings did not expose the expected property."), EBertaProjectSetupStatus::Unavailable }); } else Report.Items.Add({ TEXT("Automatically Compile Newly Added C++ Classes"), TEXT("Class unavailable"), TEXT("False"), TEXT("Live Coding settings class is not currently available."), EBertaProjectSetupStatus::Unavailable });
	for (const TCHAR* PluginId : OptionalPluginIds) { const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginId); if (!Plugin.IsValid()) Report.Items.Add({ PluginId, TEXT("Not installed"), TEXT("Enabled"), TEXT("Optional plugin is not installed."), EBertaProjectSetupStatus::PluginMissing }); else if (!Plugin->IsEnabled()) Report.Items.Add({ PluginId, TEXT("Disabled"), TEXT("Enabled"), TEXT("Apply persists enablement to the project descriptor."), EBertaProjectSetupStatus::NeedsChange, true }); else Report.Items.Add({ PluginId, TEXT("Enabled"), TEXT("Enabled"), FString(), EBertaProjectSetupStatus::Match }); }
	AddBlueprintAssistAudit(Report); return Report;
}
void FBertaProjectSetup::AuditAndReport() { const FBertaProjectSetupReport Report = Audit(); LogReport(Report, TEXT("Project Setup audit")); Notify(FText::Format(NSLOCTEXT("BertaDevKit", "ProjectSetupAudit", "Project Setup: {0} changes needed, {1} optional plugins missing."), Report.Count(EBertaProjectSetupStatus::NeedsChange), Report.Count(EBertaProjectSetupStatus::PluginMissing))); }
void FBertaProjectSetup::ApplyWithConfirmation()
{
	const FBertaProjectSetupReport Before = Audit(); LogReport(Before, TEXT("Project Setup apply preview")); const int32 Changes = Before.Count(EBertaProjectSetupStatus::NeedsChange); if (Changes == 0) { Notify(NSLOCTEXT("BertaDevKit", "ProjectSetupUpToDate", "Project defaults are already up to date.")); return; }
	const FText Preview = FText::Format(NSLOCTEXT("BertaDevKit", "ProjectSetupConfirm", "Apply {0} Project Setup changes?\n\nOptional plugins to enable: {1}\nMissing optional plugins: {2}\nEditor restart may be required."), Changes, CountPluginsToEnable(Before), Before.Count(EBertaProjectSetupStatus::PluginMissing)); if (FMessageDialog::Open(EAppMsgType::YesNo, Preview, NSLOCTEXT("BertaDevKit", "ProjectSetupTitle", "Apply Project Defaults")) != EAppReturnType::Yes) return;
	bool bPersistenceFailure = false;
	ULevelEditorPlaySettings* Play = GetMutableDefault<ULevelEditorPlaySettings>(); Play->GameGetsMouseControl = true; Play->bPromoteOutputLogWarningsDuringPIE = true; Play->PostEditChange(); Play->SaveConfig(); UEditorStyleSettings* Style = GetMutableDefault<UEditorStyleSettings>(); Style->AssetEditorOpenLocation = EAssetEditorOpenLocation::MainWindow; Style->bShowHiddenPropertiesWhilePlaying = true; Style->PostEditChange(); Style->SaveConfig(); UEditorLoadingSavingSettings* Loading = GetMutableDefault<UEditorLoadingSavingSettings>(); Loading->LoadLevelAtStartup = ELoadLevelAtStartup::LastOpened; Loading->RestoreOpenAssetTabsOnRestart = ERestoreOpenAssetTabsMethod::AlwaysRestore; Loading->PostEditChange(); Loading->SaveConfig(); UEditorPerformanceSettings* Performance = GetMutableDefault<UEditorPerformanceSettings>(); Performance->bThrottleCPUWhenNotForeground = false; Performance->PostEditChange(); Performance->SaveConfig(); UBlueprintEditorSettings* Blueprint = GetMutableDefault<UBlueprintEditorSettings>(); Blueprint->SaveOnCompile = SoC_SuccessOnly; Blueprint->bJumpToNodeErrors = true; Blueprint->PostEditChange(); Blueprint->SaveConfig(); UEditorExperimentalSettings* Experimental = GetMutableDefault<UEditorExperimentalSettings>(); Experimental->bBreakOnExceptions = true; Experimental->PostEditChange(); Experimental->SaveConfig(); UEditorPerProjectUserSettings* PerProject = GetMutableDefault<UEditorPerProjectUserSettings>(); PerProject->bAnimationReimportWarnings = true; PerProject->PostEditChange(); PerProject->SaveConfig(); UEditorProjectAppearanceSettings* Appearance = GetMutableDefault<UEditorProjectAppearanceSettings>(); Appearance->bDisplayUnitsOnComponentTransforms = true; Appearance->PostEditChange(); if (!Appearance->TryUpdateDefaultConfigFile()) { UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[Project Setup] Failed to persist Display Units on Component Transforms.")); bPersistenceFailure = true; } URendererSettings* Renderer = GetMutableDefault<URendererSettings>(); Renderer->bDefaultFeatureAutoExposure = false; Renderer->PostEditChange(); if (!Renderer->TryUpdateDefaultConfigFile()) { UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[Project Setup] Failed to persist Auto Exposure.")); bPersistenceFailure = true; }
	const FString DefaultEngine = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("DefaultEngine.ini")); GConfig->SetBool(ScriptStackSection, ScriptStackKey, true, DefaultEngine); GConfig->Flush(false, DefaultEngine); if (UObject* LiveCoding = FindSettingsClassDefault(TEXT("/Script/LiveCoding.LiveCodingSettings"))) WriteBoolProperty(LiveCoding, TEXT("bAutomaticallyCompileNewClasses"), false);
	for (const TCHAR* PluginId : OptionalPluginIds) { const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginId); if (Plugin.IsValid() && !Plugin->IsEnabled()) { FText Failure; if (!IProjectManager::Get().SetPluginEnabled(PluginId, true, Failure)) { UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[Project Setup] Failed to enable %s: %s"), PluginId, *Failure.ToString()); bPersistenceFailure = true; } } }
	if (TSharedPtr<IPlugin> BlueprintAssist = IPluginManager::Get().FindPlugin(TEXT("BlueprintAssist")); BlueprintAssist.IsValid() && BlueprintAssist->IsEnabled()) if (UObject* Settings = FindSettingsClassDefault(TEXT("/Script/BlueprintAssist.BASettings"))) if (!ApplyBlueprintAssist(Settings)) UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[Project Setup] Blueprint Assist settings were unavailable."));
	const FBertaProjectSetupReport After = Audit(); LogReport(After, TEXT("Project Setup apply result")); Notify(FText::Format(NSLOCTEXT("BertaDevKit", "ProjectSetupApplied", "Project Setup applied. {0} changes remain. {1}{2}"), After.Count(EBertaProjectSetupStatus::NeedsChange), After.RequiresRestart() ? NSLOCTEXT("BertaDevKit", "RestartRequired", "Editor restart required. ") : FText::GetEmpty(), bPersistenceFailure ? NSLOCTEXT("BertaDevKit", "PersistenceFailure", "Some changes could not be persisted; see Output Log.") : FText::GetEmpty()));
}
