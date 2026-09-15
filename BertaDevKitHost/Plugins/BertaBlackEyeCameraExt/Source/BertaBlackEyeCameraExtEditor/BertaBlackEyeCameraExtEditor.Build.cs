using UnrealBuildTool;

public class BertaBlackEyeCameraExtEditor : ModuleRules
{
    public BertaBlackEyeCameraExtEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.Add("Core");
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "ToolMenus",
            "Slate",
            "SlateCore",
            "BertaBlackEyeCameraExt",
            "Black_Eye"
        });
    }
}
