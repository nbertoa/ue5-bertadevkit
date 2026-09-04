using UnrealBuildTool;

public class BertaDevKitEditor : ModuleRules
{
	public BertaDevKitEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AssetRegistry", // FAssetData is part of the public API.
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd", // UEditorUtilityLibrary, UEditorAssetLibrary
			"Blutility", // UEditorUtilityLibrary for the existing Tools menu scope fallback
			"AssetTools", // FAssetToolsModule, FAssetRenameData
			"BertaDevKit", // UBertaDevKitSettings
			"Niagara", // UNiagaraSystem, UNiagaraEmitter
			"UMG", // UUserWidget
			"UMGEditor", // UWidgetBlueprint
			"AIModule", // UBehaviorTree, UBlackboardData, UEnvQuery
			"Slate", // FSlateNotificationManager
			"SlateCore", // FNotificationInfo, FNotificationEntry
			"ToolMenus", // UToolMenus, FToolMenuEntry
			"ContentBrowser", // UContentBrowserAssetContextMenuContext, UContentBrowserFolderContext
			"EnhancedInput",
			"DataValidation", // UEditorValidatorBase
			"BlueprintGraph", // UBlueprintEditorSettings
			"Kismet", // UBlueprint and Blueprint graph inspection
			"Projects", // IProjectManager and IPluginManager
		});
	}
}
