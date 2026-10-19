# AI — 17 tags

AI.Behavior identifies a current decision mode owned by an AI controller, brain, behavior tree integration, or state machine. AI.Perception labels stimulus or sense metadata; it does not configure UAIPerceptionComponent. AI.Role is relatively stable design metadata for a spawn definition or agent archetype. Keep those three scopes distinct.

Role.Ranged does not mean the agent owns a rifle: it may use a bow or spell. Behavior.Attack does not identify a particular Gameplay Ability, and Behavior.Guard is an active task rather than proof of Role.Guard. A project defines behavior exclusivity and transition cleanup, including possession and death. The catalog neither changes Blackboard values nor selects behavior tree branches. Perception producers validate senses and emit Event.AI notifications explicitly when wanted.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## AI.Behavior

### Berta.AI.Behavior.Attack

**Meaning:** The AI is currently pursuing an offensive behavior.

**Scope and suggested owner:** current AI decision mode; AI controller, brain, behavior tree integration, or state machine.

**Lifecycle:** Maintain while that decision mode is selected; reconcile transitions, controller replacement, and shutdown.

**Example:** A controller chooses an attack task after range validation.

**Implementation responsibility:** The project implements decisions and any Blackboard/tag synchronization.

### Berta.AI.Behavior.Chase

**Meaning:** The AI is currently pursuing a known target.

**Scope and suggested owner:** current AI decision mode; AI controller, brain, behavior tree integration, or state machine.

**Lifecycle:** Maintain while that decision mode is selected; reconcile transitions, controller replacement, and shutdown.

**Example:** A controller selects pursuit movement toward a fleeing opponent.

**Implementation responsibility:** The project implements decisions and any Blackboard/tag synchronization.

### Berta.AI.Behavior.Flee

**Meaning:** The AI is currently prioritizing escape from danger.

**Scope and suggested owner:** current AI decision mode; AI controller, brain, behavior tree integration, or state machine.

**Lifecycle:** Maintain while that decision mode is selected; reconcile transitions, controller replacement, and shutdown.

**Example:** A controller chooses an escape destination under a fear policy.

**Implementation responsibility:** The project implements decisions and any Blackboard/tag synchronization.

### Berta.AI.Behavior.Guard

**Meaning:** The AI is currently protecting a location or entity.

**Scope and suggested owner:** current AI decision mode; AI controller, brain, behavior tree integration, or state machine.

**Lifecycle:** Maintain while that decision mode is selected; reconcile transitions, controller replacement, and shutdown.

**Example:** A controller maintains a defensive post.

