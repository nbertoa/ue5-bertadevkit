using UnrealBuildTool;

public class BertaGASCompanionExt : ModuleRules
{
	public BertaGASCompanionExt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"BertaDevKit",
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GASCompanion",
			"TargetingSystem"
		});
	}
}
