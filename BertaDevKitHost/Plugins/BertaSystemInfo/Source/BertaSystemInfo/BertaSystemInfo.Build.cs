using UnrealBuildTool;

public class BertaSystemInfo : ModuleRules
{
	public BertaSystemInfo(ReadOnlyTargetRules Target) : base(Target)
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
			"ApplicationCore",
			"AudioCaptureCore",
			"AudioMixer",
			"AudioMixerCore",
			"RHI"
		});
	}
}
