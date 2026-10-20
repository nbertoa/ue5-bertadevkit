# BertaUltimateGameplayCameraExt

Optional UE 5.8 Runtime/Editor integration for a licensed local Ultimate Gameplay Camera (UGC) installation. It provides camera-preset cycling, a live debug summary, development console commands, and read-only Editor setup auditing and preset comparison. UGC owns camera evaluation and blending.

## Install and configure

1. Copy `BertaDevKitHost/Plugins/BertaUltimateGameplayCameraExt/` into `<YourProject>/Plugins/BertaUltimateGameplayCameraExt/`.
2. Install the licensed UGC plugin alongside it. The declared plugin dependency is `AuroraDevs_UGC`; UGC source, assets, and binaries are not distributed here.
3. Enable both plugins in the consuming project and build for UE 5.8. This extension is disabled by default and does not require BertaDevKit.
4. Create a **project-owned Blueprint child of the vendor `BP_UGCCameraManager`**, for example `BP_SandboxUGCCameraManager`. This retains the UGC manager inheritance. In its **Class Defaults**, assign **`Default Camera Data`** to your initial project preset, for example `DA_Camera_Medium`.
5. In the **existing** PlayerController Blueprint, assign that child as **`Player Camera Manager Class`** (`PlayerCameraManagerClass`). Keep any GAS Companion, interaction, or other controller inheritance/components already in use. Add `UBertaUGCCameraCycleComponent` to this same PlayerController.
6. Populate the component's ordered **`CameraPresets`** with distinct, valid UGC camera data assets **including the exact asset selected as the manager's `Default Camera Data`**. For example: `[DA_Camera_Medium, DA_Camera_Near, DA_Camera_Far]`.
7. Compile/save and verify the active manager and current preset with `Berta.UGC.Dump` in PIE, then try `Berta.UGC.Next`. Only after confirming transitions, optionally bind your own input actions to `SelectNextCamera()`/`SelectPreviousCamera()` or use `SelectCamera(Index)`.

**Important for preset cycling:** creating the Camera Data Assets or attaching the cycle component **does not activate a preset**. The manager initializes its own default; if it is a **different asset** from the presets in `CameraPresets`, the cycle rejects Next/Previous while that unrelated preset is active. The starting `Default Camera Data` and one element of `CameraPresets` must therefore reference **the same asset object**, not merely assets with matching values or names. The ordering may change, but the initial preset must be present.

This project-owned Blueprint child is the **recommended reproducible setup**, not a separate hard-coded class requirement of the extension: the C++ component accepts any active `AUGC_PlayerCameraManager` or derived manager. The key runtime contract is that the active data asset is absent, or belongs to the cycle. Creating the child lets the project reliably control the default rather than inheriting the vendor demo's preset. The component's selection state is local and non-replicated; it does not bind input, tick, manage possession, or write camera properties.

### Example: near, medium, and far

This is a **reusable smoke-test recipe for three UGC Camera Data Assets**, not three `CameraComponent` objects, nor vendor-provided presets. Create `DA_Camera_Medium` with base class **`UGC_CameraDataAssetBase`**, then duplicate it as `DA_Camera_Near` and `DA_Camera_Far`. Change **only** the `Camera Arm → Arm Length Settings` and `Camera FOV → FOVSettings` fields below.

| Data Asset | Min Arm Length | Max Arm Length | Min FOV | Max FOV | Arm Range Blend Time | FOVRange Blend Time |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `DA_Camera_Medium` | 100 | 250 | 90 | 125 | 0.5 | 0.5 |
| `DA_Camera_Near` | 80 | 120 | 80 | 90 | 0.5 | 0.5 |
| `DA_Camera_Far` | 350 | 450 | 95 | 105 | 0.5 | 0.5 |

**Provenance:** the Medium values above were transcribed from the user's UE 5.8 Sandbox screenshots; Near and Far are **suggested experimental values**, not official manufacturer defaults. The user reported two visible `Next` transitions in PIE after following the manager/default-data setup, but the final serialized asset values were not independently inspected. Keep the other fields inherited from Medium unchanged for this isolated comparison. Arm ranges and FOV ranges are **settings**, not guaranteed final in-game POV values.

For the example to cycle reliably, use the following **same initial asset** in both configurations:

