# Item — 16 tags

Item classifications belong to inventory definitions or instances. Type describes purpose, Rarity describes a project's named presentation or distribution tier, and State describes current instance eligibility or damage. A single item may compose a type and rarity without encoding the combination in a new tag. Type labels may overlap when a project explicitly supports multi-role items.

Rarity tags have no numeric order or drop probability in Unreal's tag system. Define ordering, colors, economics, and balance in project data; do not sort tag names alphabetically to infer rarity. Usable requires a project predicate, and Broken requires a durability or functionality policy. Item.State.Broken and Equipment.State.Broken can describe related representations, but their owners must remain explicit. No inventory, consumption, or repair implementation is included.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Item.Rarity

### Berta.Item.Rarity.Common

**Meaning:** Classifies the project's ordinary availability or quality tier.

**Scope and suggested owner:** object/archetype classification; item definition or deliberately assigned item-instance metadata.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A loot UI selects presentation for ordinary items.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Rarity.Epic

**Meaning:** Classifies the project's epic availability or quality tier.

**Scope and suggested owner:** object/archetype classification; item definition or deliberately assigned item-instance metadata.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A reward table groups high-tier rewards.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Rarity.Legendary

**Meaning:** Classifies the project's legendary availability or quality tier.

**Scope and suggested owner:** object/archetype classification; item definition or deliberately assigned item-instance metadata.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A loot UI selects its highest named catalog tier.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Rarity.Rare

**Meaning:** Classifies the project's rare availability or quality tier.

**Scope and suggested owner:** object/archetype classification; item definition or deliberately assigned item-instance metadata.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A loot UI selects rare-item presentation.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Rarity.Uncommon

**Meaning:** Classifies a tier above the project's common baseline.

**Scope and suggested owner:** object/archetype classification; item definition or deliberately assigned item-instance metadata.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A loot table groups less frequently available equipment.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

## Item.State

### Berta.Item.State.Broken

**Meaning:** The item instance is considered damaged beyond normal use.

**Scope and suggested owner:** inventory-item instance condition; item eligibility or durability controller.

**Lifecycle:** Update when use prerequisites, damage, repair, or ownership context change. Keep definition classification separate.

**Example:** A repair menu filters broken inventory objects.

**Related entries:** [Berta.Equipment.State.Broken](equipment.md#bertaequipmentstatebroken), [Berta.Equipment.State.Equipped](equipment.md#bertaequipmentstateequipped).

**Important distinctions:** Item and equipment condition belong to their corresponding representations. Broken equipment can remain equipped until project rules remove it.

**Implementation responsibility:** The project implements item use, validation, durability, and persistence.

### Berta.Item.State.Usable

**Meaning:** The item instance currently meets its own use prerequisites.

**Scope and suggested owner:** inventory-item instance condition; item eligibility or durability controller.

**Lifecycle:** Update when use prerequisites, damage, repair, or ownership context change. Keep definition classification separate.

**Example:** A consumable menu enables use after the item controller validates it.

**Implementation responsibility:** The project implements item use, validation, durability, and persistence.

## Item.Type

### Berta.Item.Type.Ammo

**Meaning:** Classifies an inventory object supplying ammunition.

**Scope and suggested owner:** object/archetype classification; item definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A reload rule selects compatible ammunition stacks.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Type.Armor

**Meaning:** Classifies an inventory object whose primary role is worn protection.

**Scope and suggested owner:** object/archetype classification; item definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** An inventory filter groups protective gear.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Type.Consumable

**Meaning:** Classifies an inventory object expended through use.

**Scope and suggested owner:** object/archetype classification; item definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A quick-slot UI lists potions and food.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Type.Currency

**Meaning:** Classifies an inventory object used as exchange value.

**Scope and suggested owner:** object/archetype classification; item definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A trade interface selects accepted currency representations.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Type.Key

**Meaning:** Classifies an inventory object serving an access function.

**Scope and suggested owner:** object/archetype classification; item definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** An interaction rule searches key items for an access credential.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Type.Material

**Meaning:** Classifies an inventory object used as a production ingredient.

**Scope and suggested owner:** object/archetype classification; item definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A crafting interface filters ore and other ingredients.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Type.Quest

**Meaning:** Classifies an inventory object associated with quest progression.

**Scope and suggested owner:** object/archetype classification; item definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A drop policy checks whether quest-linked items require special handling.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Type.Tool

**Meaning:** Classifies an inventory object used for non-weapon utility work.

**Scope and suggested owner:** object/archetype classification; item definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A harvesting rule selects a compatible tool.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Item.Type.Weapon

**Meaning:** Classifies an inventory object whose primary role is combat equipment.

**Scope and suggested owner:** object/archetype classification; item definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** An inventory filter groups equipable weapons.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.
