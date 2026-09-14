using UnrealBuildTool;

public class BertaComboGraphExtEditor : ModuleRules
{
	public BertaComboGraphExtEditor(ReadOnlyTargetRules Target) : base(Target)
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
			"BertaComboGraphExt",
			"ComboGraph",
			"DataValidation",
			"GameplayAbilities",
			"GameplayTags",
			"Niagara",
			"UnrealEd"
		});
	}
}
