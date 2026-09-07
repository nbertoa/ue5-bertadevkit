using UnrealBuildTool;

public class BertaDualSense : ModuleRules
{
	public BertaDualSense(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"ApplicationCore",
			"InputDevice",
			"InputCore",
			"Engine",
			"CoreUObject"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Projects",
			"SDL3"
		});
	}
}
