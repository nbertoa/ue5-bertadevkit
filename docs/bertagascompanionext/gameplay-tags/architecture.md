# Architecture and registration

## Ownership and dependency boundaries

The catalog lives inside the existing optional **BertaGASCompanionExt** plugin. Its Runtime and Editor modules retain their existing `Default` loading phases. The catalog is conceptually independent of GAS Companion: registration uses only `GameplayTags`, `Projects`, and native path APIs. The surrounding extension still has its existing GAS Companion and BertaDevKit dependencies, so enabling it requires those plugins.

This is an intentional initial home for projects already using GAS Companion. An independent plugin extraction could be considered when a concrete cross-project requirement justifies it. There is no additional plugin, module, manager, subsystem, settings object, tag container service, or Blueprint library for the catalog today. The generic `UBertaGameplayTagUsageFinder` stays in BertaDevKitEditor and remains independent of GAS Companion.

## Authoritative files

`BertaDevKitHost/Plugins/BertaGASCompanionExt/Config/Tags/` contains fourteen native sources:

| INI source | Families |
| --- | --- |
| `BertaAbilities.ini` | Ability |
| `BertaStates.ini` | State |
| `BertaStatusEffects.ini` | Status |
| `BertaGameplayEvents.ini` | Event |
| `BertaCooldowns.ini` | Cooldown |
| `BertaDamageAndEffects.ini` | Damage, Effect |
| `BertaResourcesAndInput.ini` | Resource, Input |
| `BertaCombatAndEquipment.ini` | Equipment, Weapon, Item |
| `BertaInteractionAndInventory.ini` | Interaction, Inventory |
| `BertaAIAndTargeting.ini` | AI, Targeting |
| `BertaQuestAndProgression.ini` | Quest, Progression |
| `BertaWorldAndVehicles.ini` | World, Vehicle |
| `BertaCameraAndUI.ini` | Camera, UI |
| `BertaGameplayCues.ini` | GameplayCue |

Each uses the native additional-source class and array format:

```ini
[/Script/GameplayTags.GameplayTagsList]
GameplayTagList=(Tag="Berta.Ability.Movement.Jump",DevComment="Initiates a deliberate takeoff from a supporting surface.")
```

For this catalog's tested UE 5.8 plugin registration path, repeat `GameplayTagList=` entries without a leading `+`. Source discovery alone does not prove that the rows loaded; see the [observed visibility issue and resolution](troubleshooting.md#sources-appear-but-predefined-tags-are-missing).

The complete names in these INI files are authoritative for registration. `DevComment` supplies the picker tooltip; comprehensive semantics live in MkDocs. Entries within each file are sorted by canonical name. Source basenames also matter: UE identifies these sources using the clean INI filename. Keep them distinctive and do not put another unrelated `BertaAbilities.ini` in a different plugin.

