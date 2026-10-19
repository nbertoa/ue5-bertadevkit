# State — 51 tags

State tags describe conditions that remain true over an interval. The suggested character-facing owner is an ASC or a project-owned actor tag provider. Each condition needs one authority that derives it, a clear entry and exit rule, and cleanup on interruption, destruction, possession changes, or respawn where relevant. Do not make a Blueprint boolean and an ASC tag independently authoritative for the same condition.

Duration Gameplay Effects can own effect-driven states. Character Movement integration should derive movement states from the actual movement authority. A local Gameplay Tag Container has no automatic replication; ASC tag replication depends on how GAS grants the tags and its networking configuration. Choose an explicit server/prediction policy and verify clients.

Combinations are useful: Grounded and Idle may coexist, as may InAir and Falling. Life-state exclusivity, stance exclusivity, and other rules are project invariants, never behavior supplied here. Native Character Movement's falling mode can include upward jump travel; a narrower descending-only meaning requires a separate, explicit project policy.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## State.Combat

### Berta.State.Combat.Aiming

**Meaning:** The character is currently directing an aimed action.

**Scope and suggested owner:** character combat condition; combat/weapon-action integration exposing character state through its ASC or tag provider.

**Lifecycle:** Maintain for the accepted combat phase; clear on completion, cancellation, equipment changes, and teardown as appropriate.

**Example:** Select an aiming pose independently of camera mode.

