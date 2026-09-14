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
| `UBertaVideoPlayerWidget` | Self-contained fullscreen Media Framework playback with optional UI audio and gameplay pause ownership. |
| `UBertaControllerUtils` | Controller feedback, light output, and Input Device Property conveniences without PlayerController casts. |
| `UBertaBTDecorator_GameplayTag` / `UBertaBTDecorator_GameplayTagQuery` | Reactive GAS conditions controlling whether Behavior Tree branches may execute. |
| `UBertaBTTask_WaitGameplayTagQuery` | Event-driven Behavior Tree wait for a Gameplay Tag Query state without Blackboard mirroring. |
| `UBertaBTTask_ActivateGameplayAbilityAndWait` | Activates one granted Gameplay Ability and waits for that exact execution to end. |
| `UBertaBTTask_WaitGameplayEvent` | Event-driven wait for the next exact or hierarchical GAS Gameplay Event. |
| `UBertaBTDecorator_AttributeThreshold` | Reactive numeric comparison against one controlled-Pawn GAS attribute. |
| `UBertaBTTask_WaitAttributeThreshold` / `UBertaBTTask_WaitTargetAttributeThreshold` | Event-driven waits until a self or Blackboard-target GAS attribute satisfies the shared numeric condition. |
| `UBertaBTTask_WaitAbilityEnd` | Waits until no active execution remains for an exact granted ability class. |
| `UBertaBTDecorator_AbilityActive` | Reactive condition for active executions of one exact granted ability class. |
| `UBertaBTTask_CancelGameplayAbility` | Immediate cancellation request for one exact granted ability spec. |
| `UBertaBTTask_SendGameplayEvent` | Sends a compact GAS Gameplay Event payload to the controlled Pawn. |
| `UBertaBTTask_SendGameplayEventToTarget` | Sends a compact GAS Gameplay Event payload to a Blackboard target Actor. |
| `UBertaBTTask_ApplyGameplayEffectToSelf` | Applies an instant, duration, or infinite Gameplay Effect to the controlled Pawn. |
| `UBertaBTTask_ApplyGameplayEffectToTarget` | Applies an outgoing Gameplay Effect from the controlled Pawn ASC to a Blackboard target ASC. |
| `UBertaBTTask_RemoveGameplayEffectsFromSelf` / `UBertaBTTask_RemoveGameplayEffectsFromTarget` | Remove active Gameplay Effects matching a non-empty query. |
| `UBertaBTDecorator_TargetGameplayTagQuery` | Reactive Gameplay Tag Query on an Actor selected from Blackboard. |
| `UBertaBTDecorator_TargetAttributeThreshold` | Reactive attribute comparison on an Actor selected from Blackboard. |
| `UBertaBTTask_WaitTargetGameplayTagQuery` | Waits across Blackboard target replacement for a target tag-query state. |
| `UBertaBTTask_ActivateGameplayAbilityWithTarget` | Triggers one granted ability with a Blackboard Actor in its Gameplay Event context. |
| `UBertaBTTask_WaitGameplayEffectApplied` | Waits for the next matching Gameplay Effect application, including instant effects. |
| `UBertaBTTask_WaitGameplayEffectRemoved` | Waits until no active Gameplay Effect matching a query remains. |
| `UBertaBTDecorator_GameplayEffectQuery` | Reactive condition for active Gameplay Effects matching a query. |
| `UBertaBTDecorator_CanActivateAbility` | Side-effect-free check of an exact granted ability's current activation rules. |
| `UBertaBTTask_WaitAbilityReady` | Bounded periodic wait for arbitrary Gameplay Ability readiness logic. |
| `UBertaGASDebugUtils` | Deterministic text snapshot of an Actor's current GAS state. |
| `UBertaGASAbilityUtils` | Read-only cooldown, cost, and activation inspection for granted Gameplay Abilities. |
| `UBertaGameplayTagDebugUtils` | Sorted tag/query summaries, exact container diffs, and Actor tag-source inspection. |
| `UBertaBlackboardDebugUtils` | Deterministic Blackboard snapshot using UE's native key-value descriptions. |
| `UBertaBehaviorTreeDebugUtils` | Public Runtime Behavior Tree execution snapshot, including active node/path descriptions. |
| `UBertaAIDebugUtils` | Combined AI, Behavior Tree, Blackboard, and GAS snapshot. |
| `UBertaBTTask_DebugGASState` | Logs the controlled Pawn's GAS snapshot at a Behavior Tree execution point. |
| `UBertaBTTask_DebugTargetGASState` | Logs a Blackboard target Actor's GAS snapshot at a Behavior Tree execution point. |
| `UBertaBTTask_DebugBlackboardState` / `UBertaBTTask_DebugAIState` | One-shot Blackboard or combined AI snapshot tasks. |
| `UBertaBTService_TraceBlackboardChanges` | Event-driven trace of selected or all Blackboard keys while a branch is relevant. |
| `UBertaBTService_TraceGameplayTags` | Event-driven count trace for explicitly selected controlled-Pawn GAS tags. |
| `UBertaBTTask_AssertGameplayTagQuery` / `UBertaBTTask_AssertAttributeThreshold` / `UBertaBTTask_AssertAbilityActive` | Immediate non-crashing GAS assertions for Behavior Tree R&D. |

