# Vehicle — 10 tags

Vehicle.Type and Vehicle.Seat classify definitions. Vehicle.State describes a vehicle or a specifically identified occupancy transition. Keep the owner explicit: when two participants enter different seats, a single vehicle-wide Entering tag cannot represent both transitions independently. A project can store state on a seat/occupancy record and expose an aggregate only when useful.

Driver and Passenger do not grant possession or seat authority. Driving does not prove the vehicle is physically moving, and Boosting may coexist with Driving. Disabled may require cleanup of input and propulsion through the vehicle controller. Types are not engine movement-component classes; an amphibious vehicle can have multiple supported classifications under project rules. No boarding, possession, physics, or replication implementation is included.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Vehicle.Seat

### Berta.Vehicle.Seat.Driver

**Meaning:** Classifies a seat that grants primary vehicle-control responsibility.

**Scope and suggested owner:** object/archetype classification; seat definition within a vehicle.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A seat definition identifies the controlling occupant position.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Vehicle.Seat.Passenger

**Meaning:** Classifies a seat intended for an occupant without primary driving control.

**Scope and suggested owner:** object/archetype classification; seat definition within a vehicle.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A boarding rule selects an available passenger position.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

## Vehicle.State

### Berta.Vehicle.State.Boosting

**Meaning:** The vehicle is currently using a temporary propulsion enhancement.

**Scope and suggested owner:** vehicle or explicitly scoped occupancy condition; vehicle controller or a specific participant/seat occupancy record.

**Lifecycle:** Maintain during the corresponding operation; clear or reconcile on completion, cancellation, possession changes, and destruction.

**Example:** A vehicle controller consumes boost reserve while active.

**Implementation responsibility:** The project chooses the state owner and implements boarding, control, propulsion, and networking.

### Berta.Vehicle.State.Disabled

**Meaning:** The vehicle is currently unavailable for normal operation.

**Scope and suggested owner:** vehicle or explicitly scoped occupancy condition; vehicle controller or a specific participant/seat occupancy record.

**Lifecycle:** Maintain during the corresponding operation; clear or reconcile on completion, cancellation, possession changes, and destruction.

**Example:** A vehicle controller rejects driving after a disabling fault.

**Implementation responsibility:** The project chooses the state owner and implements boarding, control, propulsion, and networking.

### Berta.Vehicle.State.Driving

**Meaning:** The vehicle is currently in its controlled driving phase.

**Scope and suggested owner:** vehicle or explicitly scoped occupancy condition; vehicle controller or a specific participant/seat occupancy record.

**Lifecycle:** Maintain during the corresponding operation; clear or reconcile on completion, cancellation, possession changes, and destruction.

**Example:** A vehicle controller routes accepted driver input.

**Implementation responsibility:** The project chooses the state owner and implements boarding, control, propulsion, and networking.

### Berta.Vehicle.State.Entering

**Meaning:** The vehicle interaction is currently transitioning a participant into occupancy.

**Scope and suggested owner:** vehicle or explicitly scoped occupancy condition; vehicle controller or a specific participant/seat occupancy record.

**Lifecycle:** Maintain during the corresponding operation; clear or reconcile on completion, cancellation, possession changes, and destruction.

**Example:** A vehicle controller keeps seat input gated until entry completes.

**Implementation responsibility:** The project chooses the state owner and implements boarding, control, propulsion, and networking.

### Berta.Vehicle.State.Exiting

**Meaning:** The vehicle interaction is currently transitioning a participant out of occupancy.

**Scope and suggested owner:** vehicle or explicitly scoped occupancy condition; vehicle controller or a specific participant/seat occupancy record.

**Lifecycle:** Maintain during the corresponding operation; clear or reconcile on completion, cancellation, possession changes, and destruction.

**Example:** A vehicle controller waits for a safe exit placement.

**Implementation responsibility:** The project chooses the state owner and implements boarding, control, propulsion, and networking.

## Vehicle.Type

### Berta.Vehicle.Type.Air

**Meaning:** Classifies a vehicle intended primarily for aerial travel.

**Scope and suggested owner:** object/archetype classification; vehicle definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A garage filter selects aircraft.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Vehicle.Type.Ground

**Meaning:** Classifies a vehicle intended primarily for land travel.

**Scope and suggested owner:** object/archetype classification; vehicle definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A garage filter selects cars and other land vehicles.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.

### Berta.Vehicle.Type.Water

**Meaning:** Classifies a vehicle intended primarily for water travel.

**Scope and suggested owner:** object/archetype classification; vehicle definition.

**Lifecycle:** Assign as definition metadata. Query during selection or configuration; no activation/removal lifecycle is implied.

**Example:** A vehicle spawner selects a boat-compatible route.

**Implementation responsibility:** The project supplies classification-aware rules and the actual object implementation.
