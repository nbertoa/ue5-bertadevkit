# Input — 17 tags

Input tags label player intent for routing, remapping, and presentation. Store them in project binding data or an input router. They do not create Enhanced Input Actions, Mapping Contexts, trigger phases, or ability bindings. A Move label also does not store an axis value: retain the actual input payload separately.

Input.Movement.Jump and Ability.Movement.Jump have separate roles. The former identifies an intent; the latter identifies a capability selected by project routing. GAS Companion's binding system still requires its documented Input Action and ability configuration. Do not assume adding this tag to a GSC ability establishes a binding. UI input routing must explicitly handle focus, priorities, consumption, and local-player scope.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Input.Combat

### Berta.Input.Combat.Aim

**Meaning:** Represents a request for deliberate weapon or attack aiming.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** An input router labels a held aim action.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Combat.Attack

**Meaning:** Represents a general offensive-action request.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** A binding profile labels the primary melee input.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Combat.Block

**Meaning:** Represents a request to defend with a guard.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** A binding profile labels a held defensive input.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Combat.Fire

**Meaning:** Represents a request to discharge the current weapon.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** A binding profile labels the weapon trigger action.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Combat.Parry

**Meaning:** Represents a request for a timed defensive action.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** An input router labels a parry button separately from block.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Combat.Reload

**Meaning:** Represents a request to replenish usable ammunition.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** An input router labels the reload button.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

## Input.Interaction

### Berta.Input.Interaction.Interact

**Meaning:** Represents a request for a contextual interaction.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** An input router labels the nearby-object interaction button.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Interaction.Use

**Meaning:** Represents a request to invoke an object's function.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** A binding profile labels the selected consumable action.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

## Input.Movement

### Berta.Input.Movement.Crouch

**Meaning:** Represents a player's crouch request.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** A binding profile labels a toggle-crouch action.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Movement.Dash

**Meaning:** Represents a player's dash request.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** A binding table labels a dash button independently of its ability class.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Movement.Dodge

**Meaning:** Represents a player's evasion request.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** An input router labels a directional dodge action.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Movement.Jump

**Meaning:** Represents a player's jump request.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** An input router maps IA_Jump to this label before selecting an ability.

**Related entries:** [Berta.Ability.Movement.Jump](ability.md#bertaabilitymovementjump).

**Important distinctions:** The input label does not bind an Enhanced Input Action or select/grant an ability; routing is explicit project configuration.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Movement.Look

**Meaning:** Represents a player's requested change of view orientation.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** A settings UI labels the action used for camera look.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Movement.Move

**Meaning:** Represents a player's requested direction or magnitude of locomotion.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** A project binding table associates IA_Move with a movement input label.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.Movement.Sprint

**Meaning:** Represents a player's sprint request.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** An input router labels a held sprint action.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

## Input.UI

### Berta.Input.UI.Inventory

**Meaning:** Represents a request to access inventory presentation.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** A UI router labels its inventory-toggle action.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.

### Berta.Input.UI.Pause

**Meaning:** Represents a request to enter or leave the pause flow.

**Scope and suggested owner:** input-intent label; project input binding data, local input router, or remapping UI.

**Lifecycle:** Assign in routing/binding data. Process input values and trigger phases separately; selecting the tag creates no binding.

**Example:** A UI router labels its pause action without pausing the world automatically.

**Implementation responsibility:** The project configures Enhanced Input and explicit routing to gameplay or UI.
