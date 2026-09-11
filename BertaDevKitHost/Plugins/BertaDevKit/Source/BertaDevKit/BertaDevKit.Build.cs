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
			"GameplayTags", // FGameplayTag is part of the public API.
			"UMG" // UUserWidget is part of the public Blueprint API.
		});

		// Private dependencies are only visible inside BertaDevKit.
		// Consumers of the plugin cannot see these — good for
		// implementation details that shouldn't leak into the public API.
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"GameplayAbilities" // UAbilitySystemComponent is an implementation detail.
			// Examples:
			// "EnhancedInput"    — when input helpers are added
		});
	}
}
