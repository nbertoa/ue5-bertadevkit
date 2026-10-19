# Damage — 23 tags

Damage tags describe independent dimensions of a damage operation: type, originating mechanism, and resolved hit zone. Put them on damage metadata, a project damage payload, or appropriate Gameplay Effect spec asset tags. They should not ordinarily persist as actor-owned conditions. A hit can compose Fire, Projectile, and Torso without inventing a FireProjectileTorso identifier.

The resolver supplies attribution, resistance, mitigation, shield handling, numeric magnitude, and authoritative application. True does not bypass defenses by itself. Damage.Source.Explosion describes origin while Damage.Type.Special.Explosive describes a damage rule; an explosion may produce several damage types. Hit zones require a project mapping from collision/bone information and may overlap under documented query rules.

A damage tag is neither a Gameplay Attribute nor a condition such as Burning. These classifications do not automatically configure Gameplay Effects, execution calculations, physical damage, friendly-fire rules, or hit detection.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Damage.HitZone

### Berta.Damage.HitZone.Head

**Meaning:** The resolved impact belongs to the target's head region.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A hit resolver maps a skull bone to a head damage policy.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.HitZone.Limb

**Meaning:** The resolved impact belongs to an extremity region.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A hit resolver applies a limb injury rule for an arm strike.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.HitZone.Torso

**Meaning:** The resolved impact belongs to the target's trunk region.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A hit resolver maps chest geometry to torso armor.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.HitZone.WeakPoint

**Meaning:** The resolved impact belongs to a deliberately vulnerable target region.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A boss resolver identifies an exposed core independently of anatomical zones.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

## Damage.Source

### Berta.Damage.Source.Environment

**Meaning:** Damage originates from world hazards rather than an ordinary attack.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A lava volume records environmental provenance.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Source.Explosion

**Meaning:** Damage originates from a detonation mechanism.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A grenade's area damage records explosion provenance.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Source.Hitscan

**Meaning:** Damage originates from an instantaneous trace-based attack mechanism.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A rifle trace records hitscan provenance.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Source.Melee

**Meaning:** Damage originates from a close-contact attack mechanism.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A damage spec records that a sword strike produced the harm.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Source.Projectile

**Meaning:** Damage originates from a moving projectile mechanism.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** An arrow impact records projectile provenance.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Source.StatusEffect

**Meaning:** Damage originates from an ongoing condition.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A burning tick records status-effect provenance separately from fire type.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

## Damage.Type.Elemental

### Berta.Damage.Type.Elemental.Acid

**Meaning:** Damage is classified as corrosive harm.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** An acid projectile selects acid resistance without assuming persistent corrosion.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Type.Elemental.Cold

**Meaning:** Damage is classified as cold exposure.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** An ice projectile selects cold resistance without automatically freezing its target.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Type.Elemental.Fire

**Meaning:** Damage is classified as heat or combustion.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A fireball selects fire resistance without automatically causing burning.

**Related entries:** [Berta.Status.Burning](status.md#bertastatusburning), [GameplayCue.Berta.Status.Burning](gameplay-cue.md#gameplaycuebertastatusburning).

**Important distinctions:** Fire classifies damage, Burning identifies a condition, and the cue identifies cosmetics. None automatically creates the others.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Type.Elemental.Lightning

**Meaning:** Damage is classified as electrical energy.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** An electrical attack selects lightning resistance.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Type.Elemental.Poison

**Meaning:** Damage is classified as toxic harm.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A venom tick selects poison resistance independently of its status duration.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

## Damage.Type.Physical

### Berta.Damage.Type.Physical.Blunt

**Meaning:** Damage is classified as impact or crushing force.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A hammer strike selects the target's blunt-resistance policy.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Type.Physical.Piercing

**Meaning:** Damage is classified as penetrative force.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A spear thrust selects the target's piercing-resistance policy.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Type.Physical.Slashing

**Meaning:** Damage is classified as a cutting force.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A sword cut selects the target's slashing-resistance policy.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

## Damage.Type.Special

### Berta.Damage.Type.Special.Arcane

**Meaning:** Damage is classified as magical energy outside the elemental categories.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A spell selects a project-defined arcane resistance.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Type.Special.Drowning

**Meaning:** Damage is classified as harm from inability to breathe in water.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** An oxygen system labels a suffocation tick while submerged.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Type.Special.Explosive

**Meaning:** Damage is classified as blast-related harm.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A blast resolver selects explosive protection while separately recording its source.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Type.Special.Fall

**Meaning:** Damage is classified as harm from a fall.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A landing resolver labels impact damage from excessive descent.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.

### Berta.Damage.Type.Special.True

**Meaning:** Damage is intended for the project's explicit defense-bypass classification.

**Scope and suggested owner:** damage-operation classification; damage resolver metadata, payload, or Gameplay Effect spec asset classification.

**Lifecycle:** Assign to the damage operation when type, origin, or contact zone is known. Use for resolution/filtering; it is not a timed actor condition.

**Example:** A resolver deliberately applies the game's true-damage rules.

**Implementation responsibility:** The project maps hit data and implements numeric damage, resistance, mitigation, and authoritative application.
