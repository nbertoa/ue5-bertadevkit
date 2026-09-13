using UnrealBuildTool;

public class BertaDesktopCapture : ModuleRules
{
	public BertaDesktopCapture(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"ApplicationCore",
			"RHI"
		});

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemLibraries.AddRange(new[]
			{
				"d3d11.lib",
				"dwmapi.lib",
				"runtimeobject.lib"
			});
		}
	}
}