```text
BP_SandboxUGCCameraManager (child of BP_UGCCameraManager)
  Default Camera Data: DA_Camera_Medium

Existing Third Person PlayerController
  Player Camera Manager Class: BP_SandboxUGCCameraManager
  BertaUGCCameraCycleComponent
    CameraPresets[0]: DA_Camera_Medium
    CameraPresets[1]: DA_Camera_Near
    CameraPresets[2]: DA_Camera_Far
```

In PIE use:

```text
Berta.UGC.Dump        # confirm ActivePreset is DA_Camera_Medium
Berta.UGC.Next        # Medium → Near
Berta.UGC.Next        # Near → Far
Berta.UGC.Previous    # Far → Near (test case, not yet user-confirmed)
```

Do not assign gameplay input buttons just to perform this initial test. `Default Camera Data` is **not** configured by adding `BertaUGCCameraCycleComponent`; configure it on the manager first. These examples are a quick comparison exercise, not a recommendation that every game expose three player-selectable camera distances. **Update (2026-10-20):** the Sandbox user confirmed that packaging completed and the packaged application works. The precise build configuration and per-modifier regression tests were not supplied. Full-project backup `07_UGC_CameraPresets_Validated` is the **recommended next checkpoint**, not yet confirmed as created.

### Dynamic Spring Arm and predictive collision — tested UE 5.8 migration

