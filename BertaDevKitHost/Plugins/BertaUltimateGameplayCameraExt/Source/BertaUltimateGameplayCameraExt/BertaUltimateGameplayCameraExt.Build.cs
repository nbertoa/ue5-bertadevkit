using UnrealBuildTool;

public class BertaUltimateGameplayCameraExt : ModuleRules
{
	public BertaUltimateGameplayCameraExt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AuroraDevs_UGC"
		});
	}
}
