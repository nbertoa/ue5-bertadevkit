# Status — 23 tags

Status tags identify conditions affecting a recipient, including beneficial, adverse, and contextual conditions. Status is not a synonym for debuff: Hasted, Regenerating, Shielded, Wet, and Marked can have different value depending on the game. Put ownership on the condition provider, often an active Gameplay Effect on the recipient ASC, but a non-GAS condition system can expose the same vocabulary.

A tag has no duration, magnitude, stack count, or reaction table. The project chooses stacking and expiry and must not remove a condition while another valid source still maintains it. A single present tag cannot explain how many sources contribute; query the actual effects when that matters. Neither Burning nor Frozen automatically changes attributes or movement.

Damage classification, condition state, and cosmetic Gameplay Cues are independent. A fire hit may cause no burning, and burning may remain after the original hit. Classify the original hit, maintain the condition through its owner, and trigger cosmetics separately.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Status

### Berta.Status.Bleeding

**Meaning:** An ongoing blood-loss condition affects the recipient.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A wound effect maintains bleeding until treated or expired.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Blinded

**Meaning:** The recipient suffers impaired visual capability.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** An AI perception policy reduces visual acquisition during a blind effect.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Burning

**Meaning:** An ongoing combustion-related condition affects the recipient.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A fire effect maintains this condition while applying periodic damage.

**Related entries:** [Berta.Damage.Type.Elemental.Fire](damage.md#bertadamagetypeelementalfire), [GameplayCue.Berta.Status.Burning](gameplay-cue.md#gameplaycuebertastatusburning).

**Important distinctions:** The condition remains authoritative even if its cosmetic cue is absent, suppressed, or not received. Fire classifies damage, Burning identifies a condition, and the cue identifies cosmetics. None automatically creates the others.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Chilled

**Meaning:** A cold-related condition affects the recipient short of the game's frozen threshold.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Build a frost interaction without equating chilled with frozen.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Corroded

**Meaning:** A corrosive condition affects the recipient or its protection.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** An acid effect identifies armor corrosion.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Cursed

**Meaning:** A supernatural affliction affects the recipient.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A curse effect marks a target until the curse is lifted.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Deafened

**Meaning:** The recipient suffers impaired hearing capability.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** An AI system ignores qualifying sound stimuli during deafness.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Diseased

**Meaning:** A disease condition affects the recipient.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A disease system queries infected recipients for treatment eligibility.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Exhausted

**Meaning:** The recipient suffers a fatigue-related condition.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Restrict sprint recovery under the game's exhaustion rules.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Frozen

**Meaning:** A freezing condition severely restricts the recipient under project rules.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** An ice effect marks a frozen target without requiring a particular stun implementation.

**Related entries:** [GameplayCue.Berta.Status.Frozen](gameplay-cue.md#gameplaycuebertastatusfrozen).

**Important distinctions:** The condition remains authoritative even if its cosmetic cue is absent, suppressed, or not received.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Hasted

**Meaning:** The recipient has a condition intended to increase speed.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A haste effect modifies action or movement speed as configured.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Marked

**Meaning:** The recipient is designated for tracking or a follow-up interaction.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A hunter mark makes a target eligible for a bonus effect.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Oiled

**Meaning:** The recipient is coated with a combustible or slippery oil.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A trap marks an oiled target before an ignition interaction.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Overheated

**Meaning:** The recipient has exceeded its operational heat tolerance.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A weapon status blocks firing until a cooling policy clears it.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Poisoned

**Meaning:** A toxic condition affects the recipient.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A poison effect uses its own duration and stacking policy.

**Related entries:** [GameplayCue.Berta.Status.Poisoned](gameplay-cue.md#gameplaycuebertastatuspoisoned).

**Important distinctions:** The condition remains authoritative even if its cosmetic cue is absent, suppressed, or not received.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Regenerating

**Meaning:** The recipient is currently under a resource-restoring condition.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Show recovery feedback while an active regeneration effect persists.

**Related entries:** [Berta.Ability.Passive.Regeneration](ability.md#bertaabilitypassiveregeneration).

**Important distinctions:** A granted passive can exist while restoration is suspended. Regenerating describes the currently maintained condition rather than passive identity.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Shielded

**Meaning:** The recipient currently benefits from a protective shield condition.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Display barrier feedback without treating the tag as a numeric shield amount.

**Related entries:** [Berta.Resource.Shield](resource.md#bertaresourceshield), [GameplayCue.Berta.Status.Shielded](gameplay-cue.md#gameplaycuebertastatusshielded).

**Important distinctions:** The condition remains authoritative even if its cosmetic cue is absent, suppressed, or not received. Shielded is a protective condition; Resource.Shield is a numeric-provider classification. The project decides whether a positive reserve implies that condition.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Shocked

**Meaning:** An electrical condition affects the recipient.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** An electrical effect marks a target for a follow-up reaction.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Slowed

**Meaning:** The recipient has a condition intended to reduce speed.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A slow effect reduces a movement multiplier without declaring walking locomotion.

**Related entries:** [Berta.State.Movement.Walking](state.md#bertastatemovementwalking).

**Important distinctions:** Slowed is a speed-affecting condition; Walking is the selected locomotion regime. A slowed actor can still run or stand still.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Suppressed

**Meaning:** The recipient suffers a suppression condition affecting combat effectiveness.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A suppression system modifies aiming behavior during incoming fire.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Vulnerable

**Meaning:** The recipient currently has increased susceptibility under project rules.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A damage policy checks exposure to a configured damage multiplier.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Weakened

**Meaning:** The recipient has a condition intended to reduce effectiveness or strength.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A debuff reduces outgoing damage through explicit modifiers.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.Status.Wet

**Meaning:** The recipient is coated with or saturated by water.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** A water interaction marks a target for project-defined electrical reactions.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.