The validator holds per-family name fingerprints to protect the approved contract, not a second list that registers or generates tags. Documentation never registers anything. See [maintenance](extension-guide.md#source-of-truth-and-approval-baseline).

## Startup path

The existing Runtime module's `StartupModule()` resolves `BertaGASCompanionExt` through `IPluginManager::Get().FindPlugin()`, combines its installation directory with `Config/Tags` using `FPaths::Combine`, and calls `UGameplayTagsManager::Get().AddTagIniSearchPath()` once. `Projects` is a private module dependency because plugin discovery is an implementation detail. `GameplayTags` was already a public dependency for existing extension APIs.

The loaded module's own plugin descriptor must be discoverable; a `checkf` expresses that internal invariant with diagnostic context. A missing commercial prerequisite is resolved by Unreal's plugin dependency handling before this module can load. No project path or machine installation path is compiled into registration.

In the inspected UE **5.8.2**, changelist **56702186**, `GameplayTagsManager.cpp` shows that `Get()` initializes the manager as necessary, constructs its dictionary, and then the explicit search-path call adds loose INI sources immediately. `AddTagIniSearchPath` recursively discovers `.ini` files, sorts filesystem results, loads `UGameplayTagsList` rows, adds them to the tree, and records whether a path has already been searched/added. It does not require the plugin to contain content assets.

`LaunchEngineLoop.cpp` loads the `PreDefault`, `Default`, and `PostDefault` project/plugin modules before broadcasting `GetOnPostEngineInit()`. The tag manager binds its native-tag finalization to that later delegate. The current `Default` startup therefore registers this catalog before that finalization. No additional delegate, delayed callback, cached directory member, or descriptor loading-phase change is needed.

Do not request catalog tags in static initializers or assume ordering between unrelated `Default` modules. C++ consumers that need tags during their own startup must explicitly establish a provider module dependency/load order. Ordinary runtime requests after engine initialization are the simplest integration. The catalog does not load or construct project gameplay assets during startup.

Shutdown leaves the dictionary to the engine's lifetime. The plugin is intended to be enabled/disabled with an Editor restart, not dynamically unloaded to invalidate serialized tag references. No world delegates or UObject references are retained by the catalog. Repeated registration of the same path is guarded by the native manager; the implementation still has only one caller.

## Import policy

`UGameplayTagsSettings::ImportTagsFromConfig` defaults to `true` in the inspected UE 5.8.2 source. `ShouldImportTagsFromINI()` returns that setting, and dictionary construction/refresh uses it for normal project INI sources and rebuilding registered paths. **Keep Import Tags From Config enabled for supported catalog use.** The plugin does not override project settings.

There is a useful implementation nuance: the direct `AddTagIniSearchPath()` path itself does not test this setting before its initial additional-source load. Initial visibility with import disabled therefore does not establish a reliable import policy, particularly across tree refreshes. Do not use that observation as a reason to disable config import. If a project requires it disabled, evaluate a separate explicit integration requirement rather than silently changing the project or promising equivalent refresh behavior.

## Content and packaging

`CanContainContent` stays `false`. The catalog contains configuration, not `.uasset` content. Neither the native additional-source loader nor the inspected plugin-config staging path requires that flag to be true.

UE 5.8's `CopyBuildToStagingDirectory.Automation.cs` stages `Config/**/*.ini` as UFS files for staged plugins, including their `Tags` subdirectory. A normal enabled plugin distribution should retain these sources, with `IPluginManager` resolving the installed directory at runtime. This is source evidence about the standard staging path, **not an executed package test**. Custom distribution filters, plugin packaging scripts, binary-config settings, or project exclusions can change the result. Inspect the staged files and validate requests in a packaged development build before relying on a distribution.

For fast Gameplay Tag replication, client and server must use compatible dictionaries and catalog versions. Registration does not replicate custom actor state. A package with the extension disabled has no guarantee these names remain registered from another source; dependent assets and code must not assume they do.

## Evidence and compatibility review

Version-sensitive decisions were checked against local UE 5.8.2 source:

- `Engine/Source/Runtime/GameplayTags/Private/GameplayTagsManager.cpp`: search paths, source loading, initialization, import gating, and finalization.
- `Engine/Source/Runtime/GameplayTags/Classes/GameplayTagsSettings.h` and its `.cpp`: list class, config array, redirects, and default import policy.
- `Engine/Source/Runtime/Launch/Private/LaunchEngineLoop.cpp`: module phases relative to PostEngineInit.
- `Engine/Source/Runtime/Projects/Private/PluginManager.cpp`: ordinary plugin initialization; no generic Gameplay Tags directory registration was found there.
- `Engine/Source/Programs/AutomationTool/Scripts/CopyBuildToStagingDirectory.Automation.cs`: staged plugin config traversal.
- Native `GameplayAbility.h`, `AbilitySystemComponent.h`, `AbilitySystemBlueprintLibrary.h`, and `TargetTagsGameplayEffectComponent.h`: integration examples and component-era GAS configuration.

Official references: [Gameplay Tags overview](https://dev.epicgames.com/documentation/unreal-engine/using-gameplay-tags-in-unreal-engine), [UGameplayTagsManager](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/GameplayTags/UGameplayTagsManager), and [FGameplayTagContainer](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/GameplayTags/FGameplayTagContainer).

The repository's existing tag consumers, tests, and config were inspected before adding sources. No existing catalog declaration conflicted with these names. The available local GAS Companion config had no competing Berta tag list. No local Lyra source was available; Epic's [Lyra ability documentation](https://dev.epicgames.com/documentation/unreal-engine/abilities-in-lyra-in-unreal-engine) uses examples such as `Ability.Type.Action.Dash` and `InputTag.Ability.Dash`. Those are distinct names, not aliases automatically mapped to Berta. This is a scoped inspection, not proof about every future third-party plugin. No commercial source or assets were copied, and no approved identifier was renamed.
