# Naming and semantic conventions

## Stable namespace

Use `Berta.*` for ordinary catalog vocabulary and `GameplayCue.Berta.*` for native Gameplay Cues. The cue root comes first because GAS cue tools and handlers operate in that hierarchy. Keep the approved spelling and PascalCase segments, including `AI`, `UI`, `MainHand`, `OffHand`, `CounterAttack`, and `LockOn`. Dots separate semantic levels; they are not underscores or display punctuation.

Namespaced identifiers are intentionally longer than many generic GAS or Lyra examples. They distinguish ownership in a shared global dictionary and reduce collisions between project, engine sample, and third-party vocabularies. The hierarchy is not a module path, UObject path, asset name, faction system, or network channel. Do not copy an entire sample vocabulary just to reuse a small concept.

Choose nouns or capability names for identifiers, current conditions for State/Status, and completed occurrences for Event where appropriate. Existing approved names remain canonical even when another grammatical form might be equally reasonable. Improve explanations instead of silently renaming a stable identifier.

## Registered versus owned

The dictionary establishes that a name exists and makes it selectable. Runtime ownership belongs to a concrete container or provider. An ASC owns tags through GAS mechanisms; an inventory object may store a plain container. These are distinct stores. Registering `Berta.State.Life.Dead` does not make every actor dead. Assigning `Berta.Ability.Movement.Jump` to an ability's Asset Tags does not add it to the character's owned tags.

A local `FGameplayTagContainer` has no automatic authority, replication, duration, or count semantics. Decide which system exposes it and who may update it. Do not assume arbitrary actor tag variables are automatically discoverable by GAS; use the native ASC or an explicitly implemented tag provider as appropriate.

## Hierarchical matching is directional

Suppose a container contains only `Berta.Ability.Combat.Attack.Light`:

| Query | Result | Reason |
| --- | --- | --- |
| `HasTag(Light)` | true | The exact child is present |
| `HasTag(Attack)` | true | The stored child satisfies a broader parent query |
| `HasTagExact(Attack)` | false | The parent was not explicitly added to that container |
| `HasTagExact(Light)` | true | Exact identity is present |
| `HasTag(Heavy)` | false | A sibling is not implied |

Here Light, Attack, and Heavy refer to the fully qualified entries in the [Ability reference](catalog/ability.md#bertaabilitycombatattack). `FGameplayTag::MatchesTag` follows the same child-to-parent direction. A container holding only the parent does **not** satisfy a query for a child. Granting a parent never grants every descendant.

Unreal builds hierarchy nodes for missing ancestors. The approved catalog also explicitly declares some parents, notably `Berta.Ability.Combat.Attack` and `Berta.Cooldown.Combat.Attack`, because they are useful first-class concepts. The validator counts only INI rows, not those implicit nodes. An implicit parent is available for hierarchy queries but has no dedicated approved declaration/comment in this catalog.

Use exact matching for exact identity or when sibling/parent equivalence would be unsafe. Use hierarchical matching deliberately for groups such as all attack variants. Empty All/Any queries have native logical behavior; do not assume an empty selection means the same thing for both operations. Define empty-filter policy at the calling boundary.

## Semantic boundaries

| Concept | Example | Separate from |
| --- | --- | --- |
| Ability identity | `Berta.Ability.Movement.Jump` | Input, current movement, and cue execution |
| Current condition | `Berta.State.Life.Dead` | `Berta.Event.Life.Died`, the transition occurrence |
| Damage type | `Berta.Damage.Type.Elemental.Fire` | `Berta.Status.Burning`, the recipient condition |
| Cosmetic feedback | `GameplayCue.Berta.Status.Burning` | Authoritative damage and condition state |
| Potential restriction | `Berta.Cooldown.Movement.Dash` | Ability identity and actual timer/effect implementation |
| Resource concept | `Berta.Resource.Health` | Numeric Gameplay Attribute or current value |
| Input intent | `Berta.Input.Movement.Jump` | Enhanced Input Action binding and ability grant |
| Object type | `Berta.Weapon.Type.Rifle` | Weapon condition, character condition, and AI role |
| Relation predicate | `Berta.Targeting.Relation.Enemy` | Intrinsic faction identity |
| Surface classification | `Berta.World.Surface.Metal` | Native Physical Material or physical-surface enum |
| Persistent quest outcome | `Berta.Quest.State.Completed` | `Berta.Event.Quest.Completed`, its announcement |

The [State](catalog/state.md) and [Status](catalog/status.md) entries explain overlapping cases individually. InAir, Jumping, and Falling can overlap. Idle can coexist with Grounded. Slowed does not select Walking. A regeneration passive can remain granted without a current Regenerating condition. Shielded does not hold a Shield amount. Rooted, Stunned, and InputLocked require separate restriction policies. HitConfirmed and HitReceived describe contact; DamageDealt and DamageReceived describe resolved damage.

Do not claim a family is globally mutually exclusive. A project may enforce one life-state selection, one current weapon phase, or one camera policy when that invariant helps its design. Optional objectives, concurrent status conditions, weapon classifications, and overlapping world zones demonstrate why blanket exclusivity would be wrong. Document exclusivity next to the actual project owner and test its transitions.

## Compose dimensions

Describe a projectile fire hit on the head using three existing dimensions: `Berta.Damage.Source.Projectile`, `Berta.Damage.Type.Elemental.Fire`, and `Berta.Damage.HitZone.Head`. Retain numeric magnitude and hit data in the actual payload. Do not create a new identifier for each damage-type/source/body-part combination.

Likewise combine `Berta.Item.Type.Weapon`, `Berta.Item.Rarity.Rare`, and `Berta.Weapon.Type.Rifle` on appropriate item metadata. A slot label describes equipment position; it does not replace the item identity or instance handle. Use project identifiers for individual quests, items, factions, and seats rather than expanding a reusable vocabulary into every content record.

## Controlled extension and collisions

Search the full catalog and project sources for equivalent meaning before adding a tag. Prefer project-specific namespaces such as `MyGame.*` for mechanics and content belonging to one game. Do not redeclare existing Berta names in project config or native C++ declarations just to make them visible. Different sources can contribute the same dictionary name, obscuring ownership and comments even when runtime matching still seems to work.

Inspect source basenames as well as tag names, including enabled third-party plugins. A new semantic collision should lead to a documented mapping or project integration decision; never reinterpret or rename the approved vocabulary silently. Stable names are serialized API. Use the [migration guide](extension-guide.md#deprecating-and-renaming) for changes that affect existing references.
