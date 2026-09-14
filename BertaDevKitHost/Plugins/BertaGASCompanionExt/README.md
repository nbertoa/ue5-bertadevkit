# BertaGASCompanionExt

Focused Unreal Engine 5.8 diagnostics, validation, targeting, and compatibility helpers for projects that already use GAS Companion.

## Local prerequisite

Install GAS Companion locally at `BertaDevKitHost/Plugins/GASCompanion/`. That commercial plugin is ignored by Git and is never part of the public repository.

`BertaGASCompanionExt` is disabled by default. Enable both `GASCompanion` and `BertaGASCompanionExt` only in projects where the local prerequisite is available.

## Included tools

- `UBertaGSCAbilityActivationLibrary`: structured, side-effect-free activation reports.
- `UBertaGSCAbilityReadinessLibrary`: deterministic actor-wide readiness matrix with simultaneous blockers and Problems Only output.
- `UBertaGSCEffectiveLoadoutLibrary`: effective local ASC snapshot for abilities, active effects, Attribute Sets, and owned tags with evidence-qualified provenance.
- `UBertaGSCTraceComponent`: opt-in event trace with a bounded buffer, no Tick, and no fabricated attribute history.
- `UBertaGSCAbilityQueueInputBridgeComponent`: explicit, runtime-binding-validated Enhanced Input failure forwarding with Ability Actor Info lifecycle rebinding.
- `UBertaGSCAbilitySetValidator`: Editor Data Validation for objective Ability Set configuration errors, including broken or contract-incompatible attribute initialization tables.
- `UBertaGSCContractAuditLibrary`: Editor-only deterministic audit of Ability Sets and GSC Game Feature contracts.
- `UBertaGSCTargetType_DebugProxy`: result-preserving target diagnostics.
- `UBertaGSCTargetType_TargetingPreset`: synchronous UE 5.8 Targeting System preset adapter.

The generic `UBertaGameplayTagUsageFinder` lives in `BertaDevKitEditor`, where it remains independent of GAS Companion. Its Whole Project scope covers `/Game` plus mounted project/plugin content roots, presents cancelable progress, and reports complete or partial scan counts. Full setup and manual verification steps are in the repository documentation.
