# Resource — 10 tags

Resource labels identify which resource concept a system refers to. They are useful keys in a project-owned resource-to-provider mapping, costs, UI descriptors, or inventory data. They are not numeric Gameplay Attributes, do not create Attribute Sets, and cannot be added or subtracted as amounts. Query the mapped attribute or provider for values.

Different projects may implement Ammo as an attribute, an inventory stack, or weapon-local data; the vocabulary does not choose ownership. Health and Shield can have current and maximum values without adding corresponding catalog tags. Heat can increase toward an undesirable limit rather than acting like a spendable pool. Status.Shielded is a current protective condition, whereas Resource.Shield names the reserve. Replication and reconciliation belong to the numeric provider.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Resource

### Berta.Resource.Ammo

**Meaning:** Identifies an expendable ammunition supply concept.

**Scope and suggested owner:** resource-concept classification; resource descriptor or project mapping to an attribute, inventory, or other numeric provider.

**Lifecycle:** Use as a stable mapping/filter key. The tag has no resource amount, expenditure, refill, or numeric replication lifetime.

**Example:** A resource display locates the weapon's ammunition provider.

**Implementation responsibility:** The project owns the numeric provider, value access, costs, bounds, and replication.

### Berta.Resource.Energy

**Meaning:** Identifies a general operational power reserve.

**Scope and suggested owner:** resource-concept classification; resource descriptor or project mapping to an attribute, inventory, or other numeric provider.

**Lifecycle:** Use as a stable mapping/filter key. The tag has no resource amount, expenditure, refill, or numeric replication lifetime.

**Example:** A device UI maps this label to its battery value.

**Implementation responsibility:** The project owns the numeric provider, value access, costs, bounds, and replication.

### Berta.Resource.Focus

**Meaning:** Identifies a concentration or precision expenditure pool.

**Scope and suggested owner:** resource-concept classification; resource descriptor or project mapping to an attribute, inventory, or other numeric provider.

**Lifecycle:** Use as a stable mapping/filter key. The tag has no resource amount, expenditure, refill, or numeric replication lifetime.

**Example:** An aimed-shot UI selects a focus reserve.

**Implementation responsibility:** The project owns the numeric provider, value access, costs, bounds, and replication.

### Berta.Resource.Health

**Meaning:** Identifies the resource concept representing vitality or damage tolerance.

**Scope and suggested owner:** resource-concept classification; resource descriptor or project mapping to an attribute, inventory, or other numeric provider.

**Lifecycle:** Use as a stable mapping/filter key. The tag has no resource amount, expenditure, refill, or numeric replication lifetime.

**Example:** A resource-to-attribute map selects a health attribute for a HUD bar.

**Implementation responsibility:** The project owns the numeric provider, value access, costs, bounds, and replication.

### Berta.Resource.Heat

**Meaning:** Identifies accumulated thermal load as a resource concept.

**Scope and suggested owner:** resource-concept classification; resource descriptor or project mapping to an attribute, inventory, or other numeric provider.

**Lifecycle:** Use as a stable mapping/filter key. The tag has no resource amount, expenditure, refill, or numeric replication lifetime.

**Example:** A weapon HUD locates a heat value that rises during firing.

**Implementation responsibility:** The project owns the numeric provider, value access, costs, bounds, and replication.

### Berta.Resource.Mana

**Meaning:** Identifies the resource concept representing a magical expenditure pool.

**Scope and suggested owner:** resource-concept classification; resource descriptor or project mapping to an attribute, inventory, or other numeric provider.

**Lifecycle:** Use as a stable mapping/filter key. The tag has no resource amount, expenditure, refill, or numeric replication lifetime.

**Example:** A spell-cost UI selects its mana display.

**Implementation responsibility:** The project owns the numeric provider, value access, costs, bounds, and replication.

### Berta.Resource.Oxygen

**Meaning:** Identifies a breathable-air reserve concept.

**Scope and suggested owner:** resource-concept classification; resource descriptor or project mapping to an attribute, inventory, or other numeric provider.

**Lifecycle:** Use as a stable mapping/filter key. The tag has no resource amount, expenditure, refill, or numeric replication lifetime.

**Example:** An underwater HUD locates the actor's oxygen value.

**Implementation responsibility:** The project owns the numeric provider, value access, costs, bounds, and replication.

### Berta.Resource.Rage

**Meaning:** Identifies an aggression-driven expenditure pool.

**Scope and suggested owner:** resource-concept classification; resource descriptor or project mapping to an attribute, inventory, or other numeric provider.

**Lifecycle:** Use as a stable mapping/filter key. The tag has no resource amount, expenditure, refill, or numeric replication lifetime.

**Example:** A combat cost policy selects the rage meter.

**Implementation responsibility:** The project owns the numeric provider, value access, costs, bounds, and replication.

### Berta.Resource.Shield

**Meaning:** Identifies the resource concept representing a depletable protective reserve.

**Scope and suggested owner:** resource-concept classification; resource descriptor or project mapping to an attribute, inventory, or other numeric provider.

**Lifecycle:** Use as a stable mapping/filter key. The tag has no resource amount, expenditure, refill, or numeric replication lifetime.

**Example:** A HUD binding selects a shield attribute independently of shielded status.

**Related entries:** [Berta.Status.Shielded](status.md#bertastatusshielded).

**Important distinctions:** Shielded is a protective condition; Resource.Shield is a numeric-provider classification. The project decides whether a positive reserve implies that condition.

**Implementation responsibility:** The project owns the numeric provider, value access, costs, bounds, and replication.

### Berta.Resource.Stamina

**Meaning:** Identifies the resource concept representing physical exertion capacity.

**Scope and suggested owner:** resource-concept classification; resource descriptor or project mapping to an attribute, inventory, or other numeric provider.

**Lifecycle:** Use as a stable mapping/filter key. The tag has no resource amount, expenditure, refill, or numeric replication lifetime.

**Example:** A cost policy maps sprint expenditure to a stamina attribute.

**Implementation responsibility:** The project owns the numeric provider, value access, costs, bounds, and replication.
