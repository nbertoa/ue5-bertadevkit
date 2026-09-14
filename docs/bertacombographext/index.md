# BertaComboGraphExt

BertaComboGraphExt is an optional Unreal Engine 5.8 extension for a legitimate local Combo Graph `1.6.3+5.8` installation. It adds original BertaDevKit validation and adapters without replacing the graph, transition, animation, collision, cue, or ability runner.

## Daily tools

- **Contract Validator:** deterministic Data Validation for topology, conduits, first-match transition ambiguity, notify/Combo Window timing, animation, costs, containers, and EffectContext compatibility.
- **Runtime Trace + Snapshot:** bounded observations from independent weak task records, with direct/observed/inferred claims distinguished.
- **EffectContext Compatibility:** reports configured-globals inheritance separately and uses the actually allocated context struct as the authoritative Cue Container runtime contract; it never rewrites config.
- **Targeting adapter:** optional Berta node presets plus a targeting-aware StartGraph task for synchronous multi-target Effect Containers and stock fallback.
- **Pre-input buffer:** local, finite, node-scoped buffering for `Triggered` edges using Combo Graph's existing replicated gameplay-event path.

The trace can resolve a connection asset from previous/current topology but cannot prove the raw input or reason that selected it. Concurrent task events without a sender are not attributed. The buffer does not support `Started` or `Canceled`, cannot preserve `FInputActionInstance` magnitude/timing, and waits one sampling interval at window open to avoid racing Combo Graph's direct binding. Targeting presets affect only Gameplay Effect Containers. Runtime state is never stored on shared graph asset nodes or edges; stock Combo Graph concurrency over its own mutable asset fields remains a manual-verification concern.

No runtime behavior was verified in Unreal during this implementation. See the plugin [README on GitHub](https://github.com/nbertoa/ue5-bertadevkit/blob/main/BertaDevKitHost/Plugins/BertaComboGraphExt/README.md) for the exact contracts.
