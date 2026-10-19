# Ownership, lifetime, and authority

The dictionary owns names. A project system owns the truth represented by each use of those names. Make that distinction before choosing whether to store a tag on an ASC, an object definition, a participant record, or a temporary notification payload.

## Select the right owner

| Kind | Typical owner | Lifetime contract |
| --- | --- | --- |
| Ability identifier | Gameplay Ability defaults/spec metadata | Stable identity across activations |
| Character movement condition | Integration derived from native movement, exposed on ASC/tag provider | Exactly while the chosen movement predicate is true |
| Effect-driven condition | Gameplay Effect or another condition provider | Active contributing source, including expiry and removal |
| Cooldown restriction | Duration Gameplay Effect on ASC | Chosen committed cooldown interval |
| Damage/effect classification | Damage payload or effect definition/spec metadata | Operation or definition scope |
| Resource concept | Resource descriptor/provider mapping | Stable semantic key; value has a separate lifetime |
| Input intent | Local input-routing data | Binding/intent scope, with values carried separately |
| Item/equipment classification | Definition, slot rule, or instance metadata | Definition scope or explicit instance change |
| Weapon operation | Weapon controller | Accepted firing/reload/malfunction phase |
| Interaction availability | Specific target affordance | Recomputed eligibility for that option |
| Inventory transaction event | Inventory authority | One committed occurrence |
| Target relation/filter | Query configuration and resolver | Relative to this query origin and evaluation time |
| AI behavior | Controller/brain | Current selected decision mode |
| Quest/progression state | Participant-specific saved record | Persistent record condition until transition |
| Camera/UI context | Local-player director/router | Selected or stacked local presentation context |
| Gameplay Cue | Cue implementation and explicit producer | One-shot or active cosmetic lifetime |

Do not store unqualified quest-completion or inventory-state labels on a character when several records need different values. Keep record identity alongside the tags. A quest journal record, a weapon instance, and the character may all expose tags while remaining separate owners.

## One source of truth, multiple consumers

For a derived movement condition, native movement is the source and the tag representation is a projection. Use callbacks and explicit initialization/reconciliation rather than letting arbitrary consumers toggle it. Animation, UI, targeting, and abilities can read the projection. They should not independently decide that the actor landed because an animation finished or an ability ended.

For an effect-driven condition, GAS's active effect contribution may be the source of truth. Do not maintain a second boolean that can disagree after cleansing or expiry. If another non-GAS system is authoritative, expose its condition through a deliberately owned adapter and document how the native tag count is maintained. The catalog supplies no adapter.

An availability or classification tag is not a transaction lock. A target can become unavailable between discovery and execution; validate again at the authority boundary. Likewise an InRange filter result can become stale before damage application.

## Multiple contributors and cleanup

Two effects may contribute the same condition. The effective tag must remain present while a valid source still grants it. A plain container has set membership, not source ownership or stack counts. Removing a shared tag blindly when one source expires can hide another still-active source. Prefer the native effect/tag-count mechanism for GAS-owned contributions and inspect effects when exact provenance matters.

Every mutable state needs an exit path for normal completion and relevant interruptions. Review cancellation, effect removal, death, respawn, pawn/ASC replacement, equipment removal, target destruction, and session teardown. Only handle cases relevant to the actual owner; do not add lifecycle machinery to a static classification definition.

Recommended project invariants can include exactly one selected life state, one current fire mode, or one interaction reservation. These are not universal catalog rules. Alive and Downed may deliberately coexist; Equipped and Broken may coexist; Driving and Boosting may coexist; Optional and Active objectives commonly coexist. Make the actual predicate and owner explicit before imposing exclusivity.

## State and occurrence are different

Keep `Berta.State.Life.Dead` while the life authority says dead. Emit `Berta.Event.Life.Died` when the selected death milestone occurs. A new listener must query state to discover an already-dead actor instead of expecting a past event to replay. Do not add an event tag to the ASC just to remember that it happened; persist a proper record if historical facts matter.

Emit notifications at committed milestones. A failed pickup does not emit `Berta.Event.Interaction.PickedUp`. A contact that resolves to zero damage can emit a hit notification without a positive-damage notification, depending on the documented project policy. Include enough payload context to make the result interpretable; tags do not carry numeric amounts or object identity.

GAS events are delivered to the selected ASC through native helpers and explicit listeners. No catalog-level queue, persistence, delivery guarantee, or automatic network routing exists. If reliable authoritative processing is needed, use the gameplay authority and its validated transaction/state model.

## Authority and networking

Registration makes the same names available on a machine. It does not select server authority, grant replicated tags, or transmit a local container. For GAS, inspect the particular mechanism used: active effect tags, activation-owned tags, loose tags, and project-replicated containers have different configuration and lifetime considerations. Do not assume a plain loose-tag add is a complete replication policy.

Document whether a state is authoritative, predicted, locally presented, or observed from replicated state. An ASC located on PlayerState may outlive a pawn; rebind any movement projection to the new pawn rather than retaining stale references. All such integration belongs to the project, and ordinary UObject/component lifetime rules still apply.

Fast Gameplay Tag replication uses dictionary indices. Keep client/server dictionaries compatible, including plugin enablement, catalog release, project tags, and build configuration. A tag's existence on both machines is not proof their network dictionaries are synchronized. Review the project's native replication settings and test the actual deployed combination.

## Cosmetics cannot be gameplay authority

Gameplay Cues identify feedback. Their execution path can be predicted, suppressed, locally handled, or transported through native GAS mechanisms. Do not let a cue decide whether damage happened, a pickup committed, a character landed, or a shield exists. The authoritative state and transaction must remain correct without visual or audio output. Cue cleanup removes presentation; condition cleanup belongs to the actual condition provider.

## Design review questions

Before adopting a mutable tag, answer these questions in the project implementation:

1. Which exact object or record owns the predicate, and which system may change it?
2. Is it a classification, current condition, requested filter, or occurrence?
3. What establishes truth, and what clears or recomputes it?
4. Can multiple sources contribute, and what prevents premature removal?
5. What do authority, prediction, and observers see?
6. Which interruption paths matter for this owner?
7. Are exact and hierarchical queries both intentional?

For a static classification, the answers can be simple definition metadata. Do not manufacture activation or cleanup rules for a rarity, surface, or equipment-slot name.