Debug-facing Blueprint nodes use Unreal's `DevelopmentOnly` metadata where appropriate. This signals intended development use; it is not a blanket claim about all Runtime code or runtime cost.

`UBertaControllerUtils` plays native dynamic vibration either uniformly or per motor, returning a handle for stopping only its own actions. It also delegates controller light color/reset and asset-based `ForceFeedbackEffect` play/stop to `APlayerController`, preserving Unreal's native client-RPC behavior for the effect calls. Input Device Property activation uses the resolved controller's Platform User and lets Unreal select that user's default input device; a controller does not identify one unique physical device. Properties can be queried or removed by handle. **Remove All Input Device Properties (Global)** removes active properties for every local Platform User, so use it only when global cleanup is intended.

## Video playback widget

`UBertaVideoPlayerWidget` is a focused Runtime convenience layer over UE 5.8 Media Framework. It builds its own fullscreen `UImage`, transient `UMediaPlayer`, and transient `UMediaTexture`, so a separate Widget Blueprint or Media Texture asset is not required. Configure a `UMediaSource` and `FBertaVideoPlaybackOptions` on **Create Widget**, optionally bind the events, and then call **Add to Viewport**; construction in the viewport is the activation point.

```text
Create Widget (BertaVideoPlayerWidget)
→ set Media Source and Options
→ bind On Playback Started / Completed / Failed as needed
→ Add to Viewport
```

The options control autoplay, gameplay pause, removal after natural completion, and audio. With autoplay disabled, the source opens and remains ready until `Play` is called. `Play` also safely queues the request while an asynchronous open is in progress. `Close` is terminal and idempotent: it closes the player, detaches and releases the audio/texture resources, releases only the pause reason acquired by this widget, and removes the widget. External removal performs the same resource cleanup but leaves a nonterminal widget reusable if it is later added again.

Media open and playback completion are delegate-driven rather than polled. `OnPlaybackStarted` fires after Media Framework reports playback resumed, `OnPlaybackCompleted` is reserved for a natural end, and `OnPlaybackFailed` carries a concise error before cleanup and removal. A natural end always releases resources and owned pause; **Remove on Completion** only determines whether the now-closed widget remains in its parent. Explicit `Close` and failures do not broadcast natural completion.

When audio is enabled, the widget creates a `UMediaSoundComponent` for the same player and marks it as UI sound so it can continue while gameplay is paused. Pause acquisition uses an authoritative `AGameModeBase` pause delegate tied to this widget. If the world was already paused, the widget records no ownership and therefore never unpauses it. If another pause delegate still blocks unpause, releasing the video pause leaves the world paused. Consequently, **Pause Game** requires an owning Player Controller and authoritative Game Mode; client-only playback should disable that option and leave network pause policy to the game.

