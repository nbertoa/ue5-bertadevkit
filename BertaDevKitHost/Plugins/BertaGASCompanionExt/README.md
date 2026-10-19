# BertaGASCompanionExt

Focused Unreal Engine 5.8 diagnostics, validation, targeting, and compatibility helpers for projects that already use GAS Companion.

## Local prerequisite

Install a licensed GAS Companion copy as a discoverable UE 5.8 engine plugin or locally at `BertaDevKitHost/Plugins/GASCompanion/`. That commercial plugin is never part of the public repository; the project-local directory is ignored by Git.

`BertaGASCompanionExt` is disabled by default. Enable both `GASCompanion` and `BertaGASCompanionExt` only in projects where the local prerequisite is available.

## Universal Gameplay Tags catalog

The plugin includes a reusable native Gameplay Tags vocabulary under `Config/Tags`. Preserve that directory when installing the plugin. Its existing Runtime module registers the directory using Unreal's plugin/path APIs; no project-specific paths or content assets are needed. Keep **Import Tags From Config** enabled and restart Editor after enabling the plugin or changing the INI sources. Inspect sources and tooltip descriptions in **Project Settings → Gameplay Tags → Manage Gameplay Tags**.

Names use `Berta.*`, with `GameplayCue.Berta.*` for cosmetic cue identifiers. Projects may adopt any subset. Registration supplies vocabulary only: no ability grants, actor-owned state, effects, input bindings, events, cooldown implementation, numeric attributes, cues, or custom replication are installed. The catalog uses no GAS Companion-specific behavior, although the surrounding optional extension still requires its existing prerequisites. A future independent extraction can be considered if a concrete reuse requirement emerges.

The [comprehensive MkDocs source](../../../docs/bertagascompanionext/gameplay-tags/index.md) and [published guide](https://nbertoa.github.io/ue5-bertadevkit/bertagascompanionext/gameplay-tags/) cover every individual tag, semantics, Blueprint/C++/GAS integration, manual GA_Jump adoption, extension/migration, and pending manual verification. INI rows define the registered name set; MkDocs is the comprehensive reference.

Run `python Scripts/Validate-GameplayTagCatalog.py` and `mkdocs build --strict` from the repository root for static/catalog and documentation validation. Neither proves runtime registration. Compile the optional extension explicitly with UBT's `-EnablePlugin=BertaGASCompanionExt`; see the maintenance guide for prerequisites and manual checks.

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
