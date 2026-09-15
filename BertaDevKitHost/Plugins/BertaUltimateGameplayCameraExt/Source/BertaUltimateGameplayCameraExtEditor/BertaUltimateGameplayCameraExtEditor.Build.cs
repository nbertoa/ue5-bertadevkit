using UnrealBuildTool;

public class BertaUltimateGameplayCameraExtEditor : ModuleRules
{
	public BertaUltimateGameplayCameraExtEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "EngineSettings", "AssetRegistry", "ContentBrowser",
			"BertaUltimateGameplayCameraExt", "AuroraDevs_UGC", "ToolMenus", "Slate", "SlateCore",
			"BlueprintGraph"
		});
	}
}
