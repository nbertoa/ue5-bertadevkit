# Progression — 6 tags

Progression.Event labels committed changes to experience, levels, skills, or perks. Progression.State labels current conditions derived from the progression record. The progression authority owns the numbers, caps, unlock rules, persistence, and replication; tags do not represent the actual level or experience amount.

Emit events only after a successful transaction and include the affected participant and values or unlock identifiers in project payload data. Recompute MaxLevel when the cap changes, and SkillPointAvailable when points are awarded or spent. These state tags may coexist. SkillUnlocked and PerkUnlocked do not automatically grant Gameplay Abilities; a project can explicitly connect progression records to GAS grants.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Progression.Event

### Berta.Progression.Event.ExperienceGained

**Meaning:** A progression transaction awarded experience.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A progression controller reports the accepted amount and source.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Progression.Event.LevelGained

**Meaning:** A progression transaction advanced an entity to a new level.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A progression controller reports the old and new level.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Progression.Event.PerkUnlocked

**Meaning:** A progression transaction made a perk available.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A perk system reports a newly acquired passive option.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Progression.Event.SkillUnlocked

**Meaning:** A progression transaction made a skill available.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A skill tree reports a newly committed unlock.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

## Progression.State

### Berta.Progression.State.MaxLevel

**Meaning:** The progression model currently considers the entity at its level cap.

**Scope and suggested owner:** progression-record condition; progression authority and participant progression record.

**Lifecycle:** Recompute after level/cap or spendable-point changes; clear when the corresponding predicate becomes false.

**Example:** A progression UI hides the next-level requirement.

**Implementation responsibility:** The project stores numeric progression and validates unlock/spend transactions.

### Berta.Progression.State.SkillPointAvailable

**Meaning:** The entity currently has at least one spendable skill point.

**Scope and suggested owner:** progression-record condition; progression authority and participant progression record.

**Lifecycle:** Recompute after level/cap or spendable-point changes; clear when the corresponding predicate becomes false.

**Example:** A skill-tree UI displays an unspent-point indicator.

**Implementation responsibility:** The project stores numeric progression and validates unlock/spend transactions.