The widget accepts `UMediaSource` assets rather than raw file paths and intentionally provides no loop, playlist, subtitle, skip, fade, URL, or playback-rate API. Actual codec/container availability remains determined by the selected Media Framework backend and target platform. Runtime visual/audio behavior still requires manual Unreal verification; the repository verification compiles the feature without launching Unreal.

## AI / Behavior Tree Debugging

These Runtime helpers provide small, composable diagnostics rather than replacing Unreal's Gameplay Debugger, Visual Logger, GLS, Graph Printer, or GAS Companion. Snapshot functions run only when explicitly called; the trace services are event-driven and never Tick.

### Snapshots

- **Gameplay Tag Debug Utils** formats an exact `FGameplayTagContainer` in lexical order, returns UE's supported `FGameplayTagQuery` description, and computes sorted exact added/removed tags with `HasTagExact`. It does not synthesize parent tags. Actor lookup first uses `IGameplayTagAssetInterface`, then falls back to the Actor's ASC through `UAbilitySystemGlobals`; an empty provider container is a successful `None` result.
- **Blackboard Debug Utils** reports the Blackboard asset plus every inherited/local key sorted by name. Key type and current value use `UBlackboardComponent` public APIs and `DescribeKeyValue`, so built-in Object, Class, Bool, numeric, Enum, String, Name, Vector, and Rotator types retain UE's native formatting without raw-memory interpretation.
- **Behavior Tree Debug Utils** reports root/current tree, running and paused state, active node/tasks/trees, and the public runtime path description returned by `UBehaviorTreeComponent::GetDebugInfoString`. It does not access private execution stacks or provide debugger history; use Unreal's specialized debuggers when historical/visual execution analysis is required.
- **AI Debug Utils** composes the Behavior Tree, Blackboard, and existing GAS summaries with controller/Pawn identity. A missing subsystem is shown as `None` without discarding the useful remainder of a valid controller snapshot.

All generated summaries omit timestamps and pointer addresses so separate captures remain practical to diff. The Blueprint summary functions use `DevelopmentOnly` metadata as an authoring hint; the Runtime C++ classes are not claimed to be physically stripped from Shipping builds.

### One-shot Behavior Tree tasks

**Debug Blackboard State** logs one Blackboard snapshot to `LogBertaDebug`; **Debug AI State** logs the combined AI snapshot. Both accept an optional label, complete immediately, never Tick, and generate no persistent observer state. Existing **Debug GAS State** and **Debug Target GAS State** remain useful when a smaller GAS-only capture is preferable.

### Event-driven traces

**Trace Blackboard Changes** observes configured key names while its branch is relevant. With an empty key list and **Trace All Keys When Empty** enabled, it observes all valid Blackboard entries. Each real change emits one concise native value description; missing configured keys produce one warning during registration and are skipped.

**Trace Gameplay Tags** registers `EGameplayTagEventType::AnyCountChange` for each exact configured tag on the controlled Pawn's ASC. Each event logs the callback's actual effective count and presence state. GAS propagates count changes through the changed tag's parent hierarchy, so explicitly observing a parent can also report child-driven parent-count changes. The service intentionally has no observe-all mode.

Both services are instanced per AI. They register only during branch relevance, remove every observer/delegate on cease relevance and instance destruction, and explicitly disable Tick. They observe state only and never modify Blackboard, GAS, or Behavior Tree flow.

## GAS + Behavior Tree

These Runtime nodes bridge UE 5.8 Behavior Trees to the controlled Pawn's Ability System Component without requiring a custom ASC, Pawn, or AIController. Target nodes instead resolve an Actor-compatible Blackboard key and rebind whenever that key changes. With the single documented exception of **Wait Ability Ready**, waits and reactive decorators use GAS/Blackboard delegates rather than Tick, timers, or mirrored Blackboard state. Normal Behavior Tree aborts and relevance changes unregister observers.

