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
			"Blutility", // UAssetActionUtility
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd", // UEditorUtilityLibrary, UEditorAssetLibrary
			"AssetTools", // FAssetToolsModule, FAssetRenameData
			"BertaDevKit", // UBertaDevKitSettings
			"Niagara", // UNiagaraSystem, UNiagaraEmitter
			"UMG", // UUserWidget
			"AIModule", // UBehaviorTree, UBlackboardData, UEnvQuery
			"Slate", // FSlateNotificationManager
			"SlateCore", // FNotificationInfo, FNotificationEntry
			"ToolMenus", // UToolMenus, FToolMenuEntry
			"EnhancedInput",
			"DataValidation", // UEditorValidatorBase
		});
	}
}
