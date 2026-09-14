# BertaGASCompanionExt

Focused Unreal Engine 5.8 diagnostics, validation, targeting, and compatibility helpers for projects that already use GAS Companion.

## Local prerequisite

Install GAS Companion locally at `BertaDevKitHost/Plugins/GASCompanion/`. That commercial plugin is ignored by Git and is never part of the public repository.

`BertaGASCompanionExt` is disabled by default. Enable both `GASCompanion` and `BertaGASCompanionExt` only in projects where the local prerequisite is available.

## Included tools

- `UBertaGSCAbilityActivationLibrary`: structured, side-effect-free activation reports.
- `UBertaGSCTraceComponent`: opt-in event trace with a bounded buffer and no Tick.
- `UBertaGSCAbilityQueueInputBridgeComponent`: deferred public-state reconciliation for the documented Enhanced Input queue gap.
- `UBertaGSCAbilitySetValidator`: Editor Data Validation for objective Ability Set configuration errors.
- `UBertaGSCTargetType_DebugProxy`: result-preserving target diagnostics.
- `UBertaGSCTargetType_TargetingPreset`: synchronous UE 5.8 Targeting System preset adapter.

The generic `UBertaGameplayTagUsageFinder` lives in `BertaDevKitEditor`, where it remains independent of GAS Companion. Full setup and manual verification steps are in the repository documentation.
