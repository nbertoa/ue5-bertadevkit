# Event — 50 tags

Events describe occurrences and transitions, not persistent actor conditions. Producers emit them after the declared milestone is accepted or committed. The project defines whether notifications occur at start, completion, or another explicit point and prevents duplicate emissions from overlapping callbacks. Consumers must not infer that the condition remains true merely because an event was seen.

For GAS, UAbilitySystemBlueprintLibrary::SendGameplayEventToActor sends FGameplayEventData to the actor's accessible ASC. Configure ability event triggers or listeners explicitly. This helper is not a general replicated event bus: the caller chooses where it executes, how authority is validated, and whether another network message is required. Payloads should identify the instigator, target, magnitude, and relevant context where supported and meaningful.

Not every event must use GAS; inventory, quest, and AI systems can use these labels in their own notifications. Berta supplies no dispatcher and no automatic bridge from native delegates. An observed HitConfirmed is contact evidence; DamageDealt requires resolved damage evidence.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Event.AI

### Berta.Event.AI.Alerted

**Meaning:** An AI agent entered an alert response milestone.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An AI controller announces a validated alert transition.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.AI.NoiseHeard

**Meaning:** An AI system accepted a hearing stimulus.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A perception controller reports the noise location and source.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.AI.TargetLost

**Meaning:** An AI system lost a previously tracked target under its retention policy.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A controller announces that its target memory expired.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.AI.TargetSpotted

**Meaning:** An AI system acquired a qualifying target observation.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A perception controller announces a newly seen hostile.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

## Event.Ability

### Berta.Event.Ability.Activated

**Meaning:** A Gameplay Ability began activation.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An ability observer reports the activated spec to project listeners.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Ability.Cancelled

**Meaning:** A Gameplay Ability terminated through cancellation.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An ability observer reports an interrupted cast.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Ability.Committed

**Meaning:** A Gameplay Ability successfully committed its configured cost and cooldown operation.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An ability reports successful CommitAbility rather than mere input reception.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Ability.Ended

**Meaning:** A Gameplay Ability reached its end notification.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An observer reports the end with cancellation details where available.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Ability.Failed

**Meaning:** An attempted Gameplay Ability activation or declared operation failed.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A caller reports an activation rejection with failure tags in payload data.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

## Event.Combat

### Berta.Event.Combat.AttackEnded

**Meaning:** An offensive action left its declared active lifecycle.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An attack controller reports termination and includes its outcome in payload data.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Combat.AttackStarted

**Meaning:** An offensive action entered its declared start phase.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An attack controller announces the start of a swing.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Combat.CriticalHit

**Meaning:** Hit resolution classified an outcome as critical.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A combat resolver announces a critical result for rewards or presentation.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Combat.DamageBlocked

**Meaning:** A defensive block prevented or reduced incoming damage.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** Hit resolution reports a successful shield block.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Combat.DamageDealt

**Meaning:** Outgoing damage resolution produced a damage result.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** The damage system reports actual resolved damage to the instigator.

