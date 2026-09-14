using UnrealBuildTool;

public class BertaGASCompanionExtEditor : ModuleRules
{
	public BertaGASCompanionExtEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"BertaGASCompanionExt",
			"Core",
			"CoreUObject",
			"DataValidation",
			"Engine",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GASCompanion",
			"UnrealEd"
		});
	}
}