**Related entries:** [Berta.Ability.Combat.Aim](ability.md#bertaabilitycombataim), [Berta.Camera.Mode.Aiming](camera.md#bertacameramodeaiming).

**Important distinctions:** Camera policy, character action condition, and ability identity are independent; the project coordinates them explicitly.

**Implementation responsibility:** The project reconciles action callbacks and weapon state without creating competing authorities.

### Berta.State.Combat.Attacking

**Meaning:** The character is currently in an offensive action phase.

**Scope and suggested owner:** character combat condition; combat/weapon-action integration exposing character state through its ASC or tag provider.

**Lifecycle:** Maintain for the accepted combat phase; clear on completion, cancellation, equipment changes, and teardown as appropriate.

**Example:** Gate incompatible interaction while an attack controller owns the phase.

**Implementation responsibility:** The project reconciles action callbacks and weapon state without creating competing authorities.

### Berta.State.Combat.Blocking

**Meaning:** The character is currently maintaining a defensive guard.

**Scope and suggested owner:** character combat condition; combat/weapon-action integration exposing character state through its ASC or tag provider.

**Lifecycle:** Maintain for the accepted combat phase; clear on completion, cancellation, equipment changes, and teardown as appropriate.

**Example:** Let hit resolution evaluate a block attempt.

**Implementation responsibility:** The project reconciles action callbacks and weapon state without creating competing authorities.

### Berta.State.Combat.Equipping

**Meaning:** The character is currently transitioning equipment into active use.

**Scope and suggested owner:** character combat condition; combat/weapon-action integration exposing character state through its ASC or tag provider.

**Lifecycle:** Maintain for the accepted combat phase; clear on completion, cancellation, equipment changes, and teardown as appropriate.

**Example:** Delay firing until the equip transition completes.

**Implementation responsibility:** The project reconciles action callbacks and weapon state without creating competing authorities.

### Berta.State.Combat.InCombat

**Meaning:** The combat authority currently considers the actor engaged in combat.

**Scope and suggested owner:** character combat condition; combat/weapon-action integration exposing character state through its ASC or tag provider.

**Lifecycle:** Maintain for the accepted combat phase; clear on completion, cancellation, equipment changes, and teardown as appropriate.

**Example:** Keep combat music active until the engagement timeout expires.

**Implementation responsibility:** The project reconciles action callbacks and weapon state without creating competing authorities.

### Berta.State.Combat.InCover

**Meaning:** The character is currently engaged with a cover system.

**Scope and suggested owner:** character combat condition; combat/weapon-action integration exposing character state through its ASC or tag provider.

**Lifecycle:** Maintain for the accepted combat phase; clear on completion, cancellation, equipment changes, and teardown as appropriate.

**Example:** Select cover movement while the cover controller maintains attachment.

**Implementation responsibility:** The project reconciles action callbacks and weapon state without creating competing authorities.

### Berta.State.Combat.LockedOn

**Meaning:** The character currently has an accepted focused combat target.

**Scope and suggested owner:** character combat condition; combat/weapon-action integration exposing character state through its ASC or tag provider.

**Lifecycle:** Maintain for the accepted combat phase; clear on completion, cancellation, equipment changes, and teardown as appropriate.

**Example:** Show target-lock presentation while the target remains eligible.

**Implementation responsibility:** The project reconciles action callbacks and weapon state without creating competing authorities.

### Berta.State.Combat.Parrying

**Meaning:** The character is currently inside an active parry phase.

**Scope and suggested owner:** character combat condition; combat/weapon-action integration exposing character state through its ASC or tag provider.

**Lifecycle:** Maintain for the accepted combat phase; clear on completion, cancellation, equipment changes, and teardown as appropriate.

**Example:** Let hit resolution test a timed defensive window.

**Implementation responsibility:** The project reconciles action callbacks and weapon state without creating competing authorities.

### Berta.State.Combat.Reloading

**Meaning:** The character is currently engaged in a reload action.

**Scope and suggested owner:** character combat condition; combat/weapon-action integration exposing character state through its ASC or tag provider.

**Lifecycle:** Maintain for the accepted combat phase; clear on completion, cancellation, equipment changes, and teardown as appropriate.

**Example:** Block an incompatible character action during reload.

**Related entries:** [Berta.Ability.Combat.Reload](ability.md#bertaabilitycombatreload), [Berta.Event.Combat.ReloadCompleted](event.md#bertaeventcombatreloadcompleted), [Berta.Weapon.State.Reloading](weapon.md#bertaweaponstatereloading).

**Important distinctions:** These describe weapon condition, character condition, capability identity, and completed occurrence respectively. Their owners and lifetimes can differ.

**Implementation responsibility:** The project reconciles action callbacks and weapon state without creating competing authorities.

## State.Control

### Berta.State.Control.Confused

**Meaning:** The actor's normal decision or control interpretation is disrupted.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Apply a project-defined confusion targeting policy.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.State.Control.Disarmed

**Meaning:** The actor cannot use affected weapon actions.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Block weapon attacks while leaving unarmed actions eligible if configured.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.State.Control.Feared

**Meaning:** The actor is under a fear-driven control condition.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Let the control system select flight behavior for a fear duration.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.State.Control.InputLocked

**Meaning:** The actor's relevant player input is temporarily suppressed.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Gate gameplay input during a controlled transition.

**Related entries:** [Berta.State.Control.Rooted](state.md#bertastatecontrolrooted), [Berta.State.Control.Stunned](state.md#bertastatecontrolstunned).

**Important distinctions:** Stun is an action impairment, root is positional restraint, and input lock suppresses input. None automatically implies either of the others or blocks abilities by itself.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.State.Control.Rooted

**Meaning:** The actor is prevented from changing position by a movement restraint.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Block locomotion while allowing stationary attacks if the game permits them.

**Related entries:** [Berta.State.Control.InputLocked](state.md#bertastatecontrolinputlocked), [Berta.State.Control.Stunned](state.md#bertastatecontrolstunned).

**Important distinctions:** Stun is an action impairment, root is positional restraint, and input lock suppresses input. None automatically implies either of the others or blocks abilities by itself.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.State.Control.Silenced

**Meaning:** The actor is prevented from using a project-defined set of abilities.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Reject spell activation while leaving permitted weapon actions available.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.State.Control.Stunned

**Meaning:** The actor is impaired by a condition intended to broadly interrupt action.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Block selected abilities while a stun effect is active.

**Related entries:** [Berta.State.Control.InputLocked](state.md#bertastatecontrolinputlocked), [Berta.State.Control.Rooted](state.md#bertastatecontrolrooted).

**Important distinctions:** Stun is an action impairment, root is positional restraint, and input lock suppresses input. None automatically implies either of the others or blocks abilities by itself.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

## State.Defense

### Berta.State.Defense.Invulnerable

**Meaning:** The actor currently rejects damage covered by the project's invulnerability policy.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Let damage resolution exclude protected recipients.

**Related entries:** [Berta.State.Defense.SuperArmor](state.md#bertastatedefensesuperarmor).

**Important distinctions:** Damage rejection and resistance to interruption are distinct. Super armor need not prevent damage.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

### Berta.State.Defense.SuperArmor

**Meaning:** The actor currently resists configured interruption or stagger.

**Scope and suggested owner:** recipient condition; condition provider, often an active Gameplay Effect on the recipient ASC.

**Lifecycle:** Maintain for the condition duration. Expiry, cleansing, cancellation, and overlapping contributors must leave a correct effective tag count.

**Example:** Keep an attack from being interrupted without implying damage immunity.

**Related entries:** [Berta.State.Defense.Invulnerable](state.md#bertastatedefenseinvulnerable).

**Important distinctions:** Damage rejection and resistance to interruption are distinct. Super armor need not prevent damage.

**Implementation responsibility:** The project implements modifiers, restrictions, stacking, authority, and cleanup; presence alone changes no behavior.

## State.Environment

### Berta.State.Environment.InHazardZone

**Meaning:** The actor currently occupies a hazardous area.

**Scope and suggested owner:** actor environment membership condition; environment/zone integration exposing the affected actor state.

**Lifecycle:** Derive from validated occupancy or submersion queries; remove after exit and reconcile overlapping regions and teleportation.

**Example:** Display a hazard warning while authoritative zone membership persists.

**Implementation responsibility:** The project decides overlap composition, authority, and any hazard or breathing behavior.

### Berta.State.Environment.InSafeZone

**Meaning:** The actor currently occupies an area classified as safe.

**Scope and suggested owner:** actor environment membership condition; environment/zone integration exposing the affected actor state.

**Lifecycle:** Derive from validated occupancy or submersion queries; remove after exit and reconcile overlapping regions and teleportation.

**Example:** Gate combat actions while a safe-area overlap remains valid.

**Implementation responsibility:** The project decides overlap composition, authority, and any hazard or breathing behavior.

### Berta.State.Environment.Underwater

**Meaning:** The actor is currently submerged according to an environment query.

**Scope and suggested owner:** actor environment membership condition; environment/zone integration exposing the affected actor state.

**Lifecycle:** Derive from validated occupancy or submersion queries; remove after exit and reconcile overlapping regions and teleportation.

**Example:** Enable oxygen consumption when the actor's breathing point is submerged.

**Related entries:** [Berta.State.Movement.Swimming](state.md#bertastatemovementswimming), [Berta.World.Zone.Underwater](world.md#bertaworldzoneunderwater).

**Important distinctions:** A region classification, actor submersion, and locomotion regime are separate. Swimming near the surface need not imply full submersion.

**Implementation responsibility:** The project decides overlap composition, authority, and any hazard or breathing behavior.

## State.Interaction

### Berta.State.Interaction.Carrying

**Meaning:** The actor currently carries an object in the game's carrying system.

**Scope and suggested owner:** participant interaction condition; interaction-session authority exposing the participant state.

**Lifecycle:** Maintain while the session or carrying relationship remains valid; clear on completion, cancellation, target loss, or teardown.

**Example:** Select a carrying pose for a held crate.

**Implementation responsibility:** The project owns reservations, transaction validity, and participant cleanup.

### Berta.State.Interaction.InDialogue

**Meaning:** The actor currently participates in a dialogue session.

**Scope and suggested owner:** participant interaction condition; interaction-session authority exposing the participant state.

**Lifecycle:** Maintain while the session or carrying relationship remains valid; clear on completion, cancellation, target loss, or teardown.

**Example:** Route dialogue choices while the session remains active.

**Implementation responsibility:** The project owns reservations, transaction validity, and participant cleanup.

### Berta.State.Interaction.Interacting

**Meaning:** The actor is currently engaged in an interaction session.

**Scope and suggested owner:** participant interaction condition; interaction-session authority exposing the participant state.

**Lifecycle:** Maintain while the session or carrying relationship remains valid; clear on completion, cancellation, target loss, or teardown.

**Example:** Prevent a second exclusive interaction until the current session ends.

**Implementation responsibility:** The project owns reservations, transaction validity, and participant cleanup.

### Berta.State.Interaction.Looting

**Meaning:** The actor currently participates in a loot-access session.

**Scope and suggested owner:** participant interaction condition; interaction-session authority exposing the participant state.

**Lifecycle:** Maintain while the session or carrying relationship remains valid; clear on completion, cancellation, target loss, or teardown.

**Example:** Maintain a loot panel while access is valid.

**Implementation responsibility:** The project owns reservations, transaction validity, and participant cleanup.

## State.Life

### Berta.State.Life.Alive

**Meaning:** The life-state authority considers the actor living.

**Scope and suggested owner:** actor life condition; life-state authority, exposed through the recipient ASC or actor tag provider.

**Lifecycle:** Enter after a validated life transition; leave when the authority transitions away. Reconcile reconstruction and respawn explicitly.

**Example:** Allow healing to a living target under the game's downed-state policy.

**Related entries:** [Berta.State.Life.Dead](state.md#bertastatelifedead), [Berta.State.Life.Downed](state.md#bertastatelifedowned), [Berta.State.Life.Dying](state.md#bertastatelifedying), [Berta.State.Life.Respawning](state.md#bertastateliferespawning).

**Important distinctions:** Define life-state transitions and any exclusivity in the project. A downed actor may still count as alive for selected rules; this catalog imposes no exclusive enum.

**Implementation responsibility:** The project defines recovery/death rules and whether life labels are exclusive or overlapping.

### Berta.State.Life.Dead

**Meaning:** The life-state authority considers the actor dead.

**Scope and suggested owner:** actor life condition; life-state authority, exposed through the recipient ASC or actor tag provider.

**Lifecycle:** Enter after a validated life transition; leave when the authority transitions away. Reconcile reconstruction and respawn explicitly.

**Example:** Exclude a corpse from ordinary living-target selection.

**Related entries:** [Berta.State.Life.Alive](state.md#bertastatelifealive), [Berta.State.Life.Downed](state.md#bertastatelifedowned), [Berta.State.Life.Dying](state.md#bertastatelifedying), [Berta.State.Life.Respawning](state.md#bertastateliferespawning).

**Important distinctions:** Define life-state transitions and any exclusivity in the project. A downed actor may still count as alive for selected rules; this catalog imposes no exclusive enum.

**Implementation responsibility:** The project defines recovery/death rules and whether life labels are exclusive or overlapping.

### Berta.State.Life.Downed

**Meaning:** The actor is incapacitated but remains eligible for a recovery path.

**Scope and suggested owner:** actor life condition; life-state authority, exposed through the recipient ASC or actor tag provider.

**Lifecycle:** Enter after a validated life transition; leave when the authority transitions away. Reconcile reconstruction and respawn explicitly.

**Example:** Offer a revive interaction while the recovery window remains valid.

**Related entries:** [Berta.State.Life.Alive](state.md#bertastatelifealive), [Berta.State.Life.Dead](state.md#bertastatelifedead), [Berta.State.Life.Dying](state.md#bertastatelifedying), [Berta.State.Life.Respawning](state.md#bertastateliferespawning).

**Important distinctions:** Define life-state transitions and any exclusivity in the project. A downed actor may still count as alive for selected rules; this catalog imposes no exclusive enum.

**Implementation responsibility:** The project defines recovery/death rules and whether life labels are exclusive or overlapping.

### Berta.State.Life.Dying

**Meaning:** The actor is undergoing a death transition that has not completed.

**Scope and suggested owner:** actor life condition; life-state authority, exposed through the recipient ASC or actor tag provider.

**Lifecycle:** Enter after a validated life transition; leave when the authority transitions away. Reconcile reconstruction and respawn explicitly.

**Example:** Gate ordinary interaction during an authored death sequence.

**Related entries:** [Berta.State.Life.Alive](state.md#bertastatelifealive), [Berta.State.Life.Dead](state.md#bertastatelifedead), [Berta.State.Life.Downed](state.md#bertastatelifedowned), [Berta.State.Life.Respawning](state.md#bertastateliferespawning).

**Important distinctions:** Define life-state transitions and any exclusivity in the project. A downed actor may still count as alive for selected rules; this catalog imposes no exclusive enum.

**Implementation responsibility:** The project defines recovery/death rules and whether life labels are exclusive or overlapping.

### Berta.State.Life.Respawning

**Meaning:** The actor is in a restoration or reconstruction phase after death.

**Scope and suggested owner:** actor life condition; life-state authority, exposed through the recipient ASC or actor tag provider.

**Lifecycle:** Enter after a validated life transition; leave when the authority transitions away. Reconcile reconstruction and respawn explicitly.

**Example:** Keep input restricted until spawn initialization completes.

**Related entries:** [Berta.State.Life.Alive](state.md#bertastatelifealive), [Berta.State.Life.Dead](state.md#bertastatelifedead), [Berta.State.Life.Downed](state.md#bertastatelifedowned), [Berta.State.Life.Dying](state.md#bertastatelifedying).

**Important distinctions:** Define life-state transitions and any exclusivity in the project. A downed actor may still count as alive for selected rules; this catalog imposes no exclusive enum.

**Implementation responsibility:** The project defines recovery/death rules and whether life labels are exclusive or overlapping.

## State.Movement

### Berta.State.Movement.Climbing

**Meaning:** The character is currently traversing a climbable surface.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Gate ordinary jumping while the climbing controller is engaged.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Crouching

**Meaning:** The character currently has a crouched locomotion posture.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Choose reduced-height traversal queries after crouch is accepted.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Dashing

**Meaning:** The character is currently undergoing a dash maneuver.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Gate incompatible movement while the dash controller owns the burst.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Diving

**Meaning:** The character is currently performing a deliberate diving movement phase.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Select a dive animation before or during submersion.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Dodging

**Meaning:** The character is currently performing an evasive maneuver.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Select evasion presentation without assuming invulnerability.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Falling

**Meaning:** The character is in the movement authority's falling condition.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Handle walking off a ledge without claiming that a jump ability activated.

**Related entries:** [Berta.Ability.Movement.Jump](ability.md#bertaabilitymovementjump), [Berta.Cooldown.Movement.Jump](cooldown.md#bertacooldownmovementjump), [Berta.Event.Movement.Jumped](event.md#bertaeventmovementjumped), [Berta.Event.Movement.Landed](event.md#bertaeventmovementlanded), [Berta.State.Movement.InAir](state.md#bertastatemovementinair), [Berta.State.Movement.Jumping](state.md#bertastatemovementjumping), [GameplayCue.Berta.Movement.Jump](gameplay-cue.md#gameplaycuebertamovementjump), [GameplayCue.Berta.Movement.Land](gameplay-cue.md#gameplaycuebertamovementland).

**Important distinctions:** Jump ability identity, movement intervals, transition notifications, optional cooldown, and cosmetic cues are separate contracts. See the manual GA_Jump walkthrough in the integration guide. InAir covers unsupported movement. Jumping is a deliberate project-defined phase; native falling mode can include upward travel. These labels need not be mutually exclusive.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Flying

**Meaning:** The character is currently using a flight locomotion regime.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Choose flight controls without equating flight with uncontrolled falling.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Gliding

**Meaning:** The character is currently controlling descent through gliding.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Display glider presentation while the movement integration maintains glide.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Grounded

**Meaning:** The character is supported by a walkable surface according to movement authority.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Permit a ground-only attack while the actor is stationary or moving.

**Related entries:** [Berta.State.Movement.Idle](state.md#bertastatemovementidle).

**Important distinctions:** Idle describes locomotion activity; Grounded describes support. A stationary grounded actor can satisfy both.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Idle

**Meaning:** The character is not currently performing meaningful locomotion under project thresholds.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** An animation selector uses the stationary condition while the actor remains grounded.

**Related entries:** [Berta.State.Movement.Grounded](state.md#bertastatemovementgrounded).

**Important distinctions:** Idle describes locomotion activity; Grounded describes support. A stationary grounded actor can satisfy both.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.InAir

**Meaning:** The character is airborne rather than supported by a walkable surface.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Block a ground interaction during both ascent and descent.

**Related entries:** [Berta.Ability.Movement.Jump](ability.md#bertaabilitymovementjump), [Berta.Cooldown.Movement.Jump](cooldown.md#bertacooldownmovementjump), [Berta.Event.Movement.Jumped](event.md#bertaeventmovementjumped), [Berta.Event.Movement.Landed](event.md#bertaeventmovementlanded), [Berta.State.Movement.Falling](state.md#bertastatemovementfalling), [Berta.State.Movement.Jumping](state.md#bertastatemovementjumping), [GameplayCue.Berta.Movement.Jump](gameplay-cue.md#gameplaycuebertamovementjump), [GameplayCue.Berta.Movement.Land](gameplay-cue.md#gameplaycuebertamovementland).

**Important distinctions:** Jump ability identity, movement intervals, transition notifications, optional cooldown, and cosmetic cues are separate contracts. See the manual GA_Jump walkthrough in the integration guide. InAir covers unsupported movement. Jumping is a deliberate project-defined phase; native falling mode can include upward travel. These labels need not be mutually exclusive.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Jumping

**Meaning:** The character is in the project's deliberate jump phase.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Select takeoff feedback after an accepted jump, with a project-defined phase end.

**Related entries:** [Berta.Ability.Movement.Jump](ability.md#bertaabilitymovementjump), [Berta.Cooldown.Movement.Jump](cooldown.md#bertacooldownmovementjump), [Berta.Event.Movement.Jumped](event.md#bertaeventmovementjumped), [Berta.Event.Movement.Landed](event.md#bertaeventmovementlanded), [Berta.State.Movement.Falling](state.md#bertastatemovementfalling), [Berta.State.Movement.InAir](state.md#bertastatemovementinair), [GameplayCue.Berta.Movement.Jump](gameplay-cue.md#gameplaycuebertamovementjump), [GameplayCue.Berta.Movement.Land](gameplay-cue.md#gameplaycuebertamovementland).

**Important distinctions:** Jump ability identity, movement intervals, transition notifications, optional cooldown, and cosmetic cues are separate contracts. See the manual GA_Jump walkthrough in the integration guide. InAir covers unsupported movement. Jumping is a deliberate project-defined phase; native falling mode can include upward travel. These labels need not be mutually exclusive.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Rolling

**Meaning:** The character is currently performing a rolling maneuver.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Choose a roll animation while the project controls the roll phase.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Running

**Meaning:** The character is currently using its normal running locomotion regime.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Choose a running animation below the project's sprint regime.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Sliding

**Meaning:** The character is currently traversing in a slide.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Use a slide-specific collision or presentation policy.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Sprinting

**Meaning:** The character is currently in an accepted sprint regime.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Consume stamina while the movement integration reports sprinting.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Swimming

**Meaning:** The character is currently using swimming locomotion.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Select aquatic movement behavior independently of being fully underwater.

**Related entries:** [Berta.State.Environment.Underwater](state.md#bertastateenvironmentunderwater), [Berta.World.Zone.Underwater](world.md#bertaworldzoneunderwater).

**Important distinctions:** A region classification, actor submersion, and locomotion regime are separate. Swimming near the surface need not imply full submersion.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

### Berta.State.Movement.Walking

**Meaning:** The character is currently using the project's walking locomotion regime.

**Scope and suggested owner:** character locomotion condition; Character Movement integration, exposing the condition through the character ASC or actor tag provider.

**Lifecycle:** Add while the movement authority reports this condition; remove when it ceases. Reconcile on movement-mode changes, interruption, and respawn.

**Example:** Select walking footsteps without treating a slow status as proof of walking.

**Related entries:** [Berta.Status.Slowed](status.md#bertastatusslowed).

**Important distinctions:** Slowed is a speed-affecting condition; Walking is the selected locomotion regime. A slowed actor can still run or stand still.

**Implementation responsibility:** The project defines thresholds and phase boundaries and synchronizes one derived representation with native movement.

## State.Stealth

### Berta.State.Stealth.Disguised

**Meaning:** The actor currently presents an alternate identity or appearance.

**Scope and suggested owner:** actor stealth condition; stealth or recognition authority exposing the actor condition.

**Lifecycle:** Maintain while the concealment, exposure, or identity condition is valid; reconcile changes in contributing sources.

**Example:** Let recognition logic evaluate a disguise against observer knowledge.

**Implementation responsibility:** The project defines observer-relative perception and precedence between concurrent stealth conditions.

### Berta.State.Stealth.Hidden

**Meaning:** The stealth authority considers the actor concealed under its visibility rules.

**Scope and suggested owner:** actor stealth condition; stealth or recognition authority exposing the actor condition.

**Lifecycle:** Maintain while the concealment, exposure, or identity condition is valid; reconcile changes in contributing sources.

**Example:** Suppress ordinary detection for observers covered by that policy.

**Implementation responsibility:** The project defines observer-relative perception and precedence between concurrent stealth conditions.

### Berta.State.Stealth.Revealed

**Meaning:** The actor is currently exposed by a reveal rule or effect.

**Scope and suggested owner:** actor stealth condition; stealth or recognition authority exposing the actor condition.

**Lifecycle:** Maintain while the concealment, exposure, or identity condition is valid; reconcile changes in contributing sources.

**Example:** Allow detection despite otherwise qualifying concealment.

**Implementation responsibility:** The project defines observer-relative perception and precedence between concurrent stealth conditions.