- **Conditions:** Gameplay Tag, Gameplay Tag Query, Attribute Threshold, Ability Active, Can Activate Ability, Gameplay Effect Query, Target Gameplay Tag Query, and Target Attribute Threshold.
- **Waits:** Wait Gameplay Tag Query, Wait Gameplay Event, Wait Attribute Threshold, Wait Target Attribute Threshold, Wait Ability End, Wait Target Gameplay Tag Query, Wait Gameplay Effect Applied, Wait Gameplay Effect Removed, and Wait Ability Ready.
- **Actions:** Activate Gameplay Ability And Wait, Cancel Gameplay Ability, Send Gameplay Event, Send Gameplay Event To Target, Apply Gameplay Effect To Self/Target, Remove Gameplay Effects From Self/Target, and Activate Gameplay Ability With Target.
- **Inspection:** Ability cooldown, cost, and full activation readiness without activation or cost side effects.
- **Debug:** GAS Debug Summary, Debug GAS State/Target GAS State, and focused tag-query, attribute, and active-ability assertions.

AI Behavior Trees normally execute on authority, but these helpers do not add RPCs or override GAS networking. Ability activation, Gameplay Event routing, Gameplay Effect application, prediction, and replicated notifications retain their native GAS authority/network semantics. In particular, the applied-effect wait uses the server-side application delegate, while active-effect state reflects the effects visible to that ASC.

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

**Wait Target Attribute Threshold** applies that contract to an Actor-compatible Blackboard key. It observes the key even while the target is null, unbinds the previous target ASC before rebinding a replacement, and evaluates each valid target immediately. Missing targets never satisfy the condition; completion and Behavior Tree abort remove both the Blackboard and attribute observers without ticking.

### Wait Ability End

**Wait Ability End** succeeds immediately when the exact configured class is not granted or its spec is already inactive. When active, it listens for GAS ability-end events and re-queries the spec after each matching end; it completes only when no execution remains, including concurrent per-execution abilities. It never ticks and a Behavior Tree abort unregisters the observer without canceling the ability.

### Ability Active

**Ability Active** evaluates the exact granted ability spec's native active state. GAS activation and end callbacks request standard decorator condition re-evaluation, while the spec remains the source of truth so concurrent executions are handled correctly. Missing classes/specs evaluate false; no Blackboard mirror or Tick is used.

### Cancel Gameplay Ability

**Cancel Gameplay Ability** resolves the exact granted class and calls GAS's spec-handle cancellation only when it is active. An already inactive granted spec succeeds because the desired state already holds; invalid, missing-ASC, and ungranted configurations fail. The request may affect multiple active executions of that same spec, but never unrelated ability classes. Compose with **Wait Ability End** when subsequent Behavior Tree flow must wait for termination.

### Send Gameplay Event

**Send Gameplay Event** dispatches a configured tag and magnitude to the controlled Pawn's Ability System Component. The Pawn is the instigator; an optional Actor Blackboard key populates the payload target. Success means the valid event was dispatched through GAS, not that any ability consumed it or activated. Network and prediction behavior remains GAS-owned.

**Send Gameplay Event To Target** instead dispatches to the ASC exposed by a required Actor-compatible Blackboard key. Its payload uses the controlled Pawn as `Instigator`, the selected Actor as `Target`, and preserves the configured magnitude. Success only confirms that a valid target dispatch was issued; it does not imply that an ability consumed the event. The task adds no custom RPC or network policy.

### Apply Gameplay Effect To Self

**Apply Gameplay Effect To Self** builds a normal outgoing spec at the configured finite level and applies it to the controlled Pawn's Ability System Component. It uses UE 5.8's `WasSuccessfullyApplied()` result, which correctly represents both active duration/infinite effects and the special completed handle returned by successful instant effects. GAS authority and prediction rules still determine whether application is accepted.

