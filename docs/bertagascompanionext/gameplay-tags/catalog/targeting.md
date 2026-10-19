# Targeting — 18 tags

Targeting.Mode describes a selection strategy or geometry; Relation describes a relation from the query origin to a candidate; Filter describes a predicate to evaluate during selection. Store the requested constraints on an ability, target definition, or query configuration. A project targeting resolver interprets them and supplies geometry, range, visibility traces, team rules, and life-state access.

Relation.Enemy is not an intrinsic faction label on every actor: the same candidate can be an ally to one origin and an enemy to another. Relation.Any removes only the relation constraint, not Alive or InRange. Mode.Self chooses a strategy while Relation.Self is a relational predicate. Filters are requests, not proof the candidate passed them; revalidate sensitive conditions at authoritative application time. These labels do not install native Targeting System tasks or GSC TargetTypes.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Targeting.Filter

### Berta.Targeting.Filter.Alive

**Meaning:** Requests candidates accepted as living by the target's life authority.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A healing query excludes corpses under the game's life policy.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Filter.Dead

**Meaning:** Requests candidates accepted as dead by the target's life authority.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A resurrection query selects corpses.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Filter.Downed

**Meaning:** Requests candidates eligible under the project's downed-state policy.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A revive query selects incapacitated teammates.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Filter.InRange

**Meaning:** Requests candidates within the operation's configured effective range.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A melee query rejects targets outside its reach.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Filter.Visible

**Meaning:** Requests candidates passing a visibility test for the query origin.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A targeting query performs line-of-sight evaluation.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

## Targeting.Mode

### Berta.Targeting.Mode.Area

**Meaning:** Classifies a targeting shape covering a spatial region.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A blast definition requests candidates inside a radius.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Mode.Chain

**Meaning:** Classifies a targeting operation that traverses successive recipients.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A chain spell selects the next eligible nearby candidate.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Mode.Cone

**Meaning:** Classifies a targeting shape widening along a direction.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A breath attack requests candidates within an angular sector.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Mode.GroundPoint

**Meaning:** Classifies a targeting operation that chooses a world-space placement point.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A deployment ability requests a valid terrain location.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Mode.Line

**Meaning:** Classifies a targeting shape along a directed segment or corridor.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A beam requests candidates along its trace path.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Mode.Projectile

**Meaning:** Classifies a targeting operation based on a projectile trajectory or contact.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A launcher definition requests projectile-based target resolution.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Mode.Self

**Meaning:** Classifies a targeting shape that selects the initiating entity.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A heal definition selects the caster as its recipient.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Mode.SingleTarget

**Meaning:** Classifies a targeting operation that selects one candidate.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A spell definition requests one validated enemy.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

## Targeting.Relation

### Berta.Targeting.Relation.Ally

**Meaning:** Selects candidates considered friendly relative to the query origin.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A heal filter accepts allies according to team rules.

**Related entries:** [Berta.Targeting.Relation.Enemy](targeting.md#bertatargetingrelationenemy), [Berta.Targeting.Relation.Neutral](targeting.md#bertatargetingrelationneutral).

**Important distinctions:** Resolve the relation against the querying origin. These are selection semantics, not intrinsic global faction assignments.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Relation.Any

**Meaning:** Declares that no relation restriction is requested.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** An area query accepts every relation before applying other filters.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Relation.Enemy

**Meaning:** Selects candidates considered hostile relative to the query origin.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A damage filter resolves hostility for this particular attacker.

**Related entries:** [Berta.Targeting.Relation.Ally](targeting.md#bertatargetingrelationally), [Berta.Targeting.Relation.Neutral](targeting.md#bertatargetingrelationneutral).

**Important distinctions:** Resolve the relation against the querying origin. These are selection semantics, not intrinsic global faction assignments.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Relation.Neutral

**Meaning:** Selects candidates considered neither allied nor hostile relative to the query origin.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** An interaction filter accepts neutral participants.

**Related entries:** [Berta.Targeting.Relation.Ally](targeting.md#bertatargetingrelationally), [Berta.Targeting.Relation.Enemy](targeting.md#bertatargetingrelationenemy).

**Important distinctions:** Resolve the relation against the querying origin. These are selection semantics, not intrinsic global faction assignments.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.

### Berta.Targeting.Relation.Self

**Meaning:** Selects the querying entity as a relation to itself.

**Scope and suggested owner:** target-selection configuration or predicate; ability/target definition or targeting query configuration interpreted by a resolver.

**Lifecycle:** Select for a query, then evaluate against the origin and candidates. Revalidate sensitive results when applying gameplay.

**Example:** A targeting rule limits a buff to its instigator.

**Implementation responsibility:** The project implements geometry, teams, life predicates, visibility, range, and authoritative validation.
