# Universal Gameplay Tags catalog

A Gameplay Tag is a named, hierarchical label in Unreal's shared tag dictionary. An object can store tags in an `FGameplayTag` or `FGameplayTagContainer`, expose them through a tag provider, or use them as configuration for GAS. Projects evaluate these labels to make decisions. A label has no executable behavior of its own.

BertaGASCompanionExt provides **474 explicit tags across 23 conceptual families**, ready to reuse in Unreal Engine **5.8** projects. The catalog supplies a common vocabulary for recurring concepts so each new prototype does not need to invent its own jump, life-state, damage, inventory, targeting, and presentation terminology. Projects can adopt any subset. Implicit parent nodes visible in the picker are additional hierarchy nodes, not additional explicit catalog entries.

The ordinary namespace is `Berta.*`. Cosmetic cue identifiers use `GameplayCue.Berta.*` so they participate in Unreal's native cue hierarchy. Names are stable API: serialized assets and code can depend on them. Read [conventions](conventions.md) before inventing a near-equivalent name.

## Start using the catalog

1. Install the existing BertaDevKit and BertaGASCompanionExt plugins in your UE 5.8 project, preserving their directory structure including `Config/Tags`. Install a licensed local GAS Companion copy, either as a discoverable engine plugin or project plugin. Commercial source is not distributed here.
2. Enable GAS Companion and BertaGASCompanionExt in **Edit → Plugins**. The extension descriptor also declares its BertaDevKit and other existing native prerequisites. Resolve missing dependencies before continuing.
3. Restart Unreal Editor. In **Edit → Project Settings → Project → Gameplay Tags**, keep **Import Tags From Config** enabled. This is the native default in the inspected UE 5.8.2 installation; do not change a project's deliberate policy silently. See the [registration details](architecture.md#import-policy).
4. Open **Manage Gameplay Tags**. Search for `Berta.Ability.Movement.Jump`, `Berta.State.Movement.Jumping`, and `GameplayCue.Berta.Movement.Jump`. Inspect their source and tooltip comments.
5. Select a registered tag in a Blueprint Gameplay Tag property or container. Follow the [integration guide](integration.md) to give that selection a project-owned use.

Enabling the catalog **registers vocabulary**. It does not grant an ability, add actor-owned tags, apply an effect, change movement, bind input, emit an event, execute a cue, apply cooldowns, create attributes, or install replication/gameplay logic. An empty ASC remains empty after registration. Availability in the picker is different from ownership on an actor.

## Reading path

- [Architecture and UE 5.8 evidence](architecture.md): files, startup, dependencies, import policy, and staging assumptions.
- [Conventions](conventions.md): names, composition, matching direction, explicit parents, and compatibility.
- [Integration and GA_Jump](integration.md): Blueprint, C++, GAS, and manual Sandbox adoption.
- [Ownership and lifecycle](lifecycle.md): source of truth, overlapping contributions, authority, and cleanup.
- [Extension and migration guide](extension-guide.md): adding, documenting, validating, deprecating, and redirecting names.
- [Troubleshooting and manual verification](troubleshooting.md): diagnosis and the checks that still require Unreal.

## Family reference

Every registered tag has its own heading and explanation in the appropriate reference page. Entries specify meaning, scope, suggested owner, lifecycle, example, implementation responsibility, and useful related-entry links. Suggested owners are recommendations for project systems; the plugin does not create them.

| Family | Explicit tags | Domain |
| --- | ---: | --- |
| [Ability](catalog/ability.md) | 52 | Capability identifiers |
| [State](catalog/state.md) | 51 | Actor conditions |
| [Status](catalog/status.md) | 23 | Recipient conditions |
| [Event](catalog/event.md) | 50 | Occurrences and transitions |
| [Cooldown](catalog/cooldown.md) | 48 | Potential GAS cooldown restrictions |
| [Damage](catalog/damage.md) | 23 | Type, source, and hit-zone classifications |
| [Effect](catalog/effect.md) | 8 | Effect-purpose classifications |
| [Resource](catalog/resource.md) | 10 | Resource concepts, independent of numeric providers |
| [Input](catalog/input.md) | 17 | Input-intent labels |
| [Interaction](catalog/interaction.md) | 16 | Affordances and their availability |
| [Equipment](catalog/equipment.md) | 12 | Equipment positions and instance conditions |
| [Weapon](catalog/weapon.md) | 18 | Weapon types, fire modes, and conditions |
| [Item](catalog/item.md) | 16 | Item types, rarity, and instance conditions |
| [Inventory](catalog/inventory.md) | 8 | Inventory conditions and transaction events |
| [Targeting](catalog/targeting.md) | 18 | Selection geometry, relations, and predicates |
| [AI](catalog/ai.md) | 17 | Decisions, perception labels, and archetype roles |
| [Quest](catalog/quest.md) | 11 | Participant quest and objective records |
| [Progression](catalog/progression.md) | 6 | Progression events and derived conditions |
| [World](catalog/world.md) | 19 | Surface, region, and weather metadata |
| [Vehicle](catalog/vehicle.md) | 10 | Vehicle types, operation, and seat roles |
| [Camera](catalog/camera.md) | 8 | Camera policies |
| [UI](catalog/ui.md) | 13 | Local contexts, element state, and prompt metadata |
| [GameplayCue](catalog/gameplay-cue.md) | 20 | Cosmetic feedback identifiers |
| **Total** | **474** | Explicit declarations only |

## Verification boundary

The static checker verifies the approved name set, counts, configuration structure, comments, and documentation coverage. MkDocs checks the documentation site. UBT can check C++ compilation with the optional plugin explicitly included. None of those proves the picker, packaged runtime, network behavior, or a Sandbox Blueprint works. Follow the [manual checklist](troubleshooting.md#manual-acceptance-checklist) for those checks. No Unreal process is required to run the static checker or MkDocs.