**Apply Gameplay Effect To Target** uses the same application-result contract, but builds the spec and effect context on the controlled Pawn's source ASC and applies it to the ASC exposed by an Actor-compatible Blackboard key. Missing source/target ASCs, an invalid effect class, a non-finite level, or rejected spec/application fails synchronously. The task adds no RPC or authority override; native GAS networking rules remain in force.

### Remove Gameplay Effects

**Remove Gameplay Effects From Self** and **Remove Gameplay Effects From Target** synchronously remove all active effects matching a configured `FGameplayEffectQuery` from the controlled Pawn or Blackboard-selected Actor ASC. An empty query is rejected so an unconfigured node cannot accidentally remove every effect. Zero matches succeeds as an already-clear state; when matches exist, success requires UE to remove at least one effect. Instant effects do not participate because they are not persistent active effects. UE 5.8 performs removal only for an authoritative ASC; these tasks add no RPC or client-side override.

### Target Gameplay Tag Query

**Target Gameplay Tag Query** resolves an Actor-compatible Blackboard key and evaluates the query against that Actor's Ability System Component. It observes both the Blackboard key and every query tag on the current target. Replacing or clearing the target first unbinds the old ASC, binds the new one when available, and requests standard Observer Abort re-evaluation. Missing targets, missing ASCs, and empty queries evaluate false; no Tick is used.

### Target Attribute Threshold

**Target Attribute Threshold** applies the shared numeric comparison to an attribute on the Actor selected by Blackboard. It observes both the target key and the current target ASC's attribute delegate, rebinding without stale callbacks whenever the target changes. A null target, missing ASC, or missing attribute evaluates false; no Tick or Blackboard value mirror is used.

### Wait Target Gameplay Tag Query

**Wait Target Gameplay Tag Query** waits for `Matches` or `Does Not Match` on the current Blackboard-selected Actor's ASC. It succeeds immediately only when a valid target ASC already has the requested state. While waiting it observes both target replacement and relevant tag changes; a temporarily null target remains pending even for `Does Not Match`. Abort and completion remove both observer sets without ticking.

### Activate Gameplay Ability With Target

**Activate Gameplay Ability With Target** resolves the exact granted ability spec and triggers it through GAS with a compact Gameplay Event payload. The controlled Pawn is the instigator and the Actor from the configured Blackboard key is the target. It does not invent generic target data and does not wait for the ability to end; success means GAS accepted the trigger. Compose with **Wait Ability End** when sequencing requires completion.

### Wait Gameplay Effect Applied

**Wait Gameplay Effect Applied** observes the next matching application to the controlled Pawn's ASC. It uses UE's server-side applied delegate, which includes instant and duration effects, and matches the incoming `FGameplayEffectSpec`; it does not inspect effects that existed before the task started. Because spec matching cannot evaluate active-effect custom match delegates, queries containing native or Blueprint custom delegates fail configuration explicitly. The task is event-driven and unregisters on match or abort.

### Wait Gameplay Effect Removed

**Wait Gameplay Effect Removed** succeeds immediately when no active duration/infinite effect matches its non-empty query. Otherwise it observes all effect removals and re-runs the native query after each one, completing only when zero matches remain. This correctly handles multiple matching effects; instant effects are never active and do not participate. Abort removes the delegate and the task never ticks.

### Gameplay Effect Query

**Gameplay Effect Query** is true when at least one active duration/infinite effect on the controlled Pawn's ASC matches the configured query. Native active-effect addition and removal delegates request standard Observer Abort re-evaluation; each evaluation runs UE's query instead of maintaining a parallel count. Instant effects never become active and therefore do not make this decorator true. Empty queries evaluate false and no Tick is used.

### Can Activate Ability

**Can Activate Ability** resolves the exact granted class and calls the ability's native `CanActivateAbility` path without attempting activation or causing side effects. Invalid, ungranted, or missing-ASC configurations evaluate false.

