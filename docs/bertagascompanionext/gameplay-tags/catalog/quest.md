# Quest — 11 tags

Quest.State describes a participant's persistent relationship to a quest. Quest.Objective describes tracking state or metadata for one objective. Store these on quest records or objective instances in a quest authority; placing all participants' quest states on an actor ASC loses identity and scope. Include quest/objective identifiers in project data rather than creating a tag for every quest.

Optional is metadata and can coexist with Active or Completed. The other states can form an exclusive set if the project's state machine requires it. Quest.State.Completed records an outcome; Event.Quest.Completed announces the transition once and can trigger separately validated reward processing. The catalog does not track progress, save records, grant rewards, or prevent duplicate completion.

Each entry below names a registered dictionary entry. Suggested owners are project responsibilities, not objects created by the catalog. See [integration](../integration.md), [lifecycle](../lifecycle.md), and [matching conventions](../conventions.md).

## Quest.Objective

### Berta.Quest.Objective.Active

**Meaning:** The objective is currently eligible for progress tracking.

**Scope and suggested owner:** objective-record condition or optionality classification; quest authority and the identified objective record.

**Lifecycle:** Update tracking state through committed objective transitions; Optional is durable metadata and can coexist with tracking states.

**Example:** A quest controller accepts matching kill or collection progress.

**Implementation responsibility:** The project defines objective criteria, optionality, persistence, and aggregate quest completion.

### Berta.Quest.Objective.Completed

**Meaning:** The objective has met its success criteria.

**Scope and suggested owner:** objective-record condition or optionality classification; quest authority and the identified objective record.

**Lifecycle:** Update tracking state through committed objective transitions; Optional is durable metadata and can coexist with tracking states.

**Example:** A quest journal marks a fulfilled objective.

**Implementation responsibility:** The project defines objective criteria, optionality, persistence, and aggregate quest completion.

### Berta.Quest.Objective.Failed

**Meaning:** The objective has reached a failure outcome.

**Scope and suggested owner:** objective-record condition or optionality classification; quest authority and the identified objective record.

**Lifecycle:** Update tracking state through committed objective transitions; Optional is durable metadata and can coexist with tracking states.

**Example:** A quest controller records failure to protect a target.

**Implementation responsibility:** The project defines objective criteria, optionality, persistence, and aggregate quest completion.

### Berta.Quest.Objective.Inactive

**Meaning:** The objective has not entered its tracking phase.

**Scope and suggested owner:** objective-record condition or optionality classification; quest authority and the identified objective record.

**Lifecycle:** Update tracking state through committed objective transitions; Optional is durable metadata and can coexist with tracking states.

**Example:** A quest controller waits for the previous objective.

**Implementation responsibility:** The project defines objective criteria, optionality, persistence, and aggregate quest completion.

### Berta.Quest.Objective.Optional

**Meaning:** Classifies an objective that is not mandatory under the quest's completion policy.

**Scope and suggested owner:** objective-record condition or optionality classification; quest authority and the identified objective record.

**Lifecycle:** Update tracking state through committed objective transitions; Optional is durable metadata and can coexist with tracking states.

**Example:** A journal marks a bonus objective that may also be active or completed.

**Implementation responsibility:** The project defines objective criteria, optionality, persistence, and aggregate quest completion.

## Quest.State

### Berta.Quest.State.Abandoned

**Meaning:** The participant explicitly left the accepted quest.

**Scope and suggested owner:** participant quest-record condition; quest authority and the participant-specific quest record.

**Lifecycle:** Update after committed quest transitions and persist with that record. Do not emit or grant it as a global completion occurrence.

**Example:** A quest authority records voluntary withdrawal.

**Implementation responsibility:** The project implements prerequisites, progress, rewards, persistence, and transition invariants.

### Berta.Quest.State.Active

**Meaning:** The participant has an ongoing accepted quest.

**Scope and suggested owner:** participant quest-record condition; quest authority and the participant-specific quest record.

**Lifecycle:** Update after committed quest transitions and persist with that record. Do not emit or grant it as a global completion occurrence.

**Example:** A quest journal tracks current objectives.

**Implementation responsibility:** The project implements prerequisites, progress, rewards, persistence, and transition invariants.

### Berta.Quest.State.Available

**Meaning:** The quest may currently be accepted by the participant.

**Scope and suggested owner:** participant quest-record condition; quest authority and the participant-specific quest record.

**Lifecycle:** Update after committed quest transitions and persist with that record. Do not emit or grant it as a global completion occurrence.

**Example:** A quest giver displays an available offer.

**Implementation responsibility:** The project implements prerequisites, progress, rewards, persistence, and transition invariants.

### Berta.Quest.State.Completed

**Meaning:** The quest has reached a successful persistent outcome for the participant.

**Scope and suggested owner:** participant quest-record condition; quest authority and the participant-specific quest record.

**Lifecycle:** Update after committed quest transitions and persist with that record. Do not emit or grant it as a global completion occurrence.

**Example:** A journal displays the recorded successful quest after its completion event.

**Related entries:** [Berta.Event.Quest.Completed](event.md#bertaeventquestcompleted).

**Important distinctions:** The quest record retains Completed; the event announces the successful transition and must not stand in for the saved record.

**Implementation responsibility:** The project implements prerequisites, progress, rewards, persistence, and transition invariants.

### Berta.Quest.State.Failed

**Meaning:** The quest has reached a failure outcome for the participant.

**Scope and suggested owner:** participant quest-record condition; quest authority and the participant-specific quest record.

**Lifecycle:** Update after committed quest transitions and persist with that record. Do not emit or grant it as a global completion occurrence.

**Example:** A journal displays a failed timed quest.

**Implementation responsibility:** The project implements prerequisites, progress, rewards, persistence, and transition invariants.

### Berta.Quest.State.Unavailable

**Meaning:** The quest cannot currently be offered to the participant.

**Scope and suggested owner:** participant quest-record condition; quest authority and the participant-specific quest record.

**Lifecycle:** Update after committed quest transitions and persist with that record. Do not emit or grant it as a global completion occurrence.

**Example:** A quest journal hides a quest with unmet prerequisites.

**Implementation responsibility:** The project implements prerequisites, progress, rewards, persistence, and transition invariants.
