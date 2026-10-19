# GameplayCue — 20 tags

All cue identifiers use the native GameplayCue root followed by Berta. GameplayCue.Berta belongs to Unreal's cue hierarchy; Berta.GameplayCue does not. Configure matching Gameplay Cue Notify assets or actor cue handlers and explicitly trigger them through native GAS. The catalog registers identifiers only and includes no cue assets, effects, sounds, or particle systems.

Gameplay Effects can declare cues for application and active lifetimes; explicit ASC cue execution is also available. Distinguish executed one-shot feedback from active cues with add/remove lifetimes. A looping presentation must clean up when the owning effect or condition ends. The actual cue implementation decides how parameters such as location, normal, and source context affect presentation.

Native GAS cue transport and prediction depend on the invocation path, authority, ASC networking, and replication configuration. Cues are cosmetic; do not use their receipt as reliable authoritative damage, inventory, or life state. Dedicated servers need no visual output for gameplay correctness. Verify project cue asset discovery and client behavior independently of tag registration.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## GameplayCue.Combat

### GameplayCue.Berta.Combat.Attack

**Meaning:** Identifies cosmetic feedback associated with an attack action.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** An attack implementation executes a swing presentation cue.

**Related entries:** [Berta.Ability.Combat.Attack](ability.md#bertaabilitycombatattack).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Combat.Block

**Meaning:** Identifies cosmetic feedback associated with a defensive block.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A combat resolver executes a shield impact cue.

**Related entries:** [Berta.Ability.Combat.Block](ability.md#bertaabilitycombatblock).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Combat.CriticalHit

**Meaning:** Identifies cosmetic feedback associated with a critical result.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A combat resolver executes distinct critical-hit presentation.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Combat.Fire

**Meaning:** Identifies cosmetic feedback associated with weapon discharge.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A weapon implementation executes a muzzle-flash cue.

**Related entries:** [Berta.Ability.Combat.Fire](ability.md#bertaabilitycombatfire).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Combat.Hit

**Meaning:** Identifies cosmetic feedback associated with confirmed contact.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A hit resolver executes an impact cue with surface data.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Combat.Parry

**Meaning:** Identifies cosmetic feedback associated with a successful timed deflection.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A combat resolver executes parry sparks and sound.

**Related entries:** [Berta.Ability.Combat.Parry](ability.md#bertaabilitycombatparry).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Combat.Reload

**Meaning:** Identifies cosmetic feedback associated with ammunition replenishment.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A reload implementation executes a cosmetic magazine-handling cue.

**Related entries:** [Berta.Ability.Combat.Reload](ability.md#bertaabilitycombatreload).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

## GameplayCue.Environment

### GameplayCue.Berta.Environment.Footstep

**Meaning:** Identifies cosmetic feedback associated with a foot contacting a surface.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A footstep producer supplies location and surface data to an impact cue.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Environment.Splash

**Meaning:** Identifies cosmetic feedback associated with a water impact or disturbance.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A water-contact producer executes a splash cue.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

## GameplayCue.Interaction

### GameplayCue.Berta.Interaction.Pickup

**Meaning:** Identifies cosmetic feedback associated with acquiring an object.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A successful pickup implementation executes a collection cue.

**Related entries:** [Berta.Ability.Interaction.Pickup](ability.md#bertaabilityinteractionpickup).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

## GameplayCue.Movement

### GameplayCue.Berta.Movement.Dash

**Meaning:** Identifies cosmetic feedback associated with a dash.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A dash implementation executes a short motion trail cue.

**Related entries:** [Berta.Ability.Movement.Dash](ability.md#bertaabilitymovementdash).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Movement.Dodge

**Meaning:** Identifies cosmetic feedback associated with evasion.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** An evasion implementation executes a cosmetic whoosh cue.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Movement.Jump

**Meaning:** Identifies cosmetic feedback associated with jump takeoff.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A jump implementation explicitly executes a takeoff sound cue.

**Related entries:** [Berta.Ability.Movement.Jump](ability.md#bertaabilitymovementjump), [Berta.Cooldown.Movement.Jump](cooldown.md#bertacooldownmovementjump), [Berta.Event.Movement.Jumped](event.md#bertaeventmovementjumped), [Berta.Event.Movement.Landed](event.md#bertaeventmovementlanded), [Berta.State.Movement.Falling](state.md#bertastatemovementfalling), [Berta.State.Movement.InAir](state.md#bertastatemovementinair), [Berta.State.Movement.Jumping](state.md#bertastatemovementjumping), [GameplayCue.Berta.Movement.Land](gameplay-cue.md#gameplaycuebertamovementland).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them. Jump ability identity, movement intervals, transition notifications, optional cooldown, and cosmetic cues are separate contracts. See the manual GA_Jump walkthrough in the integration guide.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Movement.Land

**Meaning:** Identifies cosmetic feedback associated with landing.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A landing implementation explicitly executes a dust or impact cue.

**Related entries:** [Berta.Ability.Movement.Jump](ability.md#bertaabilitymovementjump), [Berta.Cooldown.Movement.Jump](cooldown.md#bertacooldownmovementjump), [Berta.Event.Movement.Jumped](event.md#bertaeventmovementjumped), [Berta.Event.Movement.Landed](event.md#bertaeventmovementlanded), [Berta.State.Movement.Falling](state.md#bertastatemovementfalling), [Berta.State.Movement.InAir](state.md#bertastatemovementinair), [Berta.State.Movement.Jumping](state.md#bertastatemovementjumping), [GameplayCue.Berta.Movement.Jump](gameplay-cue.md#gameplaycuebertamovementjump).

**Important distinctions:** Jump ability identity, movement intervals, transition notifications, optional cooldown, and cosmetic cues are separate contracts. See the manual GA_Jump walkthrough in the integration guide.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Movement.Sprint

**Meaning:** Identifies cosmetic feedback associated with sprinting.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A sprint effect maintains a cosmetic trail cue.

**Related entries:** [Berta.Ability.Movement.Sprint](ability.md#bertaabilitymovementsprint).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

## GameplayCue.Status

### GameplayCue.Berta.Status.Burning

**Meaning:** Identifies cosmetic feedback associated with a burning condition.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** An active burning effect maintains flame presentation.

**Related entries:** [Berta.Damage.Type.Elemental.Fire](damage.md#bertadamagetypeelementalfire), [Berta.Status.Burning](status.md#bertastatusburning).

**Important distinctions:** The condition remains authoritative even if its cosmetic cue is absent, suppressed, or not received. Fire classifies damage, Burning identifies a condition, and the cue identifies cosmetics. None automatically creates the others.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Status.Frozen

**Meaning:** Identifies cosmetic feedback associated with a frozen condition.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A freezing effect maintains ice-shell presentation.

**Related entries:** [Berta.Status.Frozen](status.md#bertastatusfrozen).

**Important distinctions:** The condition remains authoritative even if its cosmetic cue is absent, suppressed, or not received.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Status.Healing

**Meaning:** Identifies cosmetic feedback associated with restorative activity.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A healing effect executes a restorative burst cue.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Status.Poisoned

**Meaning:** Identifies cosmetic feedback associated with a poison condition.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** An active poison effect maintains toxic presentation.

**Related entries:** [Berta.Status.Poisoned](status.md#bertastatuspoisoned).

**Important distinctions:** The condition remains authoritative even if its cosmetic cue is absent, suppressed, or not received.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.

### GameplayCue.Berta.Status.Shielded

**Meaning:** Identifies cosmetic feedback associated with protective shielding.

**Scope and suggested owner:** cosmetic Gameplay Cue identifier; Gameplay Cue Notify/actor handler, triggered explicitly by GAS effects or cue producers.

**Lifecycle:** Execute for a one-shot occurrence, or add/remove for a deliberately maintained active cue. Match cleanup to the chosen cue lifetime.

**Example:** A shield effect maintains barrier presentation.

**Related entries:** [Berta.Status.Shielded](status.md#bertastatusshielded).

**Important distinctions:** The condition remains authoritative even if its cosmetic cue is absent, suppressed, or not received.

**Implementation responsibility:** The project supplies cue assets/handlers, parameters, discovery, invocation, prediction, and network behavior.
