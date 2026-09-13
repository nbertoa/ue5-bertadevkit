# BertaDevKit

BertaDevKit is a general-purpose Unreal Engine 5.8 toolbox. Its Runtime module provides reusable Blueprint-facing utilities, while its Editor module provides conservative tooling for development workflows.

## Runtime utilities

| System | Purpose |
| --- | --- |
| `UBertaDebugUtils` | Blueprint-friendly screen and Output Log messages with context, verbosity, categories, and per-call gating. |
| `UBertaDebugDraw` | Development debug drawing for common primitives, components, strings, coordinate systems, and persistent-shape flushing. |
| `UBertaScreenStats` | Named development screen stats for common value types; updating a name replaces its displayed value. |
| `UBertaMathUtils` | Remapping, easing, angular helpers, snapping, distributions, and lightweight prediction helpers. |
| `UBertaWorldUtils` | Actor queries, traces, player/camera access, and delayed-action timer helpers. |
| `UBertaUIUtils` | Blueprint conveniences for common UI/player-input boilerplate. |
| `UBertaControllerUtils` | Controller feedback, light output, and Input Device Property conveniences without PlayerController casts. |
| `UBertaBTDecorator_GameplayTag` / `UBertaBTDecorator_GameplayTagQuery` | Reactive GAS conditions controlling whether Behavior Tree branches may execute. |
| `UBertaBTTask_WaitGameplayTagQuery` | Event-driven Behavior Tree wait for a Gameplay Tag Query state without Blackboard mirroring. |
| `UBertaBTTask_ActivateGameplayAbilityAndWait` | Activates one granted Gameplay Ability and waits for that exact execution to end. |
| `UBertaBTTask_WaitGameplayEvent` | Event-driven wait for the next exact or hierarchical GAS Gameplay Event. |
| `UBertaBTDecorator_AttributeThreshold` | Reactive numeric comparison against one controlled-Pawn GAS attribute. |
| `UBertaBTTask_WaitAttributeThreshold` | Event-driven wait until one GAS attribute satisfies the shared numeric condition. |
| `UBertaBTTask_WaitAbilityEnd` | Waits until no active execution remains for an exact granted ability class. |
| `UBertaBTDecorator_AbilityActive` | Reactive condition for active executions of one exact granted ability class. |
| `UBertaBTTask_CancelGameplayAbility` | Immediate cancellation request for one exact granted ability spec. |

Debug-facing Blueprint nodes use Unreal's `DevelopmentOnly` metadata where appropriate. This signals intended development use; it is not a blanket claim about all Runtime code or runtime cost.

`UBertaControllerUtils` plays native dynamic vibration either uniformly or per motor, returning a handle for stopping only its own actions. It also delegates controller light color/reset and asset-based `ForceFeedbackEffect` play/stop to `APlayerController`, preserving Unreal's native client-RPC behavior for the effect calls. Input Device Property activation uses the resolved controller's Platform User and lets Unreal select that user's default input device; a controller does not identify one unique physical device. Properties can be queried or removed by handle. **Remove All Input Device Properties (Global)** removes active properties for every local Platform User, so use it only when global cleanup is intended.

### Reactive Gameplay Tag decorators

Both decorators read the controlled Pawn's `UAbilitySystemComponent` directly and use Unreal's normal Behavior Tree **Observer Aborts** setting. They register Gameplay Tag change events only while relevant, so configured `Self`, `Lower Priority`, or `Both` aborts react without a tick, timer, or Blackboard boolean.

- **Gameplay Tag** is the simple single-tag condition.
- **Gameplay Tag Query** evaluates an `FGameplayTagQuery` and automatically observes every unique tag referenced by the query. The query must be non-empty; an empty query or a controlled Pawn without an Ability System Component evaluates false.

For example, a query combining `ALL(State.Combat, Weapon.Ranged)` with `NONE(Status.Stunned)` expresses `(State.Combat && Weapon.Ranged) && !Status.Stunned`. Adding or removing any referenced tag requests immediate Behavior Tree condition re-evaluation according to the configured Observer Aborts policy. Gameplay Tag hierarchy is preserved by GAS, so a query for `State.Combat` also reacts when a child such as `State.Combat.Melee` changes the parent's effective count.

The source is specifically the controlled Pawn's Ability System Component; these decorators do not read arbitrary `IGameplayTagAssetInterface` actors.

### Gameplay Tag Query Wait Task

**Wait Gameplay Tag Query** pauses a Behavior Tree Sequence until its query either **Matches** or **Does Not Match** the controlled Pawn's Ability System Component. Unlike a decorator, which controls whether a branch may execute and can drive Observer Aborts, this latent task represents an explicit sequencing step.

The task evaluates immediately when execution begins. It succeeds without waiting when the requested state already holds; otherwise it observes every tag referenced by the query and completes when a relevant change satisfies the condition. It has no Tick, polling timer, or Blackboard boolean, and a normal Behavior Tree abort unregisters its observers.

For example:

```text
Activate or start attack
→ Wait Gameplay Tag Query
    Query: State.Attacking
    Wait Until: Does Not Match
→ Choose next action
```

An empty query or a controlled Pawn without an Ability System Component fails immediately.

### Activate Gameplay Ability And Wait

**Activate Gameplay Ability And Wait** resolves the configured exact granted ability class on the controlled Pawn's Ability System Component, requests activation, and remains latent until that execution ends. A normal end succeeds; activation rejection and cancellation fail. GAS activation, failure, and per-activation end delegates drive the task, so it does not tick.

