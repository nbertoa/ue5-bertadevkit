using UnrealBuildTool;

public class BertaBlackEyeCameraExt : ModuleRules
{
    public BertaBlackEyeCameraExt(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Black_Eye"
        });
    }
}
