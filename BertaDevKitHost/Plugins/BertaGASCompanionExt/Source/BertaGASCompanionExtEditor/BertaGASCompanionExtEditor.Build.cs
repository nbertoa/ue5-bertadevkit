using UnrealBuildTool;

public class BertaGASCompanionExtEditor : ModuleRules
{
	public BertaGASCompanionExtEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"BertaGASCompanionExt",
			"DataValidation",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GASCompanion",
			"GameFeatures",
			"UnrealEd"
		});
	}
}
