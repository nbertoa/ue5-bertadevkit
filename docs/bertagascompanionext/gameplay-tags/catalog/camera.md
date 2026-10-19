# Camera — 8 tags

Camera.Mode labels camera policies or requested/current camera selections. Store it on camera profiles or the local camera director rather than assuming it is character-owned gameplay state. A project should distinguish a requested mode from the successfully selected mode and define priority during blends, dialogue, spectating, and vehicle transitions.

Aiming camera mode is not proof of State.Combat.Aiming. A camera can zoom for inspection without activating an Aim ability; a character can aim under a different camera policy. These labels do not instantiate camera assets, change view targets, or integrate a third-party camera plugin automatically. Camera ownership is commonly local-player-specific; networked gameplay must not derive authoritative hits or eligibility solely from this presentation label.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Camera.Mode

### Berta.Camera.Mode.Aiming

**Meaning:** Classifies a camera policy specialized for deliberate aiming.

**Scope and suggested owner:** camera-policy classification; camera profile or local camera director selection.

**Lifecycle:** Assign to profile metadata or track the selected policy; define how requests, priorities, and blends affect the current selection.

**Example:** A camera director selects zoom and framing for aimed fire.

**Related entries:** [Berta.Ability.Combat.Aim](ability.md#bertaabilitycombataim), [Berta.State.Combat.Aiming](state.md#bertastatecombataiming).

**Important distinctions:** Camera policy, character action condition, and ability identity are independent; the project coordinates them explicitly.

**Implementation responsibility:** The project implements camera assets, evaluation, transitions, and local-player ownership.

### Berta.Camera.Mode.Cinematic

**Meaning:** Classifies a camera policy controlled for authored presentation.

**Scope and suggested owner:** camera-policy classification; camera profile or local camera director selection.

**Lifecycle:** Assign to profile metadata or track the selected policy; define how requests, priorities, and blends affect the current selection.

**Example:** A camera director selects a cutscene policy.

**Implementation responsibility:** The project implements camera assets, evaluation, transitions, and local-player ownership.

### Berta.Camera.Mode.Dialogue

**Meaning:** Classifies a camera policy specialized for conversation framing.

**Scope and suggested owner:** camera-policy classification; camera profile or local camera director selection.

**Lifecycle:** Assign to profile metadata or track the selected policy; define how requests, priorities, and blends affect the current selection.

**Example:** A dialogue controller requests a participant shot profile.

**Implementation responsibility:** The project implements camera assets, evaluation, transitions, and local-player ownership.

### Berta.Camera.Mode.FirstPerson

**Meaning:** Classifies a camera policy centered on the controlled actor's first-person view.

**Scope and suggested owner:** camera-policy classification; camera profile or local camera director selection.

**Lifecycle:** Assign to profile metadata or track the selected policy; define how requests, priorities, and blends affect the current selection.

**Example:** A camera director selects a first-person profile.

**Implementation responsibility:** The project implements camera assets, evaluation, transitions, and local-player ownership.

### Berta.Camera.Mode.LockOn

**Meaning:** Classifies a camera policy framing a focused target.

**Scope and suggested owner:** camera-policy classification; camera profile or local camera director selection.

**Lifecycle:** Assign to profile metadata or track the selected policy; define how requests, priorities, and blends affect the current selection.

**Example:** A camera director frames both player and locked target.

**Implementation responsibility:** The project implements camera assets, evaluation, transitions, and local-player ownership.

### Berta.Camera.Mode.Spectator

**Meaning:** Classifies a camera policy for observation without ordinary actor control.

**Scope and suggested owner:** camera-policy classification; camera profile or local camera director selection.

**Lifecycle:** Assign to profile metadata or track the selected policy; define how requests, priorities, and blends affect the current selection.

**Example:** A camera director selects free-observer framing after elimination.

**Implementation responsibility:** The project implements camera assets, evaluation, transitions, and local-player ownership.

### Berta.Camera.Mode.ThirdPerson

**Meaning:** Classifies a camera policy viewing the controlled actor from outside.

**Scope and suggested owner:** camera-policy classification; camera profile or local camera director selection.

**Lifecycle:** Assign to profile metadata or track the selected policy; define how requests, priorities, and blends affect the current selection.

**Example:** A camera director selects an over-the-shoulder traversal profile.

**Implementation responsibility:** The project implements camera assets, evaluation, transitions, and local-player ownership.

### Berta.Camera.Mode.Vehicle

**Meaning:** Classifies a camera policy specialized for vehicle use.

**Scope and suggested owner:** camera-policy classification; camera profile or local camera director selection.

**Lifecycle:** Assign to profile metadata or track the selected policy; define how requests, priorities, and blends affect the current selection.

**Example:** A camera director selects chase framing for the occupied vehicle.

**Implementation responsibility:** The project implements camera assets, evaluation, transitions, and local-player ownership.
