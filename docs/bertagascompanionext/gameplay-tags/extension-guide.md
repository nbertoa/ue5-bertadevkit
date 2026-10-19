# Extending and maintaining the catalog

## Source of truth and approval baseline

The native INI rows in the plugin's `Config/Tags` directory define the registered catalog. The 23 family pages explain their semantics. `Scripts/Validate-GameplayTagCatalog.py` independently checks those files and documentation with Python's standard library; it is not a generator and Unreal does not run it.

The validator's `APPROVED` table contains each approved family's explicit count and SHA-256 fingerprint of its canonical names. A fingerprint hashes **lexically sorted full names, joined with LF, with one trailing LF, encoded as UTF-8**. This compact approval snapshot detects a same-count rename or replacement without keeping a second 474-name inventory. It does not duplicate comments or determine runtime registration.

For this release the contract is exactly 474 names and 23 families. Do not update the approval fingerprint merely to make a failing check pass. A future deliberate vocabulary change requires review of the new/removed names and semantic compatibility, then an explicit update to the baseline, counts, docs, and validation record in the same change. Fingerprints prove unchanged names relative to a reviewed snapshot; human review is still required to establish the correctness of that snapshot and the meaning of comments.

## Add one reusable tag

1. **Search for equivalence.** Search the family pages and all project/plugin definitions. Check whether an existing tag already expresses the concept, including whether composition solves the need. A new synonym creates long-term ambiguity.
2. **Choose scope and owner.** Decide whether this is ability identity, actor state, status, an event, a classification, or another approved domain. Write who owns it and whether it is stable metadata, an interval, a query predicate, or an occurrence.
3. **Choose the hierarchy.** Preserve `Berta.*`, or `GameplayCue.Berta.*` for cues. Use precise PascalCase segments and avoid project content identifiers. Account for new children changing the meaning of existing parent queries.
4. **Check collisions.** Search exact names and semantic equivalents across native tags, data tables, project config, and enabled plugin config. Check source basenames too. Never add a second declaration of an existing Berta tag to another source.
5. **Edit the appropriate INI source.** Add one complete `+GameplayTagList` row under `[/Script/GameplayTags.GameplayTagsList]`, keeping entries sorted by canonical name. Do not explicitly add hierarchy parents unless the parent is intentionally approved as a first-class concept.
6. **Write a meaningful `DevComment`.** Explain the concept concisely in original English. Escape no arbitrary quote-heavy payloads into these simple rows; use plain prose supported by the validator's intentionally narrow syntax.
7. **Document the individual tag.** Add its level-three canonical-name heading in the correct family reference page. Include Meaning, Scope and suggested owner, Lifecycle, Example, and Implementation responsibility. Explain adjacent concepts and add Related entries/Important distinctions where useful.
8. **Link both directions when helpful.** A new state may relate to an ability, event, or cue but must not imply an automatic connection. Explain the different lifetimes. Use canonical page anchors and build the site to check them.
9. **Approve the name-set change.** Review the actual INI name diff, then update the family's count and fingerprint in `APPROVED`, `EXPECTED_TOTAL` when needed, the overview table, family heading count, and any affected count claims. This is a deliberate catalog revision, not an accidental relaxation of the original 474-tag contract.
10. **Run static validation:** `python Scripts/Validate-GameplayTagCatalog.py` from the repository root. It must report consistent counts, approved fingerprints, comments, and one dedicated reference entry per tag.
11. **Build documentation:** install `requirements-docs.txt` if needed, then run `mkdocs build --strict`. Resolve errors and broken internal links; inspect the generated reference page for readability.
12. **Compile when needed.** If registration or dependencies changed, compile the relevant UE 5.8 target through UBT, explicitly enabling the optional extension. Do not count a base host build as extension coverage.
13. **Perform manual Unreal verification when required.** Restart Editor, inspect source/tooltip/picker, exercise affected references, and verify packaged/runtime behavior when relevant. Launching Unreal requires Nicolas's explicit authorization during automated work.
14. **Review compatibility and the complete diff.** Keep changes limited to the approved vocabulary and integration. Include no commercial source or unrelated assets. Preserve serialized users or provide a migration plan.

For local validation, use:

```powershell
python Scripts/Validate-GameplayTagCatalog.py
python -m pip install --requirement requirements-docs.txt
mkdocs build --strict
```

