# World — 19 tags

World labels describe surface, zone, and weather metadata. Surface tags belong to project contact/material mappings or surface definitions, Zone tags to region definitions, and Weather tags to profiles or environment selections. They are not automatically assigned to actors occupying those regions. Derive State.Environment tags separately from the authoritative environment query when actor conditions are needed.

World.Surface.Metal is not a UPhysicalMaterial, a physical-surface enum, or a friction coefficient. Map native collision material information explicitly to this semantic label. Weather.Snow describes precipitation while Surface.Snow describes contact material; either can exist without the other. Zone classifications can overlap in project geometry and require an explicit priority or composition policy. No weather, hazard, audio, or material behavior is installed.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## World.Surface

### Berta.World.Surface.Concrete

**Meaning:** Classifies a surface as manufactured concrete.

**Scope and suggested owner:** world-definition classification; surface definition or project Physical Material-to-tag mapping.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A footstep resolver selects a concrete impact response.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Surface.Dirt

**Meaning:** Classifies a surface as exposed soil.

**Scope and suggested owner:** world-definition classification; surface definition or project Physical Material-to-tag mapping.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A traversal resolver selects dirt contact presentation.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Surface.Glass

**Meaning:** Classifies the contact surface as glass material.

**Scope and suggested owner:** world-definition classification; surface definition or project Physical Material-to-tag mapping.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** An impact resolver selects glass hit presentation.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Surface.Grass

**Meaning:** Classifies a surface as grass-covered ground.

**Scope and suggested owner:** world-definition classification; surface definition or project Physical Material-to-tag mapping.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A footstep resolver selects grass rustle presentation.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Surface.Ice

**Meaning:** Classifies the contact surface as frozen-water ice.

**Scope and suggested owner:** world-definition classification; surface definition or project Physical Material-to-tag mapping.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A movement policy selects an explicit low-friction rule after classification.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Surface.Metal

**Meaning:** Classifies the contact surface as metallic material.

**Scope and suggested owner:** world-definition classification; surface definition or project Physical Material-to-tag mapping.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A hit resolver maps a Physical Material to metallic impact presentation.

**Important distinctions:** This is not a native Physical Material or Surface Type. Maintain an explicit project mapping from collision material data to the semantic label.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Surface.Sand

**Meaning:** Classifies a surface as granular sandy ground.

**Scope and suggested owner:** world-definition classification; surface definition or project Physical Material-to-tag mapping.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A footstep resolver selects sand contact presentation.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Surface.Snow

**Meaning:** Classifies a surface as snow-covered ground.

**Scope and suggested owner:** world-definition classification; surface definition or project Physical Material-to-tag mapping.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A footstep resolver selects snow compression presentation.

**Related entries:** [Berta.World.Weather.Snow](world.md#bertaworldweathersnow).

**Important distinctions:** Snowfall and snow-covered contact surfaces are independent classifications.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Surface.Stone

**Meaning:** Classifies a surface as natural rock or stone.

**Scope and suggested owner:** world-definition classification; surface definition or project Physical Material-to-tag mapping.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A footstep resolver selects a stone sound after mapping the hit material.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Surface.Wood

**Meaning:** Classifies the contact surface as wooden material.

**Scope and suggested owner:** world-definition classification; surface definition or project Physical Material-to-tag mapping.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** An impact resolver selects wood hit presentation.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

## World.Weather

### Berta.World.Weather.Clear

**Meaning:** Classifies a weather profile with no significant precipitation or storm.

**Scope and suggested owner:** world-definition classification; weather profile or environment director selection.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A weather director selects a clear-sky profile.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Weather.Rain

**Meaning:** Classifies a weather profile with liquid precipitation.

**Scope and suggested owner:** world-definition classification; weather profile or environment director selection.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A weather director selects rainfall presentation.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Weather.Snow

**Meaning:** Classifies a weather profile with frozen precipitation.

**Scope and suggested owner:** world-definition classification; weather profile or environment director selection.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A weather director selects snowfall independently of ground-surface coverage.

**Related entries:** [Berta.World.Surface.Snow](world.md#bertaworldsurfacesnow).

**Important distinctions:** Snowfall and snow-covered contact surfaces are independent classifications.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Weather.Storm

**Meaning:** Classifies a weather profile with intense atmospheric disturbance.

**Scope and suggested owner:** world-definition classification; weather profile or environment director selection.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A weather director selects a thunderstorm profile.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

## World.Zone

### Berta.World.Zone.Combat

**Meaning:** Classifies a world region intended for combat activity.

**Scope and suggested owner:** world-definition classification; zone or region definition.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A zone definition labels an arena.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Zone.Hazard

**Meaning:** Classifies a world region containing a danger.

**Scope and suggested owner:** world-definition classification; zone or region definition.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A zone definition labels a toxic area without applying damage itself.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Zone.Restricted

**Meaning:** Classifies a world region with controlled access.

**Scope and suggested owner:** world-definition classification; zone or region definition.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** An access system evaluates a security area's permissions.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Zone.Safe

**Meaning:** Classifies a world region intended as a safe area.

**Scope and suggested owner:** world-definition classification; zone or region definition.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** A zone definition labels a sanctuary for project combat rules.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.

### Berta.World.Zone.Underwater

**Meaning:** Classifies a world region lying beneath the relevant water surface.

**Scope and suggested owner:** world-definition classification; zone or region definition.

**Lifecycle:** Assign as definition metadata. Select or query the relevant definition; derive actor conditions separately.

**Example:** An environment definition labels a submerged traversal area.

**Related entries:** [Berta.State.Environment.Underwater](state.md#bertastateenvironmentunderwater), [Berta.State.Movement.Swimming](state.md#bertastatemovementswimming).

**Important distinctions:** A region classification, actor submersion, and locomotion regime are separate. Swimming near the surface need not imply full submersion.

**Implementation responsibility:** The project maps native world data and implements contact responses, zones, or weather behavior.
