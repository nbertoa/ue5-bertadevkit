# Interaction — 16 tags

Interaction.Type classifies affordances offered by a target or interaction definition. Interaction.Availability describes the current eligibility of a particular affordance, which need not equal the eligibility of every action on that object. A locked chest may permit Inspect while rejecting Open. The project chooses whether availability tags form an exclusive set.

Keep target affordance data on the interactable or interaction system, character engagement state on the participant, and occurrence notifications in Event.Interaction. An Interact ability can invoke different affordances without changing its identifier. The authority validates reach, permissions, reservation, and transaction success; labels never execute an interaction. Busy and Locked identify different reasons for rejection, and either may change without completing an interaction.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Interaction.Availability

### Berta.Interaction.Availability.Available

**Meaning:** The affordance currently accepts eligible requests.

**Scope and suggested owner:** current availability of a specific affordance; interaction authority for the target option.

**Lifecycle:** Recompute when access, occupancy, power, or prerequisites change; scope to the affected option rather than every action on the object.

**Example:** A door offers interaction after its prerequisites pass.

**Implementation responsibility:** The project defines availability precedence and validates requests at execution time.

### Berta.Interaction.Availability.Busy

**Meaning:** The affordance is occupied by a current operation or participant.

**Scope and suggested owner:** current availability of a specific affordance; interaction authority for the target option.

**Lifecycle:** Recompute when access, occupancy, power, or prerequisites change; scope to the affected option rather than every action on the object.

**Example:** A terminal temporarily rejects a second user.

**Implementation responsibility:** The project defines availability precedence and validates requests at execution time.

### Berta.Interaction.Availability.Disabled

**Meaning:** The affordance is administratively or mechanically unavailable.

**Scope and suggested owner:** current availability of a specific affordance; interaction authority for the target option.

**Lifecycle:** Recompute when access, occupancy, power, or prerequisites change; scope to the affected option rather than every action on the object.

**Example:** A powered-off switch hides its usable prompt.

**Implementation responsibility:** The project defines availability precedence and validates requests at execution time.

### Berta.Interaction.Availability.Locked

**Meaning:** The affordance requires an unmet access or unlocking condition.

**Scope and suggested owner:** current availability of a specific affordance; interaction authority for the target option.

**Lifecycle:** Recompute when access, occupancy, power, or prerequisites change; scope to the affected option rather than every action on the object.

**Example:** A chest displays a key requirement.

**Implementation responsibility:** The project defines availability precedence and validates requests at execution time.

## Interaction.Type

### Berta.Interaction.Type.Close

**Meaning:** Classifies an affordance that closes a target.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** An open hatch advertises a closing action.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.

### Berta.Interaction.Type.Craft

**Meaning:** Classifies an affordance that transforms ingredients into a result.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** A workbench advertises a crafting interaction.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.

### Berta.Interaction.Type.Drop

**Meaning:** Classifies an affordance that releases a carried object.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** A carrying menu advertises a crate-release action.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.

### Berta.Interaction.Type.Harvest

**Meaning:** Classifies an affordance that gathers resources from a source.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** A plant advertises resource gathering.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.

### Berta.Interaction.Type.Inspect

**Meaning:** Classifies an affordance that reveals information about a target.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** A clue advertises examination without collection.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.

### Berta.Interaction.Type.Loot

**Meaning:** Classifies an affordance that accesses a target's stored possessions.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** A corpse advertises loot access under project permissions.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.

### Berta.Interaction.Type.Open

**Meaning:** Classifies an affordance that opens a target.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** A closed chest advertises an opening action.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.

### Berta.Interaction.Type.Pickup

**Meaning:** Classifies an affordance that acquires an object.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** A dropped item advertises pickup eligibility.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.

### Berta.Interaction.Type.Revive

**Meaning:** Classifies an affordance that restores a downed participant.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** A teammate advertises recovery interaction while downed.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.

### Berta.Interaction.Type.Talk

**Meaning:** Classifies an affordance that starts conversation.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** An NPC advertises a dialogue action.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.

### Berta.Interaction.Type.Trade

**Meaning:** Classifies an affordance that exchanges goods or value.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** A merchant advertises a transaction interface.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.

### Berta.Interaction.Type.Use

**Meaning:** Classifies an affordance that invokes an object's function.

**Scope and suggested owner:** target-affordance classification; interactable definition or a specific interaction option.

**Lifecycle:** Assign as affordance metadata and use during option discovery. Availability and session state are evaluated separately.

**Example:** A switch advertises a use interaction to the contextual menu.

**Implementation responsibility:** The project implements target validation, interaction execution, and authority.