This workflow configures the **licensed UGC plugin's own Spring Arm** inside the existing Third Person Character; it does **not** introduce a new BertaDevKit runtime component. Follow the vendor's [current Dynamic Spring Arm instructions](https://docs.superhuman.com/@aurora-devs/documentation-ultimate-gameplay-camera/new-dynamic-spring-arm-36), not the deprecated pre-UE5.5 Collision Modifier setup.

**Known-good Sandbox experiment (user-reported PIE, 2026-10-20):**

1. In the existing `BP_ThirdPersonCharacter`, inspect the native `CameraBoom` Spring Arm and note the `FollowCamera` socket and settings before changing anything.
2. Add UGC's **`BP_UGCSpringArmComponent`** as a sibling of the old `CameraBoom` under the Character's `CapsuleComponent` **temporarily**; do not attach the new arm under the old arm.
3. Transfer the relevant original Spring Arm settings to the new UGC Spring Arm. For the Sandbox they were: **Target Arm Length 400**, **Socket Offset (0,0,0)**, **Target Offset (0,0,0)**, **Use Pawn Control Rotation enabled**, **Inherit Pitch/Yaw/Roll enabled**, **Do Collision Test enabled**, **Probe Size 12**, **Probe Channel Camera**, and **Camera Lag/Rotation Lag disabled**. Leave the UGC-specific predictive-collision defaults unchanged for the first test.
4. **Reparent the existing `FollowCamera`** to `BP_UGCSpringArmComponent` and explicitly set **FollowCamera → Parent Socket = `SpringEndpoint`**, matching the socket used under the original `CameraBoom`. Confirm camera **relative Location/Rotation = (0,0,0)**.
5. **Remove the original native `CameraBoom`** after the reparenting: the final tested Character has **only one Spring Arm**, UGC's, with `FollowCamera` as its child. Compile and save.
6. In PIE, verify the Near/Medium/Far presets still produce their expected changes in distance/composition. Rotate against a wall to observe collision interpolation, then move around a corner to look for predictive avoidance.

```text
BP_ThirdPersonCharacter (keeps its existing Character / GAS setup)
└── CapsuleComponent
    └── BP_UGCSpringArmComponent    (the only Spring Arm)
        └── FollowCamera
            Parent Socket: SpringEndpoint
            Relative Location: (0,0,0)
            Relative Rotation: (0,0,0)
```

**Pitfall directly observed:** leaving **both** `CameraBoom` and `BP_UGCSpringArmComponent` present while attaching `FollowCamera` to the new arm made the camera appear **too centered on the character**, and switching Near/Medium/Far changed distance **only slightly**. After the user **deleted `CameraBoom`**, framing/preset transitions behaved properly. UGC may have targeted the wrong arm, but **no before/after runtime `Berta.UGC.Dump` confirmed the internal mechanism**. Therefore **do not keep two competing Spring Arms in this configuration**; do not claim that every Unreal Pawn with two Spring Arms is categorically broken. For a non-removable inherited Spring Arm, inspect ownership/parent class instead of forcing deletion.

**Evidence and verification boundary:** the user observed **smooth interpolation at walls** and **anticipation at a corner** in PIE with the UGC arm; this is positive visual behavior, **not an instrumented feeler trace**. Earlier Windows packaging and project backup 07 predate this Spring Arm change; **packaging with the updated Character is still pending**. Retest Look, Jump/Sprint/Stamina/HUD and interaction after replacing the boom, and rebuild the package before describing this migration as packaged-tested.

Detailed chronology and current project state: [UGC Sandbox integration report](https://github.com/nbertoa/rnd-prototyping-engineer/blob/main/research/plugins/ultimate-gameplay-camera-sandbox-integration.md#update--dynamic-spring-arm-migration-and-predictive-collision-2026-10-20).

### Troubleshooting: `success=false` and `storedCycleIndex=-1`

In the Sandbox, `Berta.UGC.Next` initially reported `success=false` and `storedCycleIndex=-1` while the UGC manager was using a **vendor Data Asset**. Editing project presets did not affect the camera, and **adding that vendor asset to `CameraPresets` alone did not fix the failure**. The user identified the missing manager-child/default-data setup from an earlier project. After we prescribed a child manager with `Default Camera Data = DA_Camera_Medium` and reassigned the existing PlayerController, the user reported two successful Next changes in PIE.

This is a **verified functional resolution, not a forensically established single root cause**: no full before/after runtime log or Blueprint inspection was captured. Do not assume every `false` is an unrelated default. The same result can arise from an empty array, invalid/duplicate entries, missing/wrong manager, an active asset outside the cycle, invalid index, or failure to retain the pushed asset. `-1` alone says only that the stored index was not set.

To diagnose:

1. Run `Berta.UGC.Dump` and inspect **`Manager`**, **`ActivePreset`**, and **`CycleActiveIndex`**. A valid UGC manager **does not imply** that your intended data asset is active.
2. Confirm the **PlayerController in the actual GameMode** uses the project manager child and that its **`Default Camera Data` is in the component's `CameraPresets` array** (asset identity, no `None` or duplicates).
3. Re-run `Berta.UGC.Next` and Dump. For static checks use **Tools → Audit Ultimate Gameplay Camera**; it does not validate every runtime rejection.
4. If it still fails, inspect the precise early-return path in `UBertaUGCCameraCycleComponent::SelectAdjacentCamera`/`SelectCamera` before changing the Character, SpringArm, or input. Some failure paths lack a specific runtime warning, so the last console line is not a complete diagnosis.

Full account of the experiment and failure chronology: [R&D Engineer — UGC Sandbox integration](https://github.com/nbertoa/rnd-prototyping-engineer/blob/main/research/plugins/ultimate-gameplay-camera-sandbox-integration.md).

## Preset selection contract

Each operation reads UGC's current data asset rather than treating the stored cycle index as authoritative. Indices are zero-based.

| Current state | Result |
| --- | --- |
| No active asset | Next selects the first preset; Previous selects the last; direct selection uses the requested index. |
| Active asset belongs to the array | Next/Previous wrap around the array from that active asset. |
| Requested preset is already active | Success without popping or pushing camera data. |
| An unrelated asset is active, such as a temporary aim camera | Selection returns `false` without changing the stack or index. |
| Empty array, invalid or duplicate preset, invalid index, or unavailable UGC manager | Selection returns `false`. |

A real change pops only the current stack head and pushes the requested preset, leaving UGC responsible for the transition. Selection reports failure if UGC does not keep that preset active after the push; this error does not imply a rollback of UGC's stack.

## Runtime diagnostics

In non-Shipping builds, call `UBertaUGCCameraDebugLibrary::GetUGCCameraDebugSummary(PlayerController, OutSummary)` for one text snapshot of:

- the camera manager and Pawn;
- the active UGC preset and active cycle index;
- the final cached POV and SpringArm properties;
- UGC and external camera-modifier counts.

The call returns `false` and clears the output for a non-local controller, a non-UGC manager, or Shipping. The summary does not inspect the stack below its head or modifier priorities. UGC's `ShowCameraModifiersDebug` remains available for detailed modifier inspection.

### Console commands

| Command | Operation |
| --- | --- |
| `Berta.UGC.Next [LocalPlayerIndex]` | Select the next configured preset. |
| `Berta.UGC.Previous [LocalPlayerIndex]` | Select the previous configured preset. |
| `Berta.UGC.Select <CameraIndex> [LocalPlayerIndex]` | Select a preset by zero-based index. |
| `Berta.UGC.Dump [LocalPlayerIndex]` | Print the read-only debug summary. |

Commands are registered only in non-Shipping builds and act only in the console's game world. With one local player, `LocalPlayerIndex` is optional; with split screen, it is required. Selection commands require the camera-cycle component and preserve its rejection of unrelated active presets. Invalid arguments, missing controllers/components, and selection failures appear in Output Log under `LogBertaUltimateGameplayCameraExt`.

For example, for the first local player:

```text
Berta.UGC.Select 0 0
Berta.UGC.Next 0
Berta.UGC.Dump 0
```

## Editor setup audit

Choose **Tools → Audit Ultimate Gameplay Camera** and inspect Output Log under `LogBertaUGCAudit`. The pass is read-only and checks:

- the global GameMode's PlayerController and concrete camera-manager class, with full paths;
- the manager's effective **Default Camera Data** against the controller's effective `CameraPresets`, including each index and full asset path;
- source-backed range and blend-duration rules in `/Game` UGC presets;
- explicit Blueprint writes to SpringArm arm length/offset and CameraComponent FOV, plus calls to `SetFieldOfView` and `SetControlRotation`.

| Finding | Interpretation |
| --- | --- |
| Non-finite range or negative/non-finite blend time | Error. |
| Inverted Min/Max range | Warning; UGC's mapped values do not prove every reversal is invalid. |
| Global GameMode has a non-UGC manager | Warning; a map-specific GameMode override may use UGC. |
| Valid default is in a valid cycle, at any index | Info: statically consistent object membership, not proof that Next succeeds during gameplay. |
| Valid default is outside a valid cycle | Warning: conditional blocking risk if that asset becomes active. |
| Default is unassigned | Info: no statically configured initial preset; not automatically an error. |
| Empty, null/invalid, or duplicate presets | Warning: the cycle cannot select with that configuration; no compatibility success is reported. |
| Missing/unsupported default property, stale Blueprint defaults, unresolved or multiple cycle templates | Warning marked **INCONCLUSIVE**; no compatibility conclusion. |
| No declared cycle component | Info: cycling is optional, with no array to compare. |
| Explicit Blueprint camera write | Info; its runtime target and execution must be verified before declaring a conflict. |

### Effective defaults and component inheritance

In the inspected local UGC installation, **Default Camera Data** is the hard object property **`DefaultCameraData`** declared by the vendor `BP_UGCCameraManager` generated class. Its declared object class is the vendor's `UGC_CameraDataAsset_C`, derived from `UUGC_CameraDataAssetBase`. It is not a native default accessor on `AUGC_PlayerCameraManager`.

The audit uses `FindFProperty` with that exact verified name, checks its scalar hard-object type and camera-data class, and reads it through `FObjectProperty`/`FObjectPropertyBase::GetObjectPropertyValue_InContainer` on the **selected concrete class's `GetDefaultObject()`**. An inherited value and a child Blueprint CDO override are both inspected correctly. It never guesses from a Details label, spawns a camera manager, calls its startup logic, or substitutes `GetCurrentCameraDataAsset()` on a CDO. A missing field or an unsupported type, including a soft reference, is inconclusive rather than implicitly valid. The native manager and arbitrary valid UGC derivatives remain accepted manager classes; a vendor Blueprint child is a setup recommendation, not a class-name requirement.

For the cycle component, native defaults come from the concrete PlayerController CDO. SCS declarations are located through the Blueprint hierarchy and resolved with `USCS_Node::GetActualComponentTemplate` against the most-derived generated class. UE 5.8 consults existing `UInheritableComponentHandler` overrides along that hierarchy without creating records. Thus a component declared in a parent can use the child's preset override instead of stale parent data. Missing effective templates, uncompiled classes, and ambiguous multiple cycle components are inconclusive. No construction script or gameplay initialization is executed. Pawn SpringArm/Camera checks retain their existing declared-component scope; this change does not broaden them into a general component resolver.

### Interpreting and correcting a mismatch

Comparison uses **object identity**, matching Camera Cycle's actual contract, not asset names or equal settings. The report first identifies the manager/default, then prints the component/template and `CameraPresets[N]` entries. If the default is absent, its Warning explains that **if this asset becomes active**, Next/Previous and direct Select reject changes while it remains outside the cycle.

In the manager's **Class Defaults**, choose a default that belongs to the intended cycle, preferably a project-owned preset such as `DA_Camera_Medium`. Alternatively edit the PlayerController component's effective **CameraPresets** to include the default only if it really belongs among selectable presets. Compile the affected Blueprints and rerun the audit. **Index zero is a convention, not a requirement**: a default at index 1 or another index is equally consistent. Do not add vendor assets indiscriminately.

A null configured default does not prove that runtime has no active asset. If runtime truly has no active asset and the array is valid, Next starts at index zero and Previous at the last index. UGC's Blueprint initialization and later camera operations can choose other data; its current-data getter reads the runtime stack head, which need not remain the configured default. Neither `storedCycleIndex=-1` nor a default mismatch identifies the exact cause of an earlier `success=false`, and correcting this warning does not promise every selection succeeds.

### Static scope and manual verification

Pawn-specific UGC checks apply only when the Global Default GameMode uses a UGC manager. The active map may select another GameMode; a controller or manager can be replaced dynamically, and UGC can temporarily activate data outside the cycle. The audit does not inspect C++ writes, map level scripts, map-specific GameMode overrides, or all dynamic camera changes. Unloadable assets and an incomplete Asset Registry are reported instead of being counted as clean. It reads compiled defaults/templates and never writes or saves packages, Blueprints, CDOs, or provider assets.

The two Editor Automation Tests `BertaDevKit.UGC.Editor.Audit.DefaultInheritance` and `BertaDevKit.UGC.Editor.Audit.ComponentInheritance` use transient Blueprint fixtures and camera-data objects, not redistributable vendor assets. They cover inherited/default overrides, null/missing defaults, component overrides, repeated reads, and stale-class handling. They require an explicitly authorized Unreal session to execute; compilation alone does not pass them. The private Editor `UnrealEd` dependency supports fixture creation/compilation and does not affect Runtime or the base plugins.

Manual Sandbox checklist, to be executed by Nicolas after compiling the affected Blueprints:

1. Set `DA_Camera_Medium` as the manager's default and include that exact asset alongside Near/Far in the controller's cycle. Run **Tools → Audit Ultimate Gameplay Camera**. Under `LogBertaUGCAudit`, verify `[UGC Audit]`, full GameMode/controller/manager/template paths, indexed assets, and Info showing the matching index.
2. Reorder the array so Medium is not index zero. Rerun and verify Info reports its actual new index with no mismatch Warning.
3. Temporarily select a valid default absent from the array. Verify a conditional Warning identifies that default and the listed cycle, describes the blocking condition, and gives Class Defaults/component remedies without claiming a proven historical failure.
4. Clear the default and rerun. Expect Info for no configured initial preset, not an Error or a guarantee that runtime has no active asset.
5. Use a manager Blueprint child inheriting the parent's default, then override it in the child. Verify the audit reads the selected concrete child's effective CDO each time. Restore the intended default.
6. Declare the cycle on a parent PlayerController Blueprint and override/reorder its presets in the selected child. Verify the child's template path/values are reported, with parent values unchanged. Compile both classes first; an uncompiled class should produce INCONCLUSIVE rather than a parent fallback.
7. Separately test an empty array, a null element, a duplicate, absence of the optional component, and a non-UGC manager. Expect the corresponding warnings/information and no false compatibility success. Restore the working setup afterward.
8. Run the audit twice without editing anything. Verify no new dirty assets/packages or provider modifications result. If a default/template cannot be inspected, verify INCONCLUSIVE is visible and no consistency claim follows.
9. Restore the correct manager/default/cycle, then manually start PIE and run `Berta.UGC.Dump`, `Berta.UGC.Next`, and `Berta.UGC.Previous`. Confirm the existing preset transitions and other gameplay still work. This static audit does not change those commands or Camera Cycle's runtime behavior.

## Compare presets

Select exactly two UGC camera data assets in Content Browser, choose **Compare UGC Camera Presets**, and inspect Output Log under `LogBertaUGCPresetDiff`. The read-only comparison prints changed UGC base-property paths with both values.

It descends into reflected structs and instanced settings, reports array element/count changes, and compares curve and object references by asset path. Different concrete classes produce a `Class` difference while common UGC base settings are still compared. Curve contents and custom fields defined only by a data-asset subclass are outside its scope. A recursion-limit result is reported as truncated/inconclusive separately from confirmed differences.

## Verification scope

This is an optional commercial integration. The default seven-plugin host build and documentation workflow do not compile or exercise it. Compile it separately with the licensed UGC dependency installed and the extension explicitly enabled. Editor, Blueprint, console, and camera-transition behavior require a separately authorized Unreal session; see [Development](../development/index.md).

The [copied-plugin README](https://github.com/nbertoa/ue5-bertadevkit/blob/main/BertaDevKitHost/Plugins/BertaUltimateGameplayCameraExt/README.md) remains available with the plugin.
