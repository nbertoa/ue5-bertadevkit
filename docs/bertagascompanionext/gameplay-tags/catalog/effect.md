# Effect — 8 tags

Effect categories describe the purpose of an effect definition or spec. In GAS they are suitable asset classifications on a Gameplay Effect, rather than automatically granted target tags. An effect can be both Buff and Healing, or Movement and Debuff, when that composition is meaningful. The project defines filter semantics instead of assuming these categories are exclusive.

The Gameplay Effect and its components implement duration, stacking, modifiers, application requirements, and granted tags. Merely labeling an effect Damage does not damage anyone. A UI may group effects by categories; a cleanse policy still needs explicit eligibility and removal rules. Keep the category on the effect metadata, the recipient condition in Status or State, and numeric values in attributes or project data.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Effect.Category

### Berta.Effect.Category.Buff

**Meaning:** The effect is classified as beneficial under project rules.

**Scope and suggested owner:** effect-purpose classification; Gameplay Effect asset/spec metadata or a non-GAS effect definition.

**Lifecycle:** Assign as effect metadata and inspect while selecting or evaluating effects. Do not grant to the target merely to classify the effect.

**Example:** An effect browser groups allied enhancement effects.

**Implementation responsibility:** The project configures effect behavior and classification-aware filters or UI.

### Berta.Effect.Category.Control

**Meaning:** The effect's declared purpose includes restricting or redirecting control.

**Scope and suggested owner:** effect-purpose classification; Gameplay Effect asset/spec metadata or a non-GAS effect definition.

**Lifecycle:** Assign as effect metadata and inspect while selecting or evaluating effects. Do not grant to the target merely to classify the effect.

**Example:** A resistance policy selects crowd-control effects.

**Implementation responsibility:** The project configures effect behavior and classification-aware filters or UI.

### Berta.Effect.Category.Damage

**Meaning:** The effect's declared purpose includes causing harm.

**Scope and suggested owner:** effect-purpose classification; Gameplay Effect asset/spec metadata or a non-GAS effect definition.

**Lifecycle:** Assign as effect metadata and inspect while selecting or evaluating effects. Do not grant to the target merely to classify the effect.

**Example:** A cleanse UI distinguishes damaging effects from healing effects.

**Implementation responsibility:** The project configures effect behavior and classification-aware filters or UI.

### Berta.Effect.Category.Debuff

**Meaning:** The effect is classified as adverse under project rules.

**Scope and suggested owner:** effect-purpose classification; Gameplay Effect asset/spec metadata or a non-GAS effect definition.

**Lifecycle:** Assign as effect metadata and inspect while selecting or evaluating effects. Do not grant to the target merely to classify the effect.

**Example:** A cleanse policy selects eligible adverse effects.

**Implementation responsibility:** The project configures effect behavior and classification-aware filters or UI.

### Berta.Effect.Category.Healing

**Meaning:** The effect's declared purpose includes restoring health.

**Scope and suggested owner:** effect-purpose classification; Gameplay Effect asset/spec metadata or a non-GAS effect definition.

**Lifecycle:** Assign as effect metadata and inspect while selecting or evaluating effects. Do not grant to the target merely to classify the effect.

**Example:** A healing modifier query selects effects classified as restorative.

**Implementation responsibility:** The project configures effect behavior and classification-aware filters or UI.

### Berta.Effect.Category.Movement

**Meaning:** The effect's declared purpose includes changing locomotion.

**Scope and suggested owner:** effect-purpose classification; Gameplay Effect asset/spec metadata or a non-GAS effect definition.

**Lifecycle:** Assign as effect metadata and inspect while selecting or evaluating effects. Do not grant to the target merely to classify the effect.

**Example:** A movement modifier inspector groups speed-related effects.

**Implementation responsibility:** The project configures effect behavior and classification-aware filters or UI.

### Berta.Effect.Category.Shield

**Meaning:** The effect's declared purpose includes protective shielding.

**Scope and suggested owner:** effect-purpose classification; Gameplay Effect asset/spec metadata or a non-GAS effect definition.

**Lifecycle:** Assign as effect metadata and inspect while selecting or evaluating effects. Do not grant to the target merely to classify the effect.

**Example:** A barrier UI groups shield-producing effects.

**Implementation responsibility:** The project configures effect behavior and classification-aware filters or UI.

### Berta.Effect.Category.Utility

**Meaning:** The effect serves a supporting purpose outside the more specific categories.

**Scope and suggested owner:** effect-purpose classification; Gameplay Effect asset/spec metadata or a non-GAS effect definition.

**Lifecycle:** Assign as effect metadata and inspect while selecting or evaluating effects. Do not grant to the target merely to classify the effect.

**Example:** A reveal effect is classified for project UI organization.

**Implementation responsibility:** The project configures effect behavior and classification-aware filters or UI.
