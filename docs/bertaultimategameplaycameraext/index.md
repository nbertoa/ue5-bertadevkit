# BertaUltimateGameplayCameraExt

Optional UE 5.8 Runtime/Editor integration for a licensed local Ultimate Gameplay Camera (UGC) installation. It provides camera-preset cycling, a live debug summary, development console commands, and read-only Editor setup auditing and preset comparison. UGC owns camera evaluation and blending.

## Install and configure

1. Copy `BertaDevKitHost/Plugins/BertaUltimateGameplayCameraExt/` into `<YourProject>/Plugins/BertaUltimateGameplayCameraExt/`.
2. Install the licensed UGC plugin alongside it. The declared plugin dependency is `AuroraDevs_UGC`; UGC source, assets, and binaries are not distributed here.
3. Enable both plugins in the consuming project and build for UE 5.8. This extension is disabled by default and does not require BertaDevKit.
4. On your PlayerController, set `PlayerCameraManagerClass` to `AUGC_PlayerCameraManager` or its Blueprint child and add `UBertaUGCCameraCycleComponent`.
5. Fill the component's ordered `CameraPresets` array with distinct, valid UGC camera data assets. Bind your own input actions to `SelectNextCamera()` and `SelectPreviousCamera()`, or call `SelectCamera(Index)` directly.

Adding the component does not activate a preset. Selection state is local and non-replicated; the component does not bind input, tick, manage possession, or write camera properties.

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

- the global GameMode's PlayerController and camera-manager class;
- camera-cycle presets on that controller;
- source-backed range and blend-duration rules in `/Game` UGC presets;
- explicit Blueprint writes to SpringArm arm length/offset and CameraComponent FOV, plus calls to `SetFieldOfView` and `SetControlRotation`.

| Finding | Interpretation |
| --- | --- |
| Non-finite range or negative/non-finite blend time | Error. |
| Inverted Min/Max range | Warning; UGC's mapped values do not prove every reversal is invalid. |
| Global GameMode has a non-UGC manager | Warning; a map-specific GameMode override may use UGC. |
| Explicit Blueprint camera write | Info; its runtime target and execution must be verified before declaring a conflict. |

Pawn-specific UGC checks apply only when the Global Default GameMode uses a UGC manager. The audit does not inspect C++ writes, map level scripts, or map-specific GameMode overrides. Blueprint-inherited component overrides may require manual inspection. Unloadable assets and an incomplete Asset Registry are reported instead of being counted as clean.

## Compare presets

Select exactly two UGC camera data assets in Content Browser, choose **Compare UGC Camera Presets**, and inspect Output Log under `LogBertaUGCPresetDiff`. The read-only comparison prints changed UGC base-property paths with both values.

It descends into reflected structs and instanced settings, reports array element/count changes, and compares curve and object references by asset path. Different concrete classes produce a `Class` difference while common UGC base settings are still compared. Curve contents and custom fields defined only by a data-asset subclass are outside its scope. A recursion-limit result is reported as truncated/inconclusive separately from confirmed differences.

## Verification scope

This is an optional commercial integration. The default seven-plugin host build and documentation workflow do not compile or exercise it. Compile it separately with the licensed UGC dependency installed and the extension explicitly enabled. Editor, Blueprint, console, and camera-transition behavior require a separately authorized Unreal session; see [Development](../development/index.md).

The [copied-plugin README](https://github.com/nbertoa/ue5-bertadevkit/blob/main/BertaDevKitHost/Plugins/BertaUltimateGameplayCameraExt/README.md) remains available with the plugin.