This decorator is intentionally not advertised as reactive. A custom Gameplay Ability can base `CanActivateAbility` on arbitrary C++ or Blueprint world state for which GAS provides no universal change event. Unreal evaluates the condition whenever the Behavior Tree normally reaches or rechecks it, but configured Observer Aborts cannot be guaranteed to react immediately to every custom readiness change unless some other Behavior Tree event causes reevaluation. Use **Wait Ability Ready** when explicit periodic readiness waiting is required.

### Wait Ability Ready

**Wait Ability Ready** resolves the exact granted ability class and calls its native `CanActivateAbility` immediately. It succeeds at once when ready; otherwise it uses the Behavior Tree's native interval-tick support to recheck at the configured cadence until ready. The interval defaults to `0.1` seconds and is clamped to at least `0.01` seconds. Invalid, ungranted, removed, or missing-ASC abilities fail, and abort clears the per-AI wait state.

This is the campaign's deliberate polling exception: arbitrary custom `CanActivateAbility` logic has no generic GAS change delegate. The task does not create a world timer and does not attempt activation.

### Ability Inspection

`UBertaGASAbilityUtils` provides three Game-Thread-only, side-effect-free Blueprint calls for an exact ability class granted to an Actor's ASC. `Success` means the Actor, ASC, granted spec, actor info, and ability object were available for inspection; the result struct contains the gameplay answer.

- **Get Ability Cooldown Info** reports the ability's native cooldown state separately from general activation readiness, plus finite remaining/duration seconds and a clamped `0..1` remaining fraction. Abilities without cooldown return a ready zeroed result; an active cooldown without finite timing can still report `bIsOnCooldown=true` with zero timing fields.
- **Check Ability Cost** invokes `CheckCost` only. It never calls `ApplyCost` or mutates attributes.
- **Check Ability Activation** invokes `CanActivateAbility` with the granted spec handle and current actor info. It never attempts activation.

Cost and activation `FailureTags` are best-effort diagnostics produced by GAS or the ability. Custom logic may return false with an empty container; BertaDevKit does not fabricate or reinterpret failure tags. These calls retain the ASC's native authority, prediction, cooldown, cost, and custom ability semantics.

### GAS Debug Summary

**Get GAS Debug Summary** is a `DevelopmentOnly` Blueprint-callable snapshot for logs, bug reports, and R&D inspection. Given an Actor exposed through the normal GAS interface, it reports the Actor and ASC identity followed by sorted owned tags, granted abilities with active state and level, active effects with stack/timing information, and available attributes with current values. Empty sections are explicit, output contains no pointer addresses, and invalid Actors or Actors without an ASC return false.

The utility is a compact copy/paste diagnostic, not a logging UI or a replacement for GAS Companion's specialized tooling.

### Debug GAS State

**Debug GAS State** is an immediate Behavior Tree task that resolves the controlled Pawn, reuses **Get GAS Debug Summary**, writes one snapshot to `LogBertaDebug` with an optional label, and succeeds. It fails when there is no valid controlled Pawn/ASC, never ticks, and is intended for development and R&D checkpoints rather than continuous telemetry.

**Debug Target GAS State** applies the same one-shot diagnostic to an Actor-compatible Blackboard target. It reuses the exact same summary formatter, adds an optional label, and fails cleanly for a missing target or ASC. It has no Tick, observer, or persistent logging state.

### GAS Assertion Tasks

**Assert Gameplay Tag Query**, **Assert Attribute Threshold**, and **Assert Ability Active** are immediate R&D tasks for validating controlled-Pawn GAS state at a precise Behavior Tree execution point. A satisfied expectation succeeds silently. Invalid configuration, missing GAS state, or a mismatch emits one bounded `LogBertaDebug` diagnostic with the optional label and returns `Failed`; these nodes never use crash-style C++ assertions.

The tag assertion uses native query matching, the attribute assertion reuses `FBertaGameplayAttributeCondition`, and the ability assertion uses the same exact granted-spec `IsActive()` state as the active-ability decorator. They do not Tick, wait, register delegates, or automatically dump the full GAS summary.

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
