using System.IO;
using UnrealBuildTool;

public class SDL3 : ModuleRules
{
	public SDL3(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform != UnrealTargetPlatform.Win64)
		{
			throw new BuildException("BertaDualSense supports Win64 only.");
		}

		PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
		PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Win64", "SDL3.lib"));
		PublicDelayLoadDLLs.Add("SDL3.dll");
		RuntimeDependencies.Add(
			"$(TargetOutputDir)/SDL3.dll",
			Path.Combine(ModuleDirectory, "bin", "Win64", "SDL3.dll"),
			StagedFileType.NonUFS);
	}
}