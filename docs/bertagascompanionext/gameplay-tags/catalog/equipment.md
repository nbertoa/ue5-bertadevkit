# Equipment — 12 tags

Slot labels classify intended equipment positions in definitions or loadout rules. They are semantic positions rather than socket names, skeletal bones, or a complete equipment-slot implementation. Slot multiplicity, two-handed occupancy, compatibility, and accessory indices remain project data. Do not invent numbered tags for every inventory position.

Equipment.State belongs to the equipment instance or its authoritative equipment controller. Equipped records a committed assignment, while the character's Equipping state records a transition. Broken describes functionality or durability and may coexist with Equipped. Neither tag applies stats, attaches meshes, grants abilities, or chooses replication. An inventory Item.Type.Weapon and a Weapon.Type.Rifle label can accompany a MainHand equipment assignment.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Equipment.Slot

### Berta.Equipment.Slot.Accessory

**Meaning:** Classifies an accessory equipment position.

**Scope and suggested owner:** equipment-position classification; equipment-slot definition or loadout compatibility rule.

**Lifecycle:** Assign as slot metadata. Equip transactions change occupancy separately from this classification.

**Example:** An equipment rule accepts a ring in an accessory slot.

**Implementation responsibility:** The project implements slot multiplicity, compatibility, sockets, attachment, and occupancy.

### Berta.Equipment.Slot.Back

**Meaning:** Classifies a back-mounted equipment position.

**Scope and suggested owner:** equipment-position classification; equipment-slot definition or loadout compatibility rule.

**Lifecycle:** Assign as slot metadata. Equip transactions change occupancy separately from this classification.

**Example:** An equipment rule accepts a backpack in the back slot.

**Implementation responsibility:** The project implements slot multiplicity, compatibility, sockets, attachment, and occupancy.

### Berta.Equipment.Slot.Chest

**Meaning:** Classifies the equipment position intended for torso-worn objects.

**Scope and suggested owner:** equipment-position classification; equipment-slot definition or loadout compatibility rule.

**Lifecycle:** Assign as slot metadata. Equip transactions change occupancy separately from this classification.

**Example:** An equipment rule accepts body armor in the chest slot.

**Implementation responsibility:** The project implements slot multiplicity, compatibility, sockets, attachment, and occupancy.

### Berta.Equipment.Slot.Feet

**Meaning:** Classifies the equipment position intended for foot-worn objects.

**Scope and suggested owner:** equipment-position classification; equipment-slot definition or loadout compatibility rule.

**Lifecycle:** Assign as slot metadata. Equip transactions change occupancy separately from this classification.

**Example:** An equipment rule accepts boots in the feet slot.

**Implementation responsibility:** The project implements slot multiplicity, compatibility, sockets, attachment, and occupancy.

### Berta.Equipment.Slot.Gadget

**Meaning:** Classifies a utility-device equipment position.

**Scope and suggested owner:** equipment-position classification; equipment-slot definition or loadout compatibility rule.

**Lifecycle:** Assign as slot metadata. Equip transactions change occupancy separately from this classification.

**Example:** An equipment rule accepts a deployable scanner in a gadget slot.

**Implementation responsibility:** The project implements slot multiplicity, compatibility, sockets, attachment, and occupancy.

### Berta.Equipment.Slot.Hands

**Meaning:** Classifies the equipment position intended for hand-worn objects.

**Scope and suggested owner:** equipment-position classification; equipment-slot definition or loadout compatibility rule.

**Lifecycle:** Assign as slot metadata. Equip transactions change occupancy separately from this classification.

**Example:** An equipment rule accepts gloves in the hands slot.

**Implementation responsibility:** The project implements slot multiplicity, compatibility, sockets, attachment, and occupancy.

### Berta.Equipment.Slot.Head

**Meaning:** Classifies the equipment position intended for head-worn objects.

**Scope and suggested owner:** equipment-position classification; equipment-slot definition or loadout compatibility rule.

**Lifecycle:** Assign as slot metadata. Equip transactions change occupancy separately from this classification.

**Example:** An equipment rule accepts a helmet in the head slot.

**Implementation responsibility:** The project implements slot multiplicity, compatibility, sockets, attachment, and occupancy.

### Berta.Equipment.Slot.Legs

**Meaning:** Classifies the equipment position intended for leg-worn objects.

**Scope and suggested owner:** equipment-position classification; equipment-slot definition or loadout compatibility rule.

**Lifecycle:** Assign as slot metadata. Equip transactions change occupancy separately from this classification.

**Example:** An equipment rule accepts leggings in the legs slot.

**Implementation responsibility:** The project implements slot multiplicity, compatibility, sockets, attachment, and occupancy.

### Berta.Equipment.Slot.MainHand

**Meaning:** Classifies the primary hand-held equipment position.

**Scope and suggested owner:** equipment-position classification; equipment-slot definition or loadout compatibility rule.

**Lifecycle:** Assign as slot metadata. Equip transactions change occupancy separately from this classification.

**Example:** An equipment rule assigns the primary weapon to main hand.

**Implementation responsibility:** The project implements slot multiplicity, compatibility, sockets, attachment, and occupancy.

### Berta.Equipment.Slot.OffHand

**Meaning:** Classifies the secondary hand-held equipment position.

**Scope and suggested owner:** equipment-position classification; equipment-slot definition or loadout compatibility rule.

**Lifecycle:** Assign as slot metadata. Equip transactions change occupancy separately from this classification.

**Example:** An equipment rule accepts a shield in off hand.

**Implementation responsibility:** The project implements slot multiplicity, compatibility, sockets, attachment, and occupancy.

## Equipment.State

### Berta.Equipment.State.Broken

**Meaning:** The equipment instance is considered nonfunctional under durability rules.

**Scope and suggested owner:** equipment-instance condition; equipment/durability controller and its instance data.

**Lifecycle:** Update after committed equip, damage, repair, or removal transitions according to the instance condition.

**Example:** A durability controller marks destroyed armor until repaired.

**Related entries:** [Berta.Equipment.State.Equipped](equipment.md#bertaequipmentstateequipped), [Berta.Item.State.Broken](item.md#bertaitemstatebroken).

**Important distinctions:** Item and equipment condition belong to their corresponding representations. Broken equipment can remain equipped until project rules remove it.

**Implementation responsibility:** The project owns durability, assignment, effects, and replicated equipment state.

### Berta.Equipment.State.Equipped

**Meaning:** The equipment instance currently occupies an accepted equipment assignment.

**Scope and suggested owner:** equipment-instance condition; equipment/durability controller and its instance data.

**Lifecycle:** Update after committed equip, damage, repair, or removal transitions according to the instance condition.

**Example:** An equipment controller marks a helmet after a committed equip transaction.

**Related entries:** [Berta.Equipment.State.Broken](equipment.md#bertaequipmentstatebroken), [Berta.Item.State.Broken](item.md#bertaitemstatebroken).

**Important distinctions:** Item and equipment condition belong to their corresponding representations. Broken equipment can remain equipped until project rules remove it.

**Implementation responsibility:** The project owns durability, assignment, effects, and replicated equipment state.