**Related entries:** [Berta.AI.Role.Guard](ai.md#bertaairoleguard).

**Important distinctions:** The archetype role can remain Guard while the current behavior is Search, Chase, or another mode.

**Implementation responsibility:** The project implements decisions and any Blackboard/tag synchronization.

### Berta.AI.Behavior.Idle

**Meaning:** The AI is currently in a waiting or inactive decision mode.

**Scope and suggested owner:** current AI decision mode; AI controller, brain, behavior tree integration, or state machine.

**Lifecycle:** Maintain while that decision mode is selected; reconcile transitions, controller replacement, and shutdown.

**Example:** A controller selects an idle behavior branch while no task is pending.

**Implementation responsibility:** The project implements decisions and any Blackboard/tag synchronization.

### Berta.AI.Behavior.Investigate

**Meaning:** The AI is currently examining a specific uncertain stimulus.

**Scope and suggested owner:** current AI decision mode; AI controller, brain, behavior tree integration, or state machine.

**Lifecycle:** Maintain while that decision mode is selected; reconcile transitions, controller replacement, and shutdown.

**Example:** A controller approaches a reported noise location.

**Implementation responsibility:** The project implements decisions and any Blackboard/tag synchronization.

### Berta.AI.Behavior.Patrol

**Meaning:** The AI is currently following a routine monitoring route.

**Scope and suggested owner:** current AI decision mode; AI controller, brain, behavior tree integration, or state machine.

**Lifecycle:** Maintain while that decision mode is selected; reconcile transitions, controller replacement, and shutdown.

**Example:** A controller selects its patrol branch between alerts.

**Implementation responsibility:** The project implements decisions and any Blackboard/tag synchronization.

### Berta.AI.Behavior.Retreat

**Meaning:** The AI is currently withdrawing toward a tactically preferable position.

**Scope and suggested owner:** current AI decision mode; AI controller, brain, behavior tree integration, or state machine.

**Lifecycle:** Maintain while that decision mode is selected; reconcile transitions, controller replacement, and shutdown.

**Example:** A controller repositions behind cover while maintaining engagement.

**Implementation responsibility:** The project implements decisions and any Blackboard/tag synchronization.

### Berta.AI.Behavior.Search

**Meaning:** The AI is currently looking for a target without a confirmed current location.

**Scope and suggested owner:** current AI decision mode; AI controller, brain, behavior tree integration, or state machine.

**Lifecycle:** Maintain while that decision mode is selected; reconcile transitions, controller replacement, and shutdown.

**Example:** A controller searches after losing visual contact.

**Implementation responsibility:** The project implements decisions and any Blackboard/tag synchronization.

## AI.Perception

### Berta.AI.Perception.Damage

**Meaning:** Classifies perception information originating from received harm.

**Scope and suggested owner:** perception stimulus or sense classification; AI perception adapter or sense configuration metadata.

**Lifecycle:** Assign to stimulus/definition metadata when its origin is known; no persistent actor condition is implied.

**Example:** A perception adapter labels an attack-induced stimulus.

**Implementation responsibility:** The project configures native senses, validates stimuli, and dispatches optional notifications.

### Berta.AI.Perception.Hearing

**Meaning:** Classifies auditory perception information or a configured hearing sense.

**Scope and suggested owner:** perception stimulus or sense classification; AI perception adapter or sense configuration metadata.

**Lifecycle:** Assign to stimulus/definition metadata when its origin is known; no persistent actor condition is implied.

**Example:** A perception adapter labels a sound stimulus.

**Implementation responsibility:** The project configures native senses, validates stimuli, and dispatches optional notifications.

### Berta.AI.Perception.Sight

**Meaning:** Classifies visual perception information or a configured visual sense.

**Scope and suggested owner:** perception stimulus or sense classification; AI perception adapter or sense configuration metadata.

**Lifecycle:** Assign to stimulus/definition metadata when its origin is known; no persistent actor condition is implied.

**Example:** A perception adapter labels a line-of-sight stimulus.

**Implementation responsibility:** The project configures native senses, validates stimuli, and dispatches optional notifications.

## AI.Role

### Berta.AI.Role.Civilian

**Meaning:** Classifies an AI design role centered on noncombat participation.

**Scope and suggested owner:** object/archetype classification; AI archetype or spawn definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A population spawner selects civilian behavior defaults.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.AI.Role.Guard

**Meaning:** Classifies an AI design role centered on protection or security.

**Scope and suggested owner:** object/archetype classification; AI archetype or spawn definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A spawn definition selects guard behavior capabilities.

**Related entries:** [Berta.AI.Behavior.Guard](ai.md#bertaaibehaviorguard).

**Important distinctions:** The archetype role can remain Guard while the current behavior is Search, Chase, or another mode.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.AI.Role.Melee

**Meaning:** Classifies an AI design role centered on close-range combat.

**Scope and suggested owner:** object/archetype classification; AI archetype or spawn definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** An encounter composer assigns a close-combat role.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.AI.Role.Ranged

**Meaning:** Classifies an AI design role centered on distant engagement.

**Scope and suggested owner:** object/archetype classification; AI archetype or spawn definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** An encounter composer selects a ranged role that may use a bow or rifle.

**Related entries:** [Berta.Weapon.Type.Rifle](weapon.md#bertaweapontyperifle).

**Important distinctions:** Ranged is an agent design role; Rifle is weapon classification. A ranged agent may use a spell or bow and a rifle does not determine its role.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.AI.Role.Support

**Meaning:** Classifies an AI design role centered on assisting others.

**Scope and suggested owner:** object/archetype classification; AI archetype or spawn definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** An encounter composer selects a healer or buffer role.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.