**Related entries:** [Berta.Event.Combat.HitConfirmed](event.md#bertaeventcombathitconfirmed).

**Important distinctions:** Confirmed contact can resolve to zero damage. Define damage notifications from the actual damage resolver and carry the resolved value explicitly.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Combat.DamageParried

**Meaning:** A timed parry successfully intercepted an incoming attack.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** Hit resolution opens a riposte opportunity after a validated parry.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Combat.DamageReceived

**Meaning:** Incoming damage resolution produced a damage result.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** The recipient's damage authority reports its resolved damage result.

**Related entries:** [Berta.Event.Combat.HitReceived](event.md#bertaeventcombathitreceived).

**Important distinctions:** Receiving contact is distinct from receiving resolved damage; immunity, blocking, or other rules may separate the outcomes.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Combat.HitConfirmed

**Meaning:** The attacking system confirmed contact with a valid target.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A melee trace reports contact even when armor prevents health loss.

**Related entries:** [Berta.Event.Combat.DamageDealt](event.md#bertaeventcombatdamagedealt).

**Important distinctions:** Confirmed contact can resolve to zero damage. Define damage notifications from the actual damage resolver and carry the resolved value explicitly.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Combat.HitReceived

**Meaning:** The receiving system accepted a contact notification.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A recipient reacts to an impact that may deal zero damage.

**Related entries:** [Berta.Event.Combat.DamageReceived](event.md#bertaeventcombatdamagereceived).

**Important distinctions:** Receiving contact is distinct from receiving resolved damage; immunity, blocking, or other rules may separate the outcomes.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Combat.ReloadCompleted

**Meaning:** A reload operation reached successful completion.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A weapon controller reports the final ammunition transfer.

**Related entries:** [Berta.Ability.Combat.Reload](ability.md#bertaabilitycombatreload), [Berta.State.Combat.Reloading](state.md#bertastatecombatreloading), [Berta.Weapon.State.Reloading](weapon.md#bertaweaponstatereloading).

**Important distinctions:** These describe weapon condition, character condition, capability identity, and completed occurrence respectively. Their owners and lifetimes can differ.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Combat.ReloadStarted

**Meaning:** A reload operation began after acceptance.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A weapon controller reports that ammunition transfer has started.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Combat.ShieldBroken

**Meaning:** A shield crossed the project's broken or depleted threshold.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A resource resolver announces depletion once at the threshold crossing.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

## Event.Interaction

### Berta.Event.Interaction.Cancelled

**Meaning:** An interaction session terminated without normal completion.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A hold interaction reports cancellation when its target becomes unavailable.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Interaction.Completed

**Meaning:** An interaction session achieved its success condition.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A terminal reports successful use after validation.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Interaction.Crafted

**Meaning:** A crafting transaction successfully produced its result.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A crafting authority reports the resulting item and quantity.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Interaction.DialogueEnded

**Meaning:** Participants left a terminated conversation session.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A dialogue controller reports completion or interruption in the payload.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Interaction.DialogueStarted

**Meaning:** Participants entered an accepted conversation session.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A dialogue controller notifies participants after choosing the conversation.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Interaction.Dropped

**Meaning:** A held or stored object was successfully released.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A carrying controller reports the new world object after a drop.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Interaction.Opened

**Meaning:** An object successfully transitioned into an open configuration.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A door controller announces accepted opening.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Interaction.PickedUp

**Meaning:** A world object was successfully acquired.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An inventory transaction reports a committed pickup.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Interaction.Started

**Meaning:** An interaction session began after acceptance.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An interaction authority reports the start of a timed use action.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Interaction.Traded

**Meaning:** An exchange transaction successfully committed.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A trade authority reports the completed exchange to both participants.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Interaction.Used

**Meaning:** An object's intended function was successfully invoked.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A consumable controller reports a committed potion use.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

## Event.Life

### Berta.Event.Life.Despawned

**Meaning:** An entity left the project's active world population.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A lifetime controller notifies listeners before its object becomes inaccessible.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Life.Died

**Meaning:** A life-state transition reached the project's death milestone.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A life authority announces a death for scoring.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Life.Downed

**Meaning:** A life-state transition entered incapacitated recovery eligibility.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A life authority announces a newly downed teammate.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Life.Respawned

**Meaning:** An entity completed its restoration into play after death.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A respawn controller announces the new playable incarnation.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Life.Revived

**Meaning:** A downed entity successfully returned to an active life condition.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A revive controller announces successful recovery.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Life.Spawned

**Meaning:** An entity entered the project's active world population.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A spawn authority announces an initialized new actor.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

## Event.Movement

### Berta.Event.Movement.Dashed

**Meaning:** A dash occurrence reached the project's declared notification point.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A dash ability announces its accepted burst once.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Movement.Dodged

**Meaning:** An evasion occurrence reached the project's declared notification point.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An evasion system reports a completed dodge milestone.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Movement.EnteredWater

**Meaning:** The actor transitioned into a water region.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An environment integration announces a dry-to-water boundary crossing.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Movement.Jumped

**Meaning:** A deliberate jump was accepted and began.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A movement integration emits this after takeoff acceptance.

**Related entries:** [Berta.Ability.Movement.Jump](ability.md#bertaabilitymovementjump), [Berta.Cooldown.Movement.Jump](cooldown.md#bertacooldownmovementjump), [Berta.Event.Movement.Landed](event.md#bertaeventmovementlanded), [Berta.State.Movement.Falling](state.md#bertastatemovementfalling), [Berta.State.Movement.InAir](state.md#bertastatemovementinair), [Berta.State.Movement.Jumping](state.md#bertastatemovementjumping), [GameplayCue.Berta.Movement.Jump](gameplay-cue.md#gameplaycuebertamovementjump), [GameplayCue.Berta.Movement.Land](gameplay-cue.md#gameplaycuebertamovementland).

**Important distinctions:** Jump ability identity, movement intervals, transition notifications, optional cooldown, and cosmetic cues are separate contracts. See the manual GA_Jump walkthrough in the integration guide.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Movement.Landed

**Meaning:** Airborne movement transitioned to an accepted landing.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A landing callback emits this once for the landing occurrence.

**Related entries:** [Berta.Ability.Movement.Jump](ability.md#bertaabilitymovementjump), [Berta.Cooldown.Movement.Jump](cooldown.md#bertacooldownmovementjump), [Berta.Event.Movement.Jumped](event.md#bertaeventmovementjumped), [Berta.State.Movement.Falling](state.md#bertastatemovementfalling), [Berta.State.Movement.InAir](state.md#bertastatemovementinair), [Berta.State.Movement.Jumping](state.md#bertastatemovementjumping), [GameplayCue.Berta.Movement.Jump](gameplay-cue.md#gameplaycuebertamovementjump), [GameplayCue.Berta.Movement.Land](gameplay-cue.md#gameplaycuebertamovementland).

**Important distinctions:** Jump ability identity, movement intervals, transition notifications, optional cooldown, and cosmetic cues are separate contracts. See the manual GA_Jump walkthrough in the integration guide.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Movement.StartedSprinting

**Meaning:** The character entered an accepted sprint locomotion regime.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A movement controller announces sprint entry after validation.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Movement.StoppedSprinting

**Meaning:** The character left an accepted sprint locomotion regime.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A movement controller announces sprint exit on exhaustion or release.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Movement.Teleported

**Meaning:** An instantaneous relocation completed successfully.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A teleport controller reports arrival after destination validation.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

## Event.Quest

### Berta.Event.Quest.Accepted

**Meaning:** A quest was accepted for a participant.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A quest authority reports a newly committed quest enrollment.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Quest.Completed

**Meaning:** A quest reached successful completion.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A quest authority announces completion for rewards processing.

**Related entries:** [Berta.Quest.State.Completed](quest.md#bertaqueststatecompleted).

**Important distinctions:** The quest record retains Completed; the event announces the successful transition and must not stand in for the saved record.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Quest.Failed

**Meaning:** A quest transitioned into a committed failure outcome.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A quest authority reports an expired timed quest.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Event.Quest.ObjectiveCompleted

**Meaning:** An individual quest objective reached successful completion.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A quest controller reports which objective was fulfilled.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.
