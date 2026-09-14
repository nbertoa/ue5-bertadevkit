# BertaComboGraphExt

Optional UE 5.8 Runtime/Editor tooling for projects that already own and use Combo Graph `1.6.3+5.8`. The plugin is disabled by default, contains no Combo Graph source or content, and does not change Combo Graph assets at runtime.

## Contract Validator

`UBertaComboGraphValidator` participates in Unreal Data Validation for `UComboGraph` assets. `UBertaComboGraphValidationLibrary` also returns a deterministic structured report. Checks cover entry/first-node presence, reachability, forbidden cycles, parent/child/edge consistency, self/duplicate links, conduit placement and outputs, missing or ambiguous transition inputs, notify contracts and timing, effective Combo Window configuration, montage/sequence setup, additive cost semantics, Effect/Damage/Cue Container data, and Cue Container EffectContext compatibility.

The validator reports only contracts observable from the asset and current project settings. It does not infer multiplayer use, and does not claim that a SetByCaller tag is unused merely because static modifiers do not expose a direct consumer.

It intentionally does not emit a warning on every asset for concurrency. Stock Combo Graph nodes and edges are shared asset objects with mutable runtime fields, but static validation cannot prove that a graph will execute concurrently; projects that do so must verify the stock plugin's concurrency behavior manually.

## Runtime trace

Add `UBertaComboGraphTraceComponent` to the graph avatar and call `StartTracing`. Global graph start/end delegates establish independent weak task records. A short-lived timer samples public task getters only while a graph is active; there is no permanent Tick.

Graph start/end and task `EventReceived` are `Direct`. Node, queue, montage, and Combo Window changes are `Observed`; terminal node exit is `Inferred`. `TransitionObserved` may identify a connection asset as `ResolvedFromTopology`, but never claims that the edge's raw input caused the transition. Because Combo Graph's public event delegate omits its sender, gameplay events are recorded only when exactly one execution is active on the component.

## EffectContext compatibility

`UBertaComboGraphEffectContextCompatibilityLibrary` reports both whether configured `AbilitySystemGlobals` derives from `UComboGraphAbilitySystemGlobals` and whether the instance actually allocates a `FComboGraphGameplayEffectContext`-compatible struct. Both checks must pass for Cue Containers to be reported safe. No class or config is installed automatically. This release deliberately avoids an empty derived EffectContext: without Berta-owned payload it adds no current value while creating a new `Duplicate`/`NetSerialize`/Iris contract.

## Targeting System adapter

Use `UBertaComboGraphNodeMontage` or `UBertaComboGraphNodeSequence` and map an Effect Container event tag to a `UTargetingPreset`. Start the ability task with `Start Combo Graph with Berta Targeting`.

For a configured event, the adapter executes an immediate UE 5.8 Targeting System request, preserves result order, removes duplicate actors while retaining the first `FHitResult`, replaces only target data, and releases the handle. Its `UBertaComboGraphTargetingContext` source object exposes the current node, container event tag, and original `FGameplayEventData` to targeting tasks. Effect specs, application, authority, and prediction remain in the base task. No preset means exact `Super` fallback; an empty preset result intentionally means no targets. Damage and Cue Containers retain stock behavior.

## Pre-input buffer

`UBertaComboGraphInputBufferComponent` is local, opt-in, finite, and task/node scoped. It observes only outgoing `Triggered` actions while the Combo Window is closed, expires them after `BufferDurationSeconds` (default `0.2`), and delivers one candidate through Combo Graph's existing replicated gameplay-event component. On window open it waits one sampling interval so Combo Graph's direct binding wins if the physical input is still active; stale entries clear on node change, graph end, stop, invalid task, and timeout. Ambiguous concurrent executions are rejected explicitly.

Only `Triggered` edges are supported. The public seam cannot faithfully reconstruct `Started`, `Canceled`, input magnitude, elapsed time, or the original `FInputActionInstance`; no custom RPC is added.

## Modules

- `BertaComboGraphExt` (Runtime): Combo Graph, GAS/Gameplay Tasks, Enhanced Input, and UE Targeting System.
- `BertaComboGraphExtEditor` (Editor): Runtime module, Combo Graph Runtime API, Data Validation, UnrealEd, GameplayAbilities, GameplayTags, and Niagara for typed Cue source validation.

Neither module depends on `ComboGraphEditor`; base BertaDevKit modules remain independent. Automation Tests are added as source coverage but were not executed because launching Unreal was not authorized.
