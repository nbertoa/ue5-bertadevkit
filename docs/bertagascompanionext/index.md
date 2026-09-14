# BertaGASCompanionExt

BertaGASCompanionExt is an optional Unreal Engine 5.8 Runtime/Editor extension for projects already using GAS Companion. It is disabled by default and does not replace, fork, or modify GAS Companion.

## Setup

Install a licensed copy of GAS Companion at `BertaDevKitHost/Plugins/GASCompanion/`, then enable **BertaDevKit**, **GAS Companion**, **Targeting System**, and **BertaGASCompanionExt**. The local GAS Companion directory is ignored and must never be committed.

## Runtime diagnostics

- **Build GSC Ability Activation Report** returns an explicit success result plus a structured, deterministic report for one Actor and exact Ability class. It combines native BertaDevKit cooldown/cost/activation checks with grant, active, Enhanced Input binding, and public Ability Queue state.
- `UBertaGSCTraceComponent` records public `UGSCCoreComponent` events without Tick. `StartTracing`, `StopTracing`, `ClearTrace`, `GetTraceEvents`, and `DumpTraceToText` manage a bounded buffer with optional logging.
- `UBertaGSCAbilityQueueInputBridgeComponent` observes public failure/end delegates and reconciles them on the next frame. It forwards only when public queue state shows GAS Companion did not already handle the event, and deliberately skips same-frame failure/end sequences whose public state is ambiguous rather than risk duplicate activation. Treat the bridge as a conservative compatibility layer and run the version-specific manual checks below.

## Editor validation and tag search

`UBertaGSCAbilitySetValidator` integrates with Data Validation and reports exact Ability Set entries for unresolved or non-instantiable classes, invalid levels, broken Input Actions, and exact duplicate grants. Shared Input Actions across different abilities are warnings because GAS Companion supports ability stacks for one action.

The generic `UBertaGameplayTagUsageFinder` lives in BertaDevKitEditor and has no GAS Companion dependency. Call it from an Editor Utility Blueprint or C++ with selected-assets, explicit-root, or whole-game scope. It reports asset, object/class, reflected property path, stored tag, and whether the usage came from a tag, container, or query.

## Targeting

- `UBertaGSCTargetType_DebugProxy` delegates to another configured GSC TargetType and preserves its results while optionally logging and drawing new hits/actors.
- `UBertaGSCTargetType_TargetingPreset` executes a UE 5.8 `UTargetingPreset` through the supported immediate request path, converts native hit/actor results, avoids duplicates, and releases the request handle synchronously.

## Manual verification

Use `/Game/Maps/DebugMap` in a project with GAS Companion enabled. These checks require Unreal Editor and are intentionally not run by automated repository work.

1. **Activation report:** grant a known Ability, bind an Input Action, configure cooldown/cost and an Ability Queue window, then call **Build GSC Ability Activation Report** before, during, and after activation. Confirm grant level, active state, failure tags, timing, binding, queue state, and summary.
2. **Live Trace:** add `UBertaGSCTraceComponent`, call `StartTracing`, exercise ability activation/end/failure/commit, cooldowns, effects, stacks/timing, tags, and attributes, then inspect `GetTraceEvents` and `DumpTraceToText`. Repeated Start/Stop must not duplicate events; reducing `MaximumEventCount` must retain only the newest entries.
3. **Queue bridge:** add `UGSCAbilityInputBindingComponent`, `UGSCCoreComponent`, `UGSCAbilityQueueComponent`, and `UBertaGSCAbilityQueueInputBridgeComponent`. While one Ability is active, open a queue window with `UGSCAbilityQueueNotifyState` or explicit calls and press a bound second Ability that fails. Confirm it queues exactly once and activates exactly once after the first Ability ends. Repeat with the second Ability excluded, with the queue closed, and in client/server PIE to confirm native authority/prediction behavior remains unchanged. Enable bridge diagnostics to prove the public failure signal is received.
4. **Ability Set validation:** create valid and deliberately invalid `UGSCAbilitySet` assets, run **Validate Assets**, and confirm each error/warning names the asset and exact array index without modifying it.
5. **Gameplay Tag usages:** from an Editor Utility Blueprint call `FindGameplayTagUsages` for a known tag in each scope. Confirm direct tags, containers, queries, nested structs/collections, and Blueprint defaults are reported. Compare Exact and ParentOrChild modes.
6. **Target debug proxy:** create a Blueprint subclass, configure a non-proxy `InnerTargetType`, enable log/draw, and use it in a GSC Effect Container. Confirm target results match the inner TargetType exactly and diagnostics show only produced results. Verify null and recursive configuration fail safely.
7. **Targeting preset adapter:** configure a synchronous `UTargetingPreset`, use the adapter in a GSC Effect Container, and confirm selected native targets receive the effects once. Test empty results and missing preset; neither should mutate gameplay state or leak a request.