The task listens before requesting activation, including abilities that end synchronously from `ActivateAbility`. `Cancel Ability On Abort` cancels the task-owned execution when GAS exposes an instantiated ability identity; it intentionally avoids broad spec cancellation for non-instanced abilities because that could terminate unrelated executions. `Allow Remote Activation` is passed to GAS unchanged, so authority, prediction, and remote execution remain governed by the ability's normal network policy. If GAS only sends a remote request and does not establish an observable local execution, the task fails instead of entering an unfinishable latent state; AI Behavior Trees normally run on authority, where server-executed abilities provide that identity.

### Wait Gameplay Event

**Wait Gameplay Event** waits for the next matching event received by the controlled Pawn's Ability System Component. Exact mode listens only for the configured tag; hierarchical mode uses GAS's native tag-container event routing and also accepts descendant event tags. The task is event-driven, does not queue earlier events or expose payload data, and unregisters immediately after one match or a Behavior Tree abort.

### Attribute Threshold

**Attribute Threshold** compares one attribute on the controlled Pawn's Ability System Component with a configured threshold using `<`, `<=`, `==`, `!=`, `>=`, or `>`. Equality and inequality use the configured non-negative tolerance. Missing attributes evaluate false. The decorator observes GAS's native attribute-value delegate while relevant and drives standard Observer Aborts without ticking or mirroring a Blackboard value.

### Wait Attribute Threshold

**Wait Attribute Threshold** uses the same comparison and tolerance semantics as the decorator. It succeeds immediately when the current value already satisfies the condition; otherwise it listens to GAS's attribute-value delegate and completes on the first satisfying change. Invalid or missing attributes fail, and Behavior Tree abort/stop removes the observer without ticking.

### Wait Ability End

**Wait Ability End** succeeds immediately when the exact configured class is not granted or its spec is already inactive. When active, it listens for GAS ability-end events and re-queries the spec after each matching end; it completes only when no execution remains, including concurrent per-execution abilities. It never ticks and a Behavior Tree abort unregisters the observer without canceling the ability.

### Ability Active

**Ability Active** evaluates the exact granted ability spec's native active state. GAS activation and end callbacks request standard decorator condition re-evaluation, while the spec remains the source of truth so concurrent executions are handled correctly. Missing classes/specs evaluate false; no Blackboard mirror or Tick is used.

### Cancel Gameplay Ability

**Cancel Gameplay Ability** resolves the exact granted class and calls GAS's spec-handle cancellation only when it is active. An already inactive granted spec succeeds because the desired state already holds; invalid, missing-ASC, and ungranted configurations fail. The request may affect multiple active executions of that same spec, but never unrelated ability classes. Compose with **Wait Ability End** when subsequent Behavior Tree flow must wait for termination.

## Editor tools

- **Asset Naming** audits naming conventions and can apply a reviewed rename batch. It also participates in UE Data Validation.
- **Asset Cleaner** identifies conservative unused/orphan asset candidates. Its audit is read-only; cleanup revalidates candidates and opens Unreal's native deletion workflow.
- **Asset Insights** is a read-only Content Browser analysis with saved-package, dependency/referencer, Texture2D, StaticMesh, and conservative footprint information.
- **Project Setup** is an opt-in audit/apply utility for a curated allowlist of preferred project and per-project Editor defaults. It previews changes and does not mutate projects at startup.
- **Blueprint Audit** is a read-only, conservative static linter and review assistant. Findings require manual review; it has no automatic graph rewriting or refactoring action.
- **Blueprint Usage Finder** is a read-only Content Browser action for one project asset. It reports exact Blueprint graph nodes that reference the asset and offers node navigation; it is not a universal reference search.
- **World Validation** checks the open Editor level against enabled policy checks and reports violations without changing actors.

The main Editor actions are under **Tools → BertaDevKit**. Content Browser context menus provide Asset Naming, Asset Cleaner, Blueprint Audit, and Asset Insights actions for selected project assets and folders.

## Architecture and configuration

BertaDevKit maintains a strict module boundary:

- `BertaDevKit` is the Runtime module available to game code and Blueprints.
- `BertaDevKitEditor` is the Editor-only module for menus, audits, automation, asset tooling, and validation.

The Runtime module never depends on the Editor module. Runtime settings are under **Project Settings → Plugins → BertaDevKit** and persist in `Config/DefaultBertaDevKit.ini`.

The personal external-plugin catalog remains separate in [`PLUGIN_TOOLBOX.md`](https://github.com/nbertoa/ue5-bertadevkit/blob/main/BertaDevKitHost/Plugins/BertaDevKit/Docs/PLUGIN_TOOLBOX.md); it is not part of this site's primary navigation and is not a BertaDevKit dependency.

## Installation

Copy `BertaDevKitHost/Plugins/BertaDevKit/` to `<YourProject>/Plugins/BertaDevKit/`, target UE 5.8, regenerate project files if needed, build, and enable **BertaDevKit**. The repository root is a development host, not the distributable plugin directory.

## Log categories

| Category | Scope |
| --- | --- |
| `LogBertaDevKit` | Runtime utilities and systems |
| `LogBertaDebug` | Debug logging and drawing |
| `LogBertaDevKitEditor` | Editor tooling and validation |
