# Inventory — 8 tags

Inventory.State describes an inventory instance, not the items within it. The inventory authority defines whether capacity means slots, mass, volume, or another limit, and evaluates transitions after committed mutations. Full need not mean OverCapacity; a game can deliberately accept excess capacity and then apply restrictions. Locked refers to access or mutation rules, not necessarily a physically locked container.

Inventory.Event tags identify committed transaction notifications. Emit after a valid mutation and include item identity, quantities, and source/destination where applicable. They are not persistent inventory state and need not use GAS events. Avoid reporting success before a transfer's source and destination agree. The catalog supplies neither transactional storage nor network replication.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Inventory.Event

### Berta.Inventory.Event.ItemAdded

**Meaning:** An inventory transaction committed the addition of an item.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An inventory controller notifies UI after a successful insert.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Inventory.Event.ItemRemoved

**Meaning:** An inventory transaction committed removal of an item.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** An inventory controller notifies UI after consumption.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Inventory.Event.ItemTransferred

**Meaning:** An inventory transaction committed movement between inventories.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A storage controller reports source and destination in its payload.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

### Berta.Inventory.Event.StackChanged

**Meaning:** An inventory transaction changed a stack's quantity.

**Scope and suggested owner:** occurrence notification; the authority that confirms the described transition or committed transaction.

**Lifecycle:** Emit at the documented milestone with contextual payload; do not retain as persistent actor state. Define deduplication and network delivery explicitly.

**Example:** A stack controller reports previous and current counts after merging.

**Implementation responsibility:** The project implements the producer, listener/trigger, payload schema, authority, and delivery policy.

## Inventory.State

### Berta.Inventory.State.Empty

**Meaning:** The inventory currently contains no entries under its counting rules.

**Scope and suggested owner:** inventory-instance condition; inventory authority and its storage record.

**Lifecycle:** Recompute after committed storage/access changes and capacity changes; remove when the predicate is no longer true.

**Example:** An inventory UI displays an empty-storage message.

**Implementation responsibility:** The project supplies capacity units, transactional mutation, access control, and replication.

### Berta.Inventory.State.Full

**Meaning:** The inventory has reached its accepted capacity threshold.

**Scope and suggested owner:** inventory-instance condition; inventory authority and its storage record.

**Lifecycle:** Recompute after committed storage/access changes and capacity changes; remove when the predicate is no longer true.

**Example:** A pickup policy rejects acquisition when no admissible capacity remains.

**Implementation responsibility:** The project supplies capacity units, transactional mutation, access control, and replication.

### Berta.Inventory.State.Locked

**Meaning:** The inventory currently rejects specified mutations or access.

**Scope and suggested owner:** inventory-instance condition; inventory authority and its storage record.

**Lifecycle:** Recompute after committed storage/access changes and capacity changes; remove when the predicate is no longer true.

**Example:** A trade controller locks inventory changes during transaction commit.

**Implementation responsibility:** The project supplies capacity units, transactional mutation, access control, and replication.

### Berta.Inventory.State.OverCapacity

**Meaning:** The inventory currently exceeds its accepted capacity threshold.

**Scope and suggested owner:** inventory-instance condition; inventory authority and its storage record.

**Lifecycle:** Recompute after committed storage/access changes and capacity changes; remove when the predicate is no longer true.

**Example:** An encumbrance policy reacts after a capacity reduction.

**Implementation responsibility:** The project supplies capacity units, transactional mutation, access control, and replication.