The checker validates structure and coverage; it cannot judge whether a new description is semantically correct. The MkDocs build checks documentation, not UE runtime registration. Do not run `Scripts/Validate-BertaDevKit.ps1` for a compile-only check: that runner also launches an Unreal commandlet.

## Add a family

Require a concrete reusable semantic domain that existing families cannot express clearly. Decide whether a new thematic INI source improves ownership or whether an existing one remains cohesive. Add the family to the approval table, its count/fingerprint, the total, a dedicated family introduction/reference page, navigation, and the overview table. Teach the checker its page slug if the default lowercase family name is unsuitable. Review every existing family for semantic overlap before approving the addition.

No new module, plugin, runtime registry, or custom discovery mechanism is needed merely because another native tag family exists. Native hierarchy matching already provides grouping; do not add explicit parent rows solely to increase discoverability or inflate counts.

## Project-specific tags

Add game-specific names to the game's own `Config/Tags` source or other deliberately chosen native mechanism, for example under `MyGame.*`; use `GameplayCue.MyGame.*` for its cues. Do not edit the shared Berta catalog to describe one game's named boss, quest, faction, weapon instance, or ability variant unless its meaning is truly reusable.

A project may reference Berta tags alongside its own labels. Use explicit mappings where local terminology is equivalent, rather than redeclaring the same canonical name in two sources. Keep project-owned tags outside this plugin's validator scope. A source file's name should remain distinctive across all enabled plugins because Unreal tracks additional INI sources by clean filename.

## Deprecating and renaming

Treat tag names as serialized API. Blueprint defaults, Gameplay Effects, ability triggers, queries, data assets, save data, and code can reference them. A direct text rename of one INI row can invalidate those users even though the replacement name appears in the picker. C++ string literals and external data also need explicit migration; a redirect is not a substitute for reviewing them.

Prefer preserving a stable name and clarifying its documentation when meaning has not actually changed. For a deprecated concept, document the replacement and discourage new usage while retaining old definitions until migration is approved. The approved snapshot must reflect any deliberately retained declarations and updated counts. Do not silently remove a widely used name or repurpose it for incompatible meaning.

For a genuine rename, Unreal supports native `GameplayTagRedirects`. A project-level illustrative redirect in `Config/DefaultGameplayTags.ini` uses:

```ini
[/Script/GameplayTags.GameplayTagsSettings]
+GameplayTagRedirects=(OldTagName="MyGame.LegacyAction",NewTagName="MyGame.Action")
```

This is a syntax example, not a Berta redirect installed by the catalog. UE 5.8 also exposes redirects on `UGameplayTagsList` additional sources and imports them from those sources. Prefer the appropriate shared source only when the catalog itself owns an approved rename; project-only migrations belong in project config. Do not use the deprecated Engine.Engine redirect location.

A migration should identify all old references, register the destination, add the redirect in the appropriate native source, update code/config consumers, restart Editor, inspect redirect resolution, resave affected assets deliberately, and test saved/packaged/network data where applicable. Avoid redirect loops, conflicting mappings, chains, and removal before all supported serialized versions are accounted for. Native deserialization paths may redirect old references, while arbitrary string comparisons or custom save formats require their own migration review.

Use the independent `UBertaGameplayTagUsageFinder` to inspect asset references before and after migration. Its scan completeness/partial results must be considered; no static text scan can prove all binary assets or external saves were updated. This release introduces no redirects because no approved existing name was renamed.

## Compile-only verification

Resolve the UE 5.8 installation from `UE_5_8_ROOT` or the Epic Launcher manifest, following the repository's [development instructions](../../development/index.md). Call the resolved `Engine/Build/BatchFiles/Build.bat` with:

```text
BertaDevKitHostEditor Win64 Development -Project="<absolute host .uproject path>" -EnablePlugin=BertaGASCompanionExt -WaitMutex -NoHotReloadFromIDE
```

The UBT switch is singular **`-EnablePlugin=`** in the inspected UE 5.8 `TargetRules.cs`. It includes the optional plugin for that build without editing the tracked `.uproject`. Inspect compilation/link output for **both** BertaGASCompanionExt and BertaGASCompanionExtEditor. A missing GAS Companion installation leaves this verification incomplete.

Windows may reject generated paths longer than 260 characters for a deeply nested checkout. A temporary `subst` drive pointing at the same directory can shorten the project path without relocating files. Choose an unused drive and remove only that mapping after the build. This is a machine-local workaround, not a repository configuration change or an Unreal version compatibility layer.
