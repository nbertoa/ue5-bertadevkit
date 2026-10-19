# Ability — 52 tags

Ability identifiers describe what a Gameplay Ability does. Assign them to its **Asset Tags (Default AbilityTags)** defaults in UE 5.8. They identify the ability asset/spec for selection, cancellation, blocking relationships, and diagnostics; they do not become ASC-owned tags merely because the ability is granted or active.

**Activation Owned Tags** instead describe tags granted to the owner while that ability is active. Choose those only when the ability truly owns that lifetime. A movement controller may need to maintain airborne state after a jump ability has ended. Replication of activation-owned tags follows GAS configuration; naming a tag does not enable replication.

The explicitly declared Attack identifier is also the parent of Light, Heavy, Charged, and Ranged. A hierarchical query for Attack can select those children; an exact query distinguishes them. A project can use more than one identifier on an ability. Neither this catalog nor these identifiers grant abilities, bind input, or decide activation eligibility.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Ability.Combat

### Berta.Ability.Combat.Aim

**Meaning:** Requests deliberate weapon or attack aiming.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify the ability that enters an aiming workflow.

**Related entries:** [Berta.Camera.Mode.Aiming](camera.md#bertacameramodeaiming), [Berta.Cooldown.Combat.Aim](cooldown.md#bertacooldowncombataim), [Berta.State.Combat.Aiming](state.md#bertastatecombataiming).

**Important distinctions:** Camera policy, character action condition, and ability identity are independent; the project coordinates them explicitly.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Attack

**Meaning:** Identifies a general offensive attack capability and its specialized descendants.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Query all attack abilities through this intentional explicit parent.

**Related entries:** [Berta.Ability.Combat.Attack.Charged](ability.md#bertaabilitycombatattackcharged), [Berta.Ability.Combat.Attack.Heavy](ability.md#bertaabilitycombatattackheavy), [Berta.Ability.Combat.Attack.Light](ability.md#bertaabilitycombatattacklight), [Berta.Ability.Combat.Attack.Ranged](ability.md#bertaabilitycombatattackranged), [Berta.Cooldown.Combat.Attack](cooldown.md#bertacooldowncombatattack), [GameplayCue.Berta.Combat.Attack](gameplay-cue.md#gameplaycuebertacombatattack).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them. Attack is an approved explicit parent. Hierarchical queries match its variants; owning the parent does not grant any child.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Attack.Charged

**Meaning:** Builds charge before releasing an attack.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify an attack whose strength depends on charge time.

**Related entries:** [Berta.Ability.Combat.Attack](ability.md#bertaabilitycombatattack), [Berta.Ability.Combat.Attack.Heavy](ability.md#bertaabilitycombatattackheavy), [Berta.Ability.Combat.Attack.Light](ability.md#bertaabilitycombatattacklight), [Berta.Ability.Combat.Attack.Ranged](ability.md#bertaabilitycombatattackranged), [Berta.Cooldown.Combat.Attack.Charged](cooldown.md#bertacooldowncombatattackcharged).

**Important distinctions:** Attack is an approved explicit parent. Hierarchical queries match its variants; owning the parent does not grant any child.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Attack.Heavy

**Meaning:** Performs an attack variant with greater commitment or impact.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select the heavy strike for a held combat action.

**Related entries:** [Berta.Ability.Combat.Attack](ability.md#bertaabilitycombatattack), [Berta.Ability.Combat.Attack.Charged](ability.md#bertaabilitycombatattackcharged), [Berta.Ability.Combat.Attack.Light](ability.md#bertaabilitycombatattacklight), [Berta.Ability.Combat.Attack.Ranged](ability.md#bertaabilitycombatattackranged), [Berta.Cooldown.Combat.Attack.Heavy](cooldown.md#bertacooldowncombatattackheavy).

**Important distinctions:** Attack is an approved explicit parent. Hierarchical queries match its variants; owning the parent does not grant any child.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Attack.Light

**Meaning:** Performs a relatively quick or low-commitment attack variant.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify the light strike in a melee ability set.

**Related entries:** [Berta.Ability.Combat.Attack](ability.md#bertaabilitycombatattack), [Berta.Ability.Combat.Attack.Charged](ability.md#bertaabilitycombatattackcharged), [Berta.Ability.Combat.Attack.Heavy](ability.md#bertaabilitycombatattackheavy), [Berta.Ability.Combat.Attack.Ranged](ability.md#bertaabilitycombatattackranged), [Berta.Cooldown.Combat.Attack.Light](cooldown.md#bertacooldowncombatattacklight).

**Important distinctions:** Attack is an approved explicit parent. Hierarchical queries match its variants; owning the parent does not grant any child.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Attack.Ranged

**Meaning:** Performs an attack intended to reach a distant target.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Find a ranged attack without requiring a particular weapon class.

**Related entries:** [Berta.Ability.Combat.Attack](ability.md#bertaabilitycombatattack), [Berta.Ability.Combat.Attack.Charged](ability.md#bertaabilitycombatattackcharged), [Berta.Ability.Combat.Attack.Heavy](ability.md#bertaabilitycombatattackheavy), [Berta.Ability.Combat.Attack.Light](ability.md#bertaabilitycombatattacklight), [Berta.Cooldown.Combat.Attack.Ranged](cooldown.md#bertacooldowncombatattackranged).

**Important distinctions:** Attack is an approved explicit parent. Hierarchical queries match its variants; owning the parent does not grant any child.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Block

**Meaning:** Attempts to mitigate incoming attacks with a defensive guard.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify a maintained guard ability.

**Related entries:** [Berta.Cooldown.Combat.Block](cooldown.md#bertacooldowncombatblock), [GameplayCue.Berta.Combat.Block](gameplay-cue.md#gameplaycuebertacombatblock).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.CounterAttack

**Meaning:** Attacks in response to a qualifying opposing action.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select a riposte ability after a successful parry.

**Related entries:** [Berta.Cooldown.Combat.CounterAttack](cooldown.md#bertacooldowncombatcounterattack).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Dodge

**Meaning:** Performs a deliberate evasion maneuver.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select an evasion ability after direction input.

**Related entries:** [Berta.Cooldown.Combat.Dodge](cooldown.md#bertacooldowncombatdodge).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Equip

**Meaning:** Places equipment into an active usable arrangement.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Find the ability that draws a selected weapon.

**Related entries:** [Berta.Cooldown.Combat.Equip](cooldown.md#bertacooldowncombatequip).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Execute

**Meaning:** Performs a finishing action against a qualifying target.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select a finisher after checking target vulnerability.

**Related entries:** [Berta.Cooldown.Combat.Execute](cooldown.md#bertacooldowncombatexecute).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Fire

**Meaning:** Discharges a weapon or comparable firing mechanism.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select the trigger-driven firing ability.

**Related entries:** [Berta.Cooldown.Combat.Fire](cooldown.md#bertacooldowncombatfire), [GameplayCue.Berta.Combat.Fire](gameplay-cue.md#gameplaycuebertacombatfire).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.LockOn

**Meaning:** Acquires or maintains a focused combat target.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify an ability that requests target lock.

**Related entries:** [Berta.Cooldown.Combat.LockOn](cooldown.md#bertacooldowncombatlockon).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Parry

**Meaning:** Attempts a timed deflection of an incoming attack.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select a short defensive timing-window ability.

**Related entries:** [Berta.Cooldown.Combat.Parry](cooldown.md#bertacooldowncombatparry), [GameplayCue.Berta.Combat.Parry](gameplay-cue.md#gameplaycuebertacombatparry).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Reload

**Meaning:** Transfers ammunition into a weapon's usable supply.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Find the reload ability before checking ammunition availability.

**Related entries:** [Berta.Cooldown.Combat.Reload](cooldown.md#bertacooldowncombatreload), [Berta.Event.Combat.ReloadCompleted](event.md#bertaeventcombatreloadcompleted), [Berta.State.Combat.Reloading](state.md#bertastatecombatreloading), [Berta.Weapon.State.Reloading](weapon.md#bertaweaponstatereloading), [GameplayCue.Berta.Combat.Reload](gameplay-cue.md#gameplaycuebertacombatreload).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them. These describe weapon condition, character condition, capability identity, and completed occurrence respectively. Their owners and lifetimes can differ.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Roll

**Meaning:** Performs a rolling movement maneuver.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Distinguish a roll ability from a non-rolling sidestep.

**Related entries:** [Berta.Cooldown.Combat.Roll](cooldown.md#bertacooldowncombatroll).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Throw

**Meaning:** Propels a held object or throwable toward a destination.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify the grenade throwing capability.

**Related entries:** [Berta.Cooldown.Combat.Throw](cooldown.md#bertacooldowncombatthrow).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Combat.Unequip

**Meaning:** Removes equipment from its active usable arrangement.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify the capability used to stow a drawn weapon.

**Related entries:** [Berta.Cooldown.Combat.Unequip](cooldown.md#bertacooldowncombatunequip).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

## Ability.Interaction

### Berta.Ability.Interaction.Close

**Meaning:** Requests an object transition into a closed configuration.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify an ability used to close a hatch.

**Related entries:** [Berta.Cooldown.Interaction.Close](cooldown.md#bertacooldowninteractionclose).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Interaction.Drop

**Meaning:** Releases a carried or inventory object into the world.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Choose the capability used to drop a held crate.

**Related entries:** [Berta.Cooldown.Interaction.Drop](cooldown.md#bertacooldowninteractiondrop).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Interaction.Inspect

**Meaning:** Examines a target for information without assuming acquisition.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify a capability used to examine a clue.

**Related entries:** [Berta.Cooldown.Interaction.Inspect](cooldown.md#bertacooldowninteractioninspect).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Interaction.Interact

**Meaning:** Initiates a general interaction with a target.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select a contextual interaction ability.

**Related entries:** [Berta.Cooldown.Interaction.Interact](cooldown.md#bertacooldowninteractioninteract).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Interaction.Open

**Meaning:** Requests an object transition into an open configuration.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select an ability used to open a door.

**Related entries:** [Berta.Cooldown.Interaction.Open](cooldown.md#bertacooldowninteractionopen).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Interaction.Pickup

**Meaning:** Acquires a world object into a carried or inventory representation.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify an ability used to collect a dropped item.

**Related entries:** [Berta.Cooldown.Interaction.Pickup](cooldown.md#bertacooldowninteractionpickup), [GameplayCue.Berta.Interaction.Pickup](gameplay-cue.md#gameplaycuebertainteractionpickup).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Interaction.Revive

**Meaning:** Attempts to restore a downed target to active life.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select an ally recovery ability after eligibility checks.

**Related entries:** [Berta.Cooldown.Interaction.Revive](cooldown.md#bertacooldowninteractionrevive).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Interaction.Talk

**Meaning:** Initiates conversation with a dialogue participant.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select the ability that starts an NPC conversation.

**Related entries:** [Berta.Cooldown.Interaction.Talk](cooldown.md#bertacooldowninteractiontalk).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Interaction.Use

**Meaning:** Invokes the intended function of an object.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify an ability that consumes a selected potion.

**Related entries:** [Berta.Cooldown.Interaction.Use](cooldown.md#bertacooldowninteractionuse).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

## Ability.Movement

### Berta.Ability.Movement.Climb

**Meaning:** Traverses a climbable surface using a climbing capability.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select a ladder or wall climbing ability.

**Related entries:** [Berta.Cooldown.Movement.Climb](cooldown.md#bertacooldownmovementclimb).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.Crouch

**Meaning:** Requests a reduced-height locomotion posture.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify the ability responsible for requesting crouch.

**Related entries:** [Berta.Cooldown.Movement.Crouch](cooldown.md#bertacooldownmovementcrouch).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.Dash

**Meaning:** Performs a short directed burst of movement.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select a dash ability for an evasive input.

**Related entries:** [Berta.Cooldown.Movement.Dash](cooldown.md#bertacooldownmovementdash), [GameplayCue.Berta.Movement.Dash](gameplay-cue.md#gameplaycuebertamovementdash).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.DoubleJump

**Meaning:** Provides an additional jump while already airborne.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select an aerial jump ability separately from the initial takeoff.

**Related entries:** [Berta.Cooldown.Movement.DoubleJump](cooldown.md#bertacooldownmovementdoublejump).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.Glide

**Meaning:** Controls airborne descent with a gliding capability.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify a deployable glider ability.

**Related entries:** [Berta.Cooldown.Movement.Glide](cooldown.md#bertacooldownmovementglide).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.Grapple

**Meaning:** Uses an anchor or tether to reposition the character.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify a grapple ability for a valid anchor target.

**Related entries:** [Berta.Cooldown.Movement.Grapple](cooldown.md#bertacooldownmovementgrapple).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.Jump

**Meaning:** Initiates a deliberate takeoff from a supporting surface.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify GA_Jump when searching an ASC's granted abilities.

**Related entries:** [Berta.Cooldown.Movement.Jump](cooldown.md#bertacooldownmovementjump), [Berta.Event.Movement.Jumped](event.md#bertaeventmovementjumped), [Berta.Event.Movement.Landed](event.md#bertaeventmovementlanded), [Berta.Input.Movement.Jump](input.md#bertainputmovementjump), [Berta.State.Movement.Falling](state.md#bertastatemovementfalling), [Berta.State.Movement.InAir](state.md#bertastatemovementinair), [Berta.State.Movement.Jumping](state.md#bertastatemovementjumping), [GameplayCue.Berta.Movement.Jump](gameplay-cue.md#gameplaycuebertamovementjump), [GameplayCue.Berta.Movement.Land](gameplay-cue.md#gameplaycuebertamovementland).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them. Jump ability identity, movement intervals, transition notifications, optional cooldown, and cosmetic cues are separate contracts. See the manual GA_Jump walkthrough in the integration guide. The input label does not bind an Enhanced Input Action or select/grant an ability; routing is explicit project configuration.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.Mantle

**Meaning:** Pulls the character onto the top of a ledge.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Choose a ledge ascent ability after a traversal query.

**Related entries:** [Berta.Cooldown.Movement.Mantle](cooldown.md#bertacooldownmovementmantle).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.Slide

**Meaning:** Starts a low-profile sliding traversal action.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify a slide capability that requires sufficient speed.

**Related entries:** [Berta.Cooldown.Movement.Slide](cooldown.md#bertacooldownmovementslide).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.Sprint

**Meaning:** Requests sustained faster locomotion under project rules.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Find the sprint ability before checking its stamina cost.

**Related entries:** [Berta.Cooldown.Movement.Sprint](cooldown.md#bertacooldownmovementsprint), [GameplayCue.Berta.Movement.Sprint](gameplay-cue.md#gameplaycuebertamovementsprint).

**Important distinctions:** The ability identifier and cosmetic cue have independent roles; no automatic invocation connects them.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.Swim

**Meaning:** Provides deliberate movement through water.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select an ability that initiates a swimming action.

**Related entries:** [Berta.Cooldown.Movement.Swim](cooldown.md#bertacooldownmovementswim).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.Teleport

**Meaning:** Repositions to a destination without continuous traversal.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select a blink ability after destination validation.

**Related entries:** [Berta.Cooldown.Movement.Teleport](cooldown.md#bertacooldownmovementteleport).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.Vault

**Meaning:** Traverses over an obstacle without remaining on it.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify the capability used to cross a low fence.

**Related entries:** [Berta.Cooldown.Movement.Vault](cooldown.md#bertacooldownmovementvault).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Movement.WallRun

**Meaning:** Moves along a wall while supported by a wall-running mechanic.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Choose a wall traversal ability after validating the surface.

**Related entries:** [Berta.Cooldown.Movement.WallRun](cooldown.md#bertacooldownmovementwallrun).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

## Ability.Passive

### Berta.Ability.Passive.Aura

**Meaning:** Identifies a passive capability that influences an area or nearby recipients.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select a support aura passive in an ability loadout.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Passive.AutoReload

**Meaning:** Identifies a passive capability that reloads under configured conditions.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Find the passive that reloads a holstered weapon.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Passive.AutoRespawn

**Meaning:** Identifies a passive capability that requests respawning under configured conditions.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify a respawn passive before applying game-mode eligibility rules.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Passive.Regeneration

**Meaning:** Identifies a passive capability that periodically restores a resource.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Find a granted regeneration passive without assuming it is currently restoring health.

**Related entries:** [Berta.Status.Regenerating](status.md#bertastatusregenerating).

**Important distinctions:** A granted passive can exist while restoration is suspended. Regenerating describes the currently maintained condition rather than passive identity.

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

## Ability.Support

### Berta.Ability.Support.Buff

**Meaning:** Provides a capability intended to confer a beneficial modification.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify a rally ability that improves ally performance.

**Related entries:** [Berta.Cooldown.Support.Buff](cooldown.md#bertacooldownsupportbuff).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Support.Cleanse

**Meaning:** Removes adverse conditions selected by the project's cleansing rules.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify an ability that removes poison according to project rules.

**Related entries:** [Berta.Cooldown.Support.Cleanse](cooldown.md#bertacooldownsupportcleanse).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Support.Debuff

**Meaning:** Provides a capability intended to impair a recipient.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select a weakening ability for an enemy target.

**Related entries:** [Berta.Cooldown.Support.Debuff](cooldown.md#bertacooldownsupportdebuff).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Support.Deploy

**Meaning:** Places a functional device or construct into the world.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify an ability that places a support turret.

**Related entries:** [Berta.Cooldown.Support.Deploy](cooldown.md#bertacooldownsupportdeploy).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Support.Heal

**Meaning:** Provides a capability intended to restore a recipient's health.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Identify a healing spell before validating its target.

**Related entries:** [Berta.Cooldown.Support.Heal](cooldown.md#bertacooldownsupportheal).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Support.Shield

**Meaning:** Provides a capability intended to create protective shielding.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select an ability that applies a protective barrier.

**Related entries:** [Berta.Cooldown.Support.Shield](cooldown.md#bertacooldownsupportshield).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.

### Berta.Ability.Support.Summon

**Meaning:** Creates or calls a supported entity into play.

**Scope and suggested owner:** Gameplay Ability identifier; Gameplay Ability definition (Asset Tags), selected through a granted spec on an ASC.

**Lifecycle:** Assign in ability defaults. Retain as identity across activations; granting or ending the ability does not add or remove this identifier on the actor.

**Example:** Select the ability used to summon a companion.

**Related entries:** [Berta.Cooldown.Support.Summon](cooldown.md#bertacooldownsupportsummon).

**Implementation responsibility:** The project grants the ability, configures activation and input, and implements its action and any separately owned state.
