# BertaGASCompanionExt

BertaGASCompanionExt is an optional Unreal Engine 5.8 Runtime/Editor extension for projects already using GAS Companion. It is disabled by default and does not replace, fork, or modify GAS Companion.

## Setup

Install a licensed copy of GAS Companion at `BertaDevKitHost/Plugins/GASCompanion/`, then enable **BertaDevKit**, **GAS Companion**, **Targeting System**, and **BertaGASCompanionExt**. The local GAS Companion directory is ignored and must never be committed.

## Runtime diagnostics

- **Build GSC Ability Activation Report** returns an explicit success result plus a structured, deterministic report for one Actor and exact Ability class. It combines native BertaDevKit cooldown/cost/activation checks with grant, active, Enhanced Input binding, and public Ability Queue state.
- `UBertaGSCTraceComponent` records public `UGSCCoreComponent` events without Tick. `StartTracing`, `StopTracing`, `ClearTrace`, `GetTraceEvents`, and `DumpTraceToText` manage a bounded buffer with optional logging. Attribute `OldValue` is marked known only when a prior value was observed in the current trace; the first observation is formatted as `Old=unknown`. Explicit old stack counts remain known.
- `UBertaGSCAbilityQueueInputBridgeComponent` never observes global ability failures. Call `ReportInputDrivenFailure` only from the handler of the supplied Enhanced Input action, immediately after that handler's activation attempt fails. The bridge verifies the action against `UGSCAbilityInputBindingComponent`'s current public runtime binding, queue state, and allow-list before forwarding. It listens to `UGSCCoreComponent::OnInitAbilityActorInfo` so auto-start waits for and rebinds to PlayerState or Pawn ASCs. Ability-end reconciliation is deferred one frame and is allowed only for a failure the bridge itself submitted, so native GSC handling wins without making unrelated queue state the bridge's responsibility.

GAS Companion does not expose a public delegate from its private Enhanced Input activation handler carrying a failed attempt's provenance. Consequently the bridge cannot safely integrate automatically with that private handler: projects needing this compatibility path must use an explicit input handler that calls `ReportInputDrivenFailure`. AI, Gameplay Event, and programmatic activation failures are intentionally ignored unless a caller violates that explicit API precondition.

## Editor validation and tag search

`UBertaGSCAbilitySetValidator` integrates with Data Validation and reports exact Ability Set entries for unresolved or non-instantiable classes, invalid levels, broken Input Actions, unresolved configured `InitializationData`, and exact duplicate grants. Shared Input Actions across different abilities are warnings because GAS Companion supports ability stacks for one action. GAS Companion delegates initialization to UE's `UAttributeSet::InitFromMetaDataTable` and declares no additional public row-structure contract, so the validator does not invent one.

The generic `UBertaGameplayTagUsageFinder` lives in BertaDevKitEditor and has no GAS Companion dependency. Call it from an Editor Utility Blueprint or C++ with selected-assets, explicit-root, or Whole Project scope. The existing C++ enum identifiers remain `ExplicitGameRoot` and `WholeGame` for Blueprint compatibility, but their displayed names and behavior cover mounted project content roots: `/Game`, project content plugins, and mounted project Game Feature Plugins, without scanning arbitrary Engine content. Large scans show cancelable progress. The return value is false when canceled or when candidate assets fail to load, while partial results remain available and `OutSummary` reports `Status=PartialCanceled` or `Status=PartialLoadFailures` plus candidate, processed, loaded, failed, skipped, and result counts. Load failures are aggregated and observable without one log per result.

## Targeting

- `UBertaGSCTargetType_DebugProxy` delegates to another configured GSC TargetType and preserves its results while optionally logging and drawing new hits/actors.
- `UBertaGSCTargetType_TargetingPreset` executes a UE 5.8 `UTargetingPreset` through the supported immediate request path, converts native hit/actor results, avoids duplicates, and releases the request handle synchronously.

## Manual verification

Use `/Game/Maps/DebugMap` in a project with GAS Companion enabled. These checks require Unreal Editor and are intentionally not run by automated repository work.

1. **Activation report:** grant a known Ability, bind an Input Action, configure cooldown/cost and an Ability Queue window, then call **Build GSC Ability Activation Report** before, during, and after activation. Confirm grant level, active state, failure tags, timing, binding, queue state, and summary.
2. **Live Trace:** add `UBertaGSCTraceComponent`, call `StartTracing`, exercise ability activation/end/failure/commit, cooldowns, effects, stacks/timing, tags, and attributes, then inspect `GetTraceEvents` and `DumpTraceToText`. Confirm the first attribute observation says `Old=unknown`; subsequent observations use only the last actually observed value, including across clamps and non-additive modifiers. Repeated Start/Stop and `ClearTrace` must reset that history without duplicate events; reducing `MaximumEventCount` must retain only the newest entries.
3. **Queue bridge:** test ASC in Character and PlayerState on server and owning client. From an explicit Enhanced Input handler, report a failed bound ability during an open queue and confirm it queues and activates exactly once. Confirm closed queue, disallowed ability, mismatched action, and AI/code/Gameplay Event failures never forward. Repeat after respawn, repossession, and PlayerState replication; auto-start must wait for and rebind to the current ASC. Simulated proxies must remain passive unless explicitly driven.
4. **Ability Set validation:** validate assets with valid and broken configured `InitializationData`; the broken path must report the asset and exact `GrantedAttributes` index. Also confirm normal valid/invalid Ability, Attribute, Effect, Input Action, level, and duplicate cases remain read-only.
5. **Gameplay Tag usages:** search `/Game`, a project content plugin, and a mounted Game Feature Plugin. Cancel a Whole Project scan and confirm partial results/counts, then force or observe a load failure and confirm the aggregated warning. Confirm direct tags, containers, queries, nested structs, arrays, sets, maps, nested package-owned UObjects, and Blueprint CDO defaults; compare Exact and ParentOrChild modes and check deterministic ordering.
6. **Target debug proxy:** create a Blueprint subclass, configure a non-proxy `InnerTargetType`, enable log/draw, and use it in a GSC Effect Container. Confirm target results match the inner TargetType exactly and diagnostics show only produced results. Verify null and recursive configuration fail safely.
7. **Targeting preset adapter:** configure a synchronous `UTargetingPreset`, use the adapter in a GSC Effect Container, and confirm selected native targets receive the effects once. Test empty results and missing preset; neither should mutate gameplay state or leak a request.
