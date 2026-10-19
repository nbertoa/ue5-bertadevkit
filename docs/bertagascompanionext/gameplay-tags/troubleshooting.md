# Troubleshooting and manual verification

Begin by identifying which layer failed: plugin discovery, source registration, name lookup, runtime ownership, or project gameplay behavior. These layers have different evidence. An entry in Gameplay Tag Manager proves dictionary visibility in that session; it does not prove an ASC owns it or an ability implements its meaning.

## Tags do not appear in the picker

1. In **Edit → Plugins**, confirm **GAS Companion** and **BertaGASCompanionExt** are enabled, with the existing descriptor prerequisites satisfied. A plugin folder on disk is insufficient if it is disabled. The extension is disabled by default.
2. If the commercial prerequisite is missing, install a licensed copy as a project or engine plugin recognized by UE 5.8. The public repository intentionally omits it. Do not copy commercial source into a public commit or remove the dependency to hide the setup problem.
3. Restart Editor after enabling the extension or editing tag INI files. Do not rely on hot reload to reconcile all serialized references or dictionary state.
4. Inspect **Project Settings → Project → Gameplay Tags → Import Tags From Config**. Keep it enabled for this integration. See [import policy](architecture.md#import-policy) for the initial explicit-call/refresh distinction.
5. Open **Manage Gameplay Tags** and search for the full name. Remove picker search text or metadata/category filters that may be hiding the family. A cue-specific picker is expected to restrict selections to the cue hierarchy.
6. Confirm the plugin installation includes `Config/Tags/*.ini` with the correct section and complete rows. The Runtime module resolves its own plugin directory; the files do not belong in `Content` or a hardcoded host-only directory.
7. If needed, debug the native Runtime `StartupModule` and `AddTagIniSearchPath` call in your local development session. Inspect the resolved directory and discovered files. No custom registration settings or Berta console commands exist.

Do not redeclare a missing name in `DefaultGameplayTags.ini` as the first workaround: that hides why the plugin source was unavailable and creates source ambiguity later.

## Wrong source, duplicate name, or collision

Inspect the entry's source in Gameplay Tag Manager and compare the catalog's [source-file table](architecture.md#authoritative-files). A Berta name supplied by a game config file, native declaration, or third-party table deserves investigation even if matching works. Search project/plugin config, data tables, and native declarations for duplicate names. Check clean source filenames as well as full tag names.

Run `python Scripts/Validate-GameplayTagCatalog.py` to detect duplicates within this catalog. That scope does not include arbitrary external plugins or binary project assets. Use the native manager and the generic usage finder for the actual project's sources and serialized users. Resolve equivalence through an explicit project mapping; do not silently rename approved Berta identifiers.

## Missing comment or unexpected entry count

The picker can show implicit ancestors in addition to the 474 explicit rows. Counting every visible node will exceed the declared catalog count. An implicit parent need not have a standalone tooltip description. Inspect an explicit leaf such as `Berta.Ability.Movement.Jump` and verify its `DevComment` and source.

The validator checks nonempty substantive comments, distinct comments, ordering, and per-family fingerprints. It does not interpret English quality or prove native parsing. If the comment shown differs from the source, check duplicates, stale Editor state, and the actual plugin copy being loaded before editing the text again.

## Invalid serialized or Blueprint references

If a reference becomes invalid after disabling/removing the extension, re-enable its provider and restart before investigating a rename. A project asset referencing these names has an explicit catalog dependency even when the plugin remains globally optional. Decide whether that project should require it or migrate those references to a project-owned vocabulary before disabling it.

For renamed names, inspect native redirects and the [migration procedure](extension-guide.md#deprecating-and-renaming). Locate references in Blueprint defaults, effects, queries, data assets, and code. Do not assume a valid new name proves every old serialized reference migrated. Avoid mass resaves or deletion of invalid references without understanding the intended replacement.

## C++ requests fail

`FGameplayTag::RequestGameplayTag` performs lookup rather than registration. Confirm spelling, capitalization, enabled plugin, imported source, and request timing. Do not request these names in static initializers. A consumer needing them during startup must explicitly order itself after the provider module. Request after engine initialization when possible.

For optional integrations, use the native `ErrorIfNotFound=false` argument only with an explicit `IsValid()` check and a clear missing-provider outcome. Suppressing the diagnostic and treating an invalid restriction as success can create eligibility bugs. A required catalog request failing is a setup/contract defect to diagnose, not a reason to invent a runtime tag registry.

## Registered but absent on the ASC

This is expected until your project grants an owned tag through an appropriate mechanism. Ability Asset Tags classify an ability; they do not populate the actor's owned-tag list. A Blueprint container is not automatically the ASC container. An event occurrence is not persistent state. Inspect the actual active effects, activation-owned tags, loose-tag path, and intended owner.

If a state disappears too early, compare its provider's lifetime with ability, movement, and effect lifetimes. If it remains too long, inspect interruption, removal, respawn, and overlapping-source cleanup. Keep one source of truth and do not add a second independent toggle merely to make a diagnostic look correct.

## Cooldown tags have no effect

Confirm a duration Gameplay Effect grants the cooldown tag through **Grant Tags to Target Actor**, the ability selects that effect, and its real activation/commit path checks and applies it. Inspect effect duration and removal. A dictionary entry, ability identifier, local tag variable, or cue does not create a cooldown. Review custom GSC/project ability overrides against native `GetCooldownTags`/`CheckCooldown` behavior.

## Gameplay Cue is not recognized or produces no feedback

Verify the identifier starts with `GameplayCue.Berta`, and inspect `BertaGameplayCues.ini` as its dictionary source. Then verify there is an actual project cue notify/handler associated with that name, that its asset directory participates in GAS cue discovery, and that gameplay explicitly invokes it or applies an effect configured with it.

Registration supplies no audio, particles, cue assets, or handlers. Inspect one-shot versus active cue lifetime, parameters, local/server invocation, prediction, and suppression. Test on the intended clients. Do not infer failed gameplay from missing cosmetics or move authoritative damage into the cue to make it visible.

## Missing tags in a packaged build

Check the extension is enabled for the packaged target and its native prerequisites are present. Inspect staging manifests/package contents for this plugin's `Config/Tags` files and confirm custom filters did not strip them. The standard staging code includes plugin config INI files independently of `CanContainContent`; this release's source inspection is not a package test.

In a packaged Development build, request representative tags after initialization and inspect explicit validity diagnostics from your project. Check that no editor-only module performs registration and no project-specific absolute directory was assumed. Verify cues separately because packaged asset discovery/loading differs from tag dictionary registration. Do not copy all Berta definitions into project config as a packaging fix without first resolving the missing plugin distribution.

## Network dictionary or ownership mismatch

When fast Gameplay Tag replication is enabled, client/server dictionaries must be compatible. Compare plugin enablement, catalog revision, project definitions, config import policy, build configuration, and cooked/staged sources. Never hot-deploy a different tag dictionary to only one side and assume the shared namespace guarantees compatible indices.

If the dictionary matches but owned state differs, inspect the actual GAS granting/replication mechanism and authority. A local container or ordinary loose-tag mutation does not by itself establish the required server/client synchronization. Cue replication is cosmetic and is not a reliable replacement for replicated gameplay state. The catalog adds no custom replication path.

## Manual acceptance checklist

These checks require Nicolas to launch UE 5.8 manually. They are **pending until observed**; static validation, MkDocs, and UBT do not pass them on his behalf.

1. With a licensed GAS Companion installation, enable **GAS Companion** and **BertaGASCompanionExt**, resolve prerequisites, and restart Editor.
2. Open **Edit → Project Settings → Project → Gameplay Tags**, verify **Import Tags From Config**, then open **Manage Gameplay Tags**.
3. Inspect plugin sources such as `BertaAbilities.ini`, `BertaStates.ini`, and `BertaGameplayCues.ini`; verify the full thematic source set listed in Architecture. Check for unexpected second sources or conflicting definitions.
4. Find exactly `Berta.Ability.Movement.Jump`, `Berta.State.Movement.Jumping`, and `GameplayCue.Berta.Movement.Jump`. Hover each and compare its tooltip with the INI `DevComment`.
5. In a project test Blueprint, add a Gameplay Tag property and Gameplay Tag Container property. Select representative catalog entries, compile, save, reopen, and confirm selections persist.
6. After engine initialization, use the native C++ request example for representative names and inspect validity. Confirm a deliberately optional missing project name returns invalid rather than silently satisfying a restriction.
7. Put only `Berta.Ability.Combat.Attack.Light` in a test container. Confirm non-exact Attack matches, exact Attack does not, exact Light does, and Heavy does not. Then put only Attack in another container and confirm Light does not match.
8. Inspect an otherwise empty test actor ASC before and after enabling the catalog. Confirm registration did not grant abilities, effects, owned state, cooldowns, input bindings, or numeric attributes.
9. In the Sandbox, manually follow the [GA_Jump walkthrough](integration.md#complete-manual-ga_jump-walkthrough), keeping the working GSC parent classes and input/grant setup. Test takeoff, held/released input, walk-off, ability end/cancellation, landing, forced mode change, and respawn against native movement. Confirm there is one movement-state authority and no duplicate events/cues.
10. If cooldowns, event listeners, or cue assets are adopted, test each configured mechanism independently. Repeat the relevant checks with server/owning client when networking is required.
11. Package through the project's authorized workflow, retain the plugin config sources, and repeat representative C++ lookup, ownership, Blueprint, hierarchy, and cue checks in a packaged Development runtime. Inspect dictionary compatibility for networked builds.
12. Confirm no unexpected duplicate definitions or semantic collisions with the complete enabled-plugin set. Record UE version, plugin/catalog revision, target, and observed results before calling runtime/packaging verification complete.
