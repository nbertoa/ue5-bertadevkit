# Weapon — 18 tags

Weapon.Type classifies a weapon definition. Weapon.FireMode classifies a selected or supported firing policy, and Weapon.State describes a weapon instance's current operational condition. A weapon may support several fire modes but needs separate project data for the currently selected mode. These sibling type labels do not imply a rifle is automatically tagged Melee, Sniper, or another sibling.

The weapon controller owns readiness, ammunition, firing cadence, reload, and malfunction transitions. Weapon.State.Reloading belongs to the weapon; State.Combat.Reloading belongs to the character; Ability.Combat.Reload identifies a capability; Event.Combat.ReloadCompleted announces an occurrence. Decide which controller produces each instead of mirroring them indiscriminately. Weapon tags neither determine an AI's tactical role nor automatically classify outgoing damage.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Weapon.FireMode

### Berta.Weapon.FireMode.Automatic

**Meaning:** Classifies a firing mode that repeats while trigger conditions remain satisfied.

**Scope and suggested owner:** weapon firing-policy classification; weapon firing profile or selected-mode data on the weapon controller.

**Lifecycle:** Assign to supported-mode definitions; track the currently selected mode separately when a weapon supports several.

**Example:** A weapon profile selects continuous trigger handling.

**Implementation responsibility:** The project implements trigger processing, cadence, selection, ammunition use, and networking.

### Berta.Weapon.FireMode.Burst

**Meaning:** Classifies a firing mode that produces a bounded sequence per accepted trigger.

**Scope and suggested owner:** weapon firing-policy classification; weapon firing profile or selected-mode data on the weapon controller.

**Lifecycle:** Assign to supported-mode definitions; track the currently selected mode separately when a weapon supports several.

**Example:** A weapon profile selects a three-shot burst policy.

**Implementation responsibility:** The project implements trigger processing, cadence, selection, ammunition use, and networking.

### Berta.Weapon.FireMode.Single

**Meaning:** Classifies a firing mode that produces one discharge per accepted trigger cycle.

**Scope and suggested owner:** weapon firing-policy classification; weapon firing profile or selected-mode data on the weapon controller.

**Lifecycle:** Assign to supported-mode definitions; track the currently selected mode separately when a weapon supports several.

**Example:** A weapon profile selects semi-automatic trigger handling.

**Implementation responsibility:** The project implements trigger processing, cadence, selection, ammunition use, and networking.

## Weapon.State

### Berta.Weapon.State.Firing

**Meaning:** The weapon instance is currently executing a firing sequence.

**Scope and suggested owner:** weapon-instance condition; weapon controller and weapon instance data.

**Lifecycle:** Maintain during the corresponding operational phase; reconcile cancellation, ammunition changes, malfunction clearing, and destruction.

**Example:** A weapon controller maintains muzzle presentation for a firing phase.

**Implementation responsibility:** The project implements weapon operation and exposes character-level state only when appropriate.

### Berta.Weapon.State.Jammed

**Meaning:** The weapon instance is obstructed by a firing malfunction.

**Scope and suggested owner:** weapon-instance condition; weapon controller and weapon instance data.

**Lifecycle:** Maintain during the corresponding operational phase; reconcile cancellation, ammunition changes, malfunction clearing, and destruction.

**Example:** A weapon controller requires a clearing action before firing.

**Implementation responsibility:** The project implements weapon operation and exposes character-level state only when appropriate.

### Berta.Weapon.State.Ready

**Meaning:** The weapon instance is operationally ready under its controller's rules.

**Scope and suggested owner:** weapon-instance condition; weapon controller and weapon instance data.

**Lifecycle:** Maintain during the corresponding operational phase; reconcile cancellation, ammunition changes, malfunction clearing, and destruction.

**Example:** A firing controller checks readiness before accepting a shot.

**Implementation responsibility:** The project implements weapon operation and exposes character-level state only when appropriate.

### Berta.Weapon.State.Reloading

**Meaning:** The weapon instance is currently undergoing ammunition replenishment.

**Scope and suggested owner:** weapon-instance condition; weapon controller and weapon instance data.

**Lifecycle:** Maintain during the corresponding operational phase; reconcile cancellation, ammunition changes, malfunction clearing, and destruction.

**Example:** A weapon controller rejects firing during its reload operation.

**Related entries:** [Berta.Ability.Combat.Reload](ability.md#bertaabilitycombatreload), [Berta.Event.Combat.ReloadCompleted](event.md#bertaeventcombatreloadcompleted), [Berta.State.Combat.Reloading](state.md#bertastatecombatreloading).

**Important distinctions:** These describe weapon condition, character condition, capability identity, and completed occurrence respectively. Their owners and lifetimes can differ.

**Implementation responsibility:** The project implements weapon operation and exposes character-level state only when appropriate.

## Weapon.Type

### Berta.Weapon.Type.Axe

**Meaning:** Classifies a weapon built around an axe-style cutting head.

**Scope and suggested owner:** object/archetype classification; weapon definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** An equipment filter selects axe-compatible upgrades.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Weapon.Type.Bow

**Meaning:** Classifies a bow-style projectile weapon.

**Scope and suggested owner:** object/archetype classification; weapon definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** An equipment UI groups bows independently of projectile damage source.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Weapon.Type.Dagger

**Meaning:** Classifies a short stabbing or cutting blade.

**Scope and suggested owner:** object/archetype classification; weapon definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A loadout filter selects dagger-specific animations.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Weapon.Type.Melee

**Meaning:** Classifies a weapon intended for close-contact attacks.

**Scope and suggested owner:** object/archetype classification; weapon definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A loadout filter selects close-range weapons regardless of shape.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Weapon.Type.Pistol

**Meaning:** Classifies a firearm with handgun-style construction and handling.

**Scope and suggested owner:** object/archetype classification; weapon definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A loadout filter selects pistol handling animations.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Weapon.Type.Rifle

**Meaning:** Classifies a firearm with rifle-style construction and handling.

**Scope and suggested owner:** object/archetype classification; weapon definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** An equipment filter accepts rifle attachments.

**Related entries:** [Berta.AI.Role.Ranged](ai.md#bertaairoleranged).

**Important distinctions:** Ranged is an agent design role; Rifle is weapon classification. A ranged agent may use a spell or bow and a rifle does not determine its role.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Weapon.Type.Shotgun

**Meaning:** Classifies a firearm with shotgun-style construction and handling.

**Scope and suggested owner:** object/archetype classification; weapon definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A loadout rule selects shotgun handling data.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Weapon.Type.Sniper

**Meaning:** Classifies a weapon intended for a sniper-style precision role.

**Scope and suggested owner:** object/archetype classification; weapon definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A loadout UI groups precision weapons under a project-defined sniper category.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Weapon.Type.Sword

**Meaning:** Classifies a weapon built around a sword-style blade.

**Scope and suggested owner:** object/archetype classification; weapon definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** An animation rule selects sword-compatible attack sets.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Weapon.Type.Throwing

**Meaning:** Classifies a weapon intended to be thrown.

**Scope and suggested owner:** object/archetype classification; weapon definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** An inventory filter selects throwable knives.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Weapon.Type.Unarmed

**Meaning:** Classifies a weapon representation for attacks without a separate held weapon.

**Scope and suggested owner:** object/archetype classification; weapon definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A combat definition represents fists as an unarmed loadout option.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.
