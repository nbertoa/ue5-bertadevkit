# Cooldown — 48 tags

Cooldown identifiers name restrictions a project may enforce through native GAS. A typical ability checks tags returned by its configured cooldown Gameplay Effect, applies that duration effect on successful commitment, and becomes eligible when the restriction expires. Configure duration, tag granting, activation checks, prediction, and replication deliberately. A registered or locally stored cooldown tag alone has no timing or enforcement behavior.

In UE 5.8 use the cooldown Gameplay Effect and its **Target Tags** component for granted owner tags. Verify the ability's GetCooldownTags path observes those tags; custom ability overrides must preserve that contract. No custom Berta cooldown manager is supplied. Shared cooldowns are possible by configuring multiple abilities to check the same tag.

Attack is intentionally explicit as well as a parent of its variants. A broad hierarchical restriction can affect all variants; choose exact or hierarchical behavior intentionally. Do not assign every child merely because the parent is present. Passive abilities have no approved cooldown entries in this release.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Cooldown.Combat

### Berta.Cooldown.Combat.Aim

**Meaning:** Identifies a cooldown restriction associated with the Aim capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Aim ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Aim](ability.md#bertaabilitycombataim).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Attack

**Meaning:** Identifies a cooldown restriction associated with the Attack capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Attack ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Attack](ability.md#bertaabilitycombatattack), [Berta.Cooldown.Combat.Attack.Charged](cooldown.md#bertacooldowncombatattackcharged), [Berta.Cooldown.Combat.Attack.Heavy](cooldown.md#bertacooldowncombatattackheavy), [Berta.Cooldown.Combat.Attack.Light](cooldown.md#bertacooldowncombatattacklight), [Berta.Cooldown.Combat.Attack.Ranged](cooldown.md#bertacooldowncombatattackranged).

**Important distinctions:** Attack is an explicit parent restriction identifier. Confirm the intended hierarchical cooldown checks before mixing shared and variant restrictions.

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Attack.Charged

**Meaning:** Identifies a cooldown restriction associated with the Attack Charged capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Attack Charged ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Attack.Charged](ability.md#bertaabilitycombatattackcharged), [Berta.Cooldown.Combat.Attack](cooldown.md#bertacooldowncombatattack), [Berta.Cooldown.Combat.Attack.Heavy](cooldown.md#bertacooldowncombatattackheavy), [Berta.Cooldown.Combat.Attack.Light](cooldown.md#bertacooldowncombatattacklight), [Berta.Cooldown.Combat.Attack.Ranged](cooldown.md#bertacooldowncombatattackranged).

**Important distinctions:** Attack is an explicit parent restriction identifier. Confirm the intended hierarchical cooldown checks before mixing shared and variant restrictions.

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Attack.Heavy

**Meaning:** Identifies a cooldown restriction associated with the Attack Heavy capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Attack Heavy ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Attack.Heavy](ability.md#bertaabilitycombatattackheavy), [Berta.Cooldown.Combat.Attack](cooldown.md#bertacooldowncombatattack), [Berta.Cooldown.Combat.Attack.Charged](cooldown.md#bertacooldowncombatattackcharged), [Berta.Cooldown.Combat.Attack.Light](cooldown.md#bertacooldowncombatattacklight), [Berta.Cooldown.Combat.Attack.Ranged](cooldown.md#bertacooldowncombatattackranged).

**Important distinctions:** Attack is an explicit parent restriction identifier. Confirm the intended hierarchical cooldown checks before mixing shared and variant restrictions.

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Attack.Light

**Meaning:** Identifies a cooldown restriction associated with the Attack Light capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Attack Light ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Attack.Light](ability.md#bertaabilitycombatattacklight), [Berta.Cooldown.Combat.Attack](cooldown.md#bertacooldowncombatattack), [Berta.Cooldown.Combat.Attack.Charged](cooldown.md#bertacooldowncombatattackcharged), [Berta.Cooldown.Combat.Attack.Heavy](cooldown.md#bertacooldowncombatattackheavy), [Berta.Cooldown.Combat.Attack.Ranged](cooldown.md#bertacooldowncombatattackranged).

**Important distinctions:** Attack is an explicit parent restriction identifier. Confirm the intended hierarchical cooldown checks before mixing shared and variant restrictions.

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Attack.Ranged

**Meaning:** Identifies a cooldown restriction associated with the Attack Ranged capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Attack Ranged ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Attack.Ranged](ability.md#bertaabilitycombatattackranged), [Berta.Cooldown.Combat.Attack](cooldown.md#bertacooldowncombatattack), [Berta.Cooldown.Combat.Attack.Charged](cooldown.md#bertacooldowncombatattackcharged), [Berta.Cooldown.Combat.Attack.Heavy](cooldown.md#bertacooldowncombatattackheavy), [Berta.Cooldown.Combat.Attack.Light](cooldown.md#bertacooldowncombatattacklight).

**Important distinctions:** Attack is an explicit parent restriction identifier. Confirm the intended hierarchical cooldown checks before mixing shared and variant restrictions.

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Block

**Meaning:** Identifies a cooldown restriction associated with the Block capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Block ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Block](ability.md#bertaabilitycombatblock).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.CounterAttack

**Meaning:** Identifies a cooldown restriction associated with the CounterAttack capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding CounterAttack ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.CounterAttack](ability.md#bertaabilitycombatcounterattack).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Dodge

**Meaning:** Identifies a cooldown restriction associated with the Dodge capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Dodge ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Dodge](ability.md#bertaabilitycombatdodge).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Equip

**Meaning:** Identifies a cooldown restriction associated with the Equip capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Equip ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Equip](ability.md#bertaabilitycombatequip).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Execute

**Meaning:** Identifies a cooldown restriction associated with the Execute capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Execute ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Execute](ability.md#bertaabilitycombatexecute).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Fire

**Meaning:** Identifies a cooldown restriction associated with the Fire capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Fire ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Fire](ability.md#bertaabilitycombatfire).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.LockOn

**Meaning:** Identifies a cooldown restriction associated with the LockOn capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding LockOn ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.LockOn](ability.md#bertaabilitycombatlockon).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Parry

**Meaning:** Identifies a cooldown restriction associated with the Parry capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Parry ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Parry](ability.md#bertaabilitycombatparry).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Reload

**Meaning:** Identifies a cooldown restriction associated with the Reload capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Reload ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Reload](ability.md#bertaabilitycombatreload).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Roll

**Meaning:** Identifies a cooldown restriction associated with the Roll capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Roll ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Roll](ability.md#bertaabilitycombatroll).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Throw

**Meaning:** Identifies a cooldown restriction associated with the Throw capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Throw ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Throw](ability.md#bertaabilitycombatthrow).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Combat.Unequip

**Meaning:** Identifies a cooldown restriction associated with the Unequip capability in the combat domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Unequip ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Combat.Unequip](ability.md#bertaabilitycombatunequip).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

## Cooldown.Interaction

### Berta.Cooldown.Interaction.Close

**Meaning:** Identifies a cooldown restriction associated with the Close capability in the interaction domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Close ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Interaction.Close](ability.md#bertaabilityinteractionclose).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Interaction.Drop

**Meaning:** Identifies a cooldown restriction associated with the Drop capability in the interaction domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Drop ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Interaction.Drop](ability.md#bertaabilityinteractiondrop).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Interaction.Inspect

**Meaning:** Identifies a cooldown restriction associated with the Inspect capability in the interaction domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Inspect ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Interaction.Inspect](ability.md#bertaabilityinteractioninspect).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Interaction.Interact

**Meaning:** Identifies a cooldown restriction associated with the Interact capability in the interaction domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Interact ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Interaction.Interact](ability.md#bertaabilityinteractioninteract).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Interaction.Open

**Meaning:** Identifies a cooldown restriction associated with the Open capability in the interaction domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Open ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Interaction.Open](ability.md#bertaabilityinteractionopen).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Interaction.Pickup

**Meaning:** Identifies a cooldown restriction associated with the Pickup capability in the interaction domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Pickup ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Interaction.Pickup](ability.md#bertaabilityinteractionpickup).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Interaction.Revive

**Meaning:** Identifies a cooldown restriction associated with the Revive capability in the interaction domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Revive ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Interaction.Revive](ability.md#bertaabilityinteractionrevive).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Interaction.Talk

**Meaning:** Identifies a cooldown restriction associated with the Talk capability in the interaction domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Talk ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Interaction.Talk](ability.md#bertaabilityinteractiontalk).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Interaction.Use

**Meaning:** Identifies a cooldown restriction associated with the Use capability in the interaction domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Use ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Interaction.Use](ability.md#bertaabilityinteractionuse).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

## Cooldown.Movement

### Berta.Cooldown.Movement.Climb

**Meaning:** Identifies a cooldown restriction associated with the Climb capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Climb ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Climb](ability.md#bertaabilitymovementclimb).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.Crouch

**Meaning:** Identifies a cooldown restriction associated with the Crouch capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Crouch ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Crouch](ability.md#bertaabilitymovementcrouch).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.Dash

**Meaning:** Identifies a cooldown restriction associated with the Dash capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Dash ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Dash](ability.md#bertaabilitymovementdash).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.DoubleJump

**Meaning:** Identifies a cooldown restriction associated with the DoubleJump capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding DoubleJump ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.DoubleJump](ability.md#bertaabilitymovementdoublejump).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.Glide

**Meaning:** Identifies a cooldown restriction associated with the Glide capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Glide ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Glide](ability.md#bertaabilitymovementglide).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.Grapple

**Meaning:** Identifies a cooldown restriction associated with the Grapple capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Grapple ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Grapple](ability.md#bertaabilitymovementgrapple).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.Jump

**Meaning:** Identifies a cooldown restriction associated with the Jump capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Jump ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Jump](ability.md#bertaabilitymovementjump), [Berta.Event.Movement.Jumped](event.md#bertaeventmovementjumped), [Berta.Event.Movement.Landed](event.md#bertaeventmovementlanded), [Berta.State.Movement.Falling](state.md#bertastatemovementfalling), [Berta.State.Movement.InAir](state.md#bertastatemovementinair), [Berta.State.Movement.Jumping](state.md#bertastatemovementjumping), [GameplayCue.Berta.Movement.Jump](gameplay-cue.md#gameplaycuebertamovementjump), [GameplayCue.Berta.Movement.Land](gameplay-cue.md#gameplaycuebertamovementland).

**Important distinctions:** Jump ability identity, movement intervals, transition notifications, optional cooldown, and cosmetic cues are separate contracts. See the manual GA_Jump walkthrough in the integration guide.

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.Mantle

**Meaning:** Identifies a cooldown restriction associated with the Mantle capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Mantle ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Mantle](ability.md#bertaabilitymovementmantle).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.Slide

**Meaning:** Identifies a cooldown restriction associated with the Slide capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Slide ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Slide](ability.md#bertaabilitymovementslide).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.Sprint

**Meaning:** Identifies a cooldown restriction associated with the Sprint capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Sprint ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Sprint](ability.md#bertaabilitymovementsprint).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.Swim

**Meaning:** Identifies a cooldown restriction associated with the Swim capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Swim ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Swim](ability.md#bertaabilitymovementswim).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.Teleport

**Meaning:** Identifies a cooldown restriction associated with the Teleport capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Teleport ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Teleport](ability.md#bertaabilitymovementteleport).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.Vault

**Meaning:** Identifies a cooldown restriction associated with the Vault capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Vault ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.Vault](ability.md#bertaabilitymovementvault).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Movement.WallRun

**Meaning:** Identifies a cooldown restriction associated with the WallRun capability in the movement domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding WallRun ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Movement.WallRun](ability.md#bertaabilitymovementwallrun).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

## Cooldown.Support

### Berta.Cooldown.Support.Buff

**Meaning:** Identifies a cooldown restriction associated with the Buff capability in the support domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Buff ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Support.Buff](ability.md#bertaabilitysupportbuff).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Support.Cleanse

**Meaning:** Identifies a cooldown restriction associated with the Cleanse capability in the support domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Cleanse ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Support.Cleanse](ability.md#bertaabilitysupportcleanse).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Support.Debuff

**Meaning:** Identifies a cooldown restriction associated with the Debuff capability in the support domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Debuff ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Support.Debuff](ability.md#bertaabilitysupportdebuff).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Support.Deploy

**Meaning:** Identifies a cooldown restriction associated with the Deploy capability in the support domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Deploy ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Support.Deploy](ability.md#bertaabilitysupportdeploy).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Support.Heal

**Meaning:** Identifies a cooldown restriction associated with the Heal capability in the support domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Heal ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Support.Heal](ability.md#bertaabilitysupportheal).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Support.Shield

**Meaning:** Identifies a cooldown restriction associated with the Shield capability in the support domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Shield ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Support.Shield](ability.md#bertaabilitysupportshield).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.

### Berta.Cooldown.Support.Summon

**Meaning:** Identifies a cooldown restriction associated with the Summon capability in the support domain.

**Scope and suggested owner:** native GAS cooldown restriction identifier; cooldown Gameplay Effect granting the tag to the ability owner ASC.

**Lifecycle:** Grant through a configured duration effect on the chosen commitment path; native effect expiry/removal ends that contribution.

**Example:** A duration Gameplay Effect grants this tag after the corresponding Summon ability commits; that ability checks it through its configured cooldown tags.

**Related entries:** [Berta.Ability.Support.Summon](ability.md#bertaabilitysupportsummon).

**Implementation responsibility:** The project configures GetCooldownTags-compatible checks, duration, shared restrictions, prediction, and authority.
