# UI — 13 tags

UI.Context labels local interface routing contexts. UI.State describes a particular widget or panel's condition. UI.Prompt classifies a prompt definition for text, icons, and action presentation. Keep ownership local and specific: an inventory panel can be Open while another panel is Closed; a global unqualified UI.State.Open on an ASC cannot explain which panel it refers to.

Contexts may be stacked, and Disabled can coexist with an open panel under project rules. The UI router supplies focus, input consumption, modal priorities, navigation, and cleanup. Pause context alone does not pause the world. Prompt labels neither bind Enhanced Input nor authorize server actions; validated gameplay remains with the relevant authority. This catalog provides no widgets or input routing implementation.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## UI.Context

### Berta.UI.Context.Dialogue

**Meaning:** Classifies a conversation-interface routing context.

**Scope and suggested owner:** local interface-routing context; local-player UI router or context descriptor.

**Lifecycle:** Select/push while the context is active and restore the previous routing context on dismissal or teardown.

**Example:** A UI router selects dialogue-choice navigation.

**Implementation responsibility:** The project implements context stacks, focus, input routing, and navigation.

### Berta.UI.Context.Gameplay

**Meaning:** Classifies the normal in-play interface routing context.

**Scope and suggested owner:** local interface-routing context; local-player UI router or context descriptor.

**Lifecycle:** Select/push while the context is active and restore the previous routing context on dismissal or teardown.

**Example:** A UI router selects HUD navigation rules.

**Implementation responsibility:** The project implements context stacks, focus, input routing, and navigation.

### Berta.UI.Context.Inventory

**Meaning:** Classifies an inventory-interface routing context.

**Scope and suggested owner:** local interface-routing context; local-player UI router or context descriptor.

**Lifecycle:** Select/push while the context is active and restore the previous routing context on dismissal or teardown.

**Example:** A UI router selects item-grid navigation.

**Implementation responsibility:** The project implements context stacks, focus, input routing, and navigation.

### Berta.UI.Context.Map

**Meaning:** Classifies a map-interface routing context.

**Scope and suggested owner:** local interface-routing context; local-player UI router or context descriptor.

**Lifecycle:** Select/push while the context is active and restore the previous routing context on dismissal or teardown.

**Example:** A UI router selects map pan and zoom input.

**Implementation responsibility:** The project implements context stacks, focus, input routing, and navigation.

### Berta.UI.Context.Menu

**Meaning:** Classifies a general menu routing context.

**Scope and suggested owner:** local interface-routing context; local-player UI router or context descriptor.

**Lifecycle:** Select/push while the context is active and restore the previous routing context on dismissal or teardown.

**Example:** A UI router selects main-menu navigation.

**Implementation responsibility:** The project implements context stacks, focus, input routing, and navigation.

### Berta.UI.Context.Pause

**Meaning:** Classifies the pause-interface routing context.

**Scope and suggested owner:** local interface-routing context; local-player UI router or context descriptor.

**Lifecycle:** Select/push while the context is active and restore the previous routing context on dismissal or teardown.

**Example:** A UI router selects pause-screen focus rules without assuming world pause.

**Implementation responsibility:** The project implements context stacks, focus, input routing, and navigation.

### Berta.UI.Context.Settings

**Meaning:** Classifies a configuration-interface routing context.

**Scope and suggested owner:** local interface-routing context; local-player UI router or context descriptor.

**Lifecycle:** Select/push while the context is active and restore the previous routing context on dismissal or teardown.

**Example:** A UI router selects settings-panel navigation.

**Implementation responsibility:** The project implements context stacks, focus, input routing, and navigation.

## UI.Prompt

### Berta.UI.Prompt.Cancel

**Meaning:** Classifies a prompt inviting dismissal or cancellation.

**Scope and suggested owner:** prompt-purpose classification; prompt definition or local presentation lookup.

**Lifecycle:** Assign as prompt metadata; choose a prompt instance when an appropriate action is offered.

**Example:** A dialog selects its back-action presentation.

**Implementation responsibility:** The project supplies localization, input glyph mapping, visibility, and validated action handling.

### Berta.UI.Prompt.Confirm

**Meaning:** Classifies a prompt inviting acceptance of a pending choice.

**Scope and suggested owner:** prompt-purpose classification; prompt definition or local presentation lookup.

**Lifecycle:** Assign as prompt metadata; choose a prompt instance when an appropriate action is offered.

**Example:** A confirmation dialog selects its accept-action presentation.

**Implementation responsibility:** The project supplies localization, input glyph mapping, visibility, and validated action handling.

### Berta.UI.Prompt.Interact

**Meaning:** Classifies a prompt inviting a contextual world interaction.

**Scope and suggested owner:** prompt-purpose classification; prompt definition or local presentation lookup.

**Lifecycle:** Assign as prompt metadata; choose a prompt instance when an appropriate action is offered.

**Example:** A prompt definition selects localized interaction text and input glyphs.

**Implementation responsibility:** The project supplies localization, input glyph mapping, visibility, and validated action handling.

## UI.State

### Berta.UI.State.Closed

**Meaning:** The identified interface element is currently closed.

**Scope and suggested owner:** specific interface-element condition; widget/panel controller for the identified element.

**Lifecycle:** Update on open, close, or interactivity changes and clean up when the element is destroyed.

**Example:** A panel controller marks a dismissed inventory panel as closed.

**Implementation responsibility:** The project owns widget lifecycle, enabled state, focus, and presentation.

### Berta.UI.State.Disabled

**Meaning:** The identified interface element currently rejects interaction.

**Scope and suggested owner:** specific interface-element condition; widget/panel controller for the identified element.

**Lifecycle:** Update on open, close, or interactivity changes and clean up when the element is destroyed.

**Example:** A widget controller disables a purchase button while funds are insufficient.

**Implementation responsibility:** The project owns widget lifecycle, enabled state, focus, and presentation.

### Berta.UI.State.Open

**Meaning:** The identified interface element is currently open.

**Scope and suggested owner:** specific interface-element condition; widget/panel controller for the identified element.

**Lifecycle:** Update on open, close, or interactivity changes and clean up when the element is destroyed.

**Example:** A panel controller marks the visible inventory panel as open.

**Implementation responsibility:** The project owns widget lifecycle, enabled state, focus, and presentation.
