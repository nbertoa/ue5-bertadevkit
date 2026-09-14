using UnrealBuildTool;

public class BertaDevKit : ModuleRules
{
	public BertaDevKit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public dependencies are visible to both this module
		// and any module that depends on BertaDevKit.
		// Keep this list minimal — every entry here becomes
		// a transitive dependency for all consumers.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"AIModule", // UBTDecorator is part of the public API.
			"Core",
			"CoreUObject",
			"Engine",
			"DeveloperSettings", // UDeveloperSettings
			"GameplayAbilities", // UGameplayAbility is part of the public AI API.
			"GameplayTags", // FGameplayTag is part of the public API.
			"MediaAssets", // UMediaSource is part of the public video widget API.
			"UMG" // UUserWidget is part of the public Blueprint API.
		});

		// Private dependencies are only visible inside BertaDevKit.
		// Consumers of the plugin cannot see these — good for
		// implementation details that shouldn't leak into the public API.
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AudioMixer" // UMediaSoundComponent implementation uses USynthComponent.
		});
	}
}
