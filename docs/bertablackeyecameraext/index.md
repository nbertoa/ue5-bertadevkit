# BertaBlackEyeCameraExt

BertaBlackEyeCameraExt is an optional UE 5.8 Runtime/Editor extension for temporary reveals with Black Eye cine cameras. Black Eye controls Follow, LookAt, Orbit, Shot Lists, procedural camera behavior, and any Black Eye manager/priority stack. Berta coordinates one local reveal session, its return and input ownership, generic participants, prototype camera cycling, diagnostics, and Editor authoring checks. It uses the local `PlayerController` and standard `SetViewTargetWithBlend`; `ABlackEyeCameraManager` is not required.

## Requirements and installation

Install a legitimate Black Eye Camera 2.0 copy separately, copy `BertaDevKitHost/Plugins/BertaBlackEyeCameraExt/` to `<YourProject>/Plugins/BertaBlackEyeCameraExt/`, enable **Black Eye** (`Black_Eye`) and **BertaBlackEyeCameraExt**, then build the UE 5.8 C++ project. The extension is disabled by default and depends on `Black_Eye`; Black Eye is not distributed in this repository. The locally inspected API is version `2.0.7` for UE 5.8. Compatibility with other vendor versions has not been verified.

## Reveal workflows

For a zone, place an `ABertaBlackEyeCameraTrigger`, assign a same-world `ABlackEyeCineCameraActorBase` on its `RevealComponent.TargetCamera`, and choose `EndMode`:

| Trigger EndMode | Start/exit | Reveal component mode |
| --- | --- | --- |
| `Timed` | Local player's Pawn overlaps; exit after HoldTime | Timed |
| `OnEndOverlap` | Pawn overlaps; exit when that matching Actor leaves or is destroyed | Manual |
| `Manual` | No overlap start; call trigger Start/Stop | Manual |

Default `CanActorTrigger` accepts locally controlled Pawns. The trigger's `bEnabled` gates overlap and explicit Start; calling `SetTriggerEnabled(false)` during a reveal also requests Stop. A successful wrapper start consumes `bTriggerOnce`; `ResetCameraTrigger` clears that use immediately if idle or after a controlled exit. Stop does not reset one-shot use.

For a scripted boss/puzzle event, add `UBertaBlackEyeCameraRevealComponent` to any Actor, assign `TargetCamera`, and call `StartCameraReveal()`—for example, at a health threshold. Component `DurationMode = Timed` exits after HoldTime; `Manual` needs `StopCameraReveal()`. C++ can specify a local player with `StartCameraRevealForController(PlayerController, Mode)`. Start returns false for invalid camera/world/controller/manager, timing, or lifecycle state. `ResetCameraReveal()` is an alias of controlled Stop, not a hard cancel. `GetRevealState()` exposes Idle, BlendingIn, Holding, Active, and BlendingOut; CameraReached reports elapsed BlendIn time, not measured visual convergence.

## Settings, return, and ownership

`FBertaBlackEyeRevealSettings` groups BlendIn/Hold/BlendOut durations, blend functions/exponents, outgoing-POV lock flags, and optional Move/Look/Full input restrictions. Durations may be zero; HoldTime matters only in Timed sessions. Timing and exponents must be finite and nonnegative. Blend parameters go to the active `PlayerCameraManager` through `SetViewTargetWithBlend`, whose implementation determines the actual transition. Use `InlineSettings` or a `UBertaBlackEyeRevealPreset` Data Asset; the preset holds settings only and effective settings are copied at session start.

Move and Look restrictions add then remove this reveal's `SetIgnoreMoveInput`/`SetIgnoreLookInput` contributions. Full input pushes a temporary high-priority, binding-free `UInputComponent` blocker and removes it on cleanup. Other systems' input contributions are not deliberately reset. The reveal also captures ControlRotation: on a normal return it restores before BlendOut and again after its duration, before releasing the Look contribution.

`ReturnTargetPolicy` is snapshotted at Start:

| Policy | Target at exit | Fallback |
| --- | --- | --- |
| `CapturedViewTarget` (default) | ViewTarget before reveal | Captured Pawn → controller |
| `CurrentPawn` | Controller's Pawn at exit, including new possession | Captured ViewTarget → controller |
| `ExplicitTarget` | Any valid Actor in `ExplicitReturnTarget` | Captured ViewTarget → current Pawn → controller |

The active explicit actor reference is weak; its loss produces a warning and fallback. `ExternalCameraChangePolicy` is also snapshotted. Default `RestoreConfiguredTarget` performs the return even after another ViewTarget change. `RespectExternalChange` checks the current/pending ViewTarget **at exit before Berta's own return**; after an external takeover it leaves camera and ControlRotation intact and immediately removes Berta-owned locks and participant/gameplay contributions. This is not continuous arbitration: a Manual reveal may remain active until Stop. Loss of TargetCamera uses its separate cleanup path.

## Participants and hooks

With `bNotifyParticipants`, distinct valid Actors implementing `IBertaCinematicParticipant` receive `OnCinematicPause(Source)` and, if still valid, `OnCinematicResume(Source)` only for a pause this session actually sent. Source is the reveal component. The Actor decides how multiple Sources combine, such as counting outstanding Sources before resuming. A project boss might pause movement/brain logic on Pause and resume it on Resume; the extension supplies no AI/GAS policy.

Both reveal component and trigger expose Started, CameraReached, Ending, and Finished delegates. Started occurs before BlendIn request, CameraReached after the requested BlendIn duration, Ending before the return/ownership decision, and Finished after cleanup; normal Finished is suppressed during EndPlay. Blueprint gameplay hooks (`ApplyGameplayLock`, `RemoveGameplayLock`, `OnCinematicStarted`, `OnCinematicEnding`, `OnCinematicFinished`) can add project-specific behavior independently of C++ input locks.

## Camera switcher and diagnostics

Attach `UBertaBlackEyeCameraSwitcherComponent` to a local PlayerController, configure a unique same-world camera list, then call `SelectCamera(Index)`, `SelectNextCamera()`, or `SelectPreviousCamera()`. Invalid/empty/duplicate lists, bad timing, or an unusable controller reject selection. Selecting the already-current camera succeeds without another blend. Next/Previous wrap; from outside the list they choose first/last respectively. `GetSelectedCameraIndex`, `GetSelectedCamera`, and `IsCameraSelectionValid` inspect the configuration/current ViewTarget. The switcher keeps no return stack or Black Eye priority state.

In non-Shipping builds, `UBertaBlackEyeCameraDebugLibrary::GetBlackEyeCameraDebugSummary` reports the local controller/manager, current target and rotation, active reveal state/settings/policies, and last replayable component. It returns false with explanatory text for an unusable controller; in Shipping it returns false with an unavailable message. The commands use a game world and local player: omit `LocalPlayerIndex` only when there is exactly one.

| Console input | Use |
| --- | --- |
| `Berta.BlackEye.Dump [LocalPlayerIndex]` | Print summary; no switcher required. |
| `Berta.BlackEye.Next [LocalPlayerIndex]` / `Previous [LocalPlayerIndex]` | Cycle a controller switcher. |
| `Berta.BlackEye.Select <CameraIndex> [LocalPlayerIndex]` | Choose a switcher index. |
| `Berta.BlackEye.ReplayLastReveal [LocalPlayerIndex]` | Re-run the most recent successful component for this controller. |
| `Berta.BlackEye.Trace 1` / `Berta.BlackEye.Trace 0` | Enable/disable an off-by-default console **variable**. |

Trace lines in Output Log carry owner, session ID, monotonic real-time delta, start decisions, transitions, return/fallback, external takeover, and cleanup. Replay scans current-world components on demand and uses the selected component's **current** configuration, not recorded camera state. It bypasses trigger `bEnabled`/`bTriggerOnce` without changing their state, rejects an active last component, and repeats Manual mode until explicitly stopped. Destroyed components and previous worlds are not retained. Console commands, Trace, and Replay metadata are omitted in Shipping.

## Editor tools and Black Eye integration

**Level Editor → Tools → Berta Black Eye Camera** contains **Create Berta Reveal Trigger** (one selected Black Eye camera, spawn at camera, assign it, select trigger), **Create Berta Reveal Trigger Around Actor** (one camera plus one other Actor, use that Actor's bounds with padding), and **Audit Black Eye Camera Setup** (read-only Output Log scan). Creation uses Editor transactions for Undo/Redo and does not save the level. Audit Errors cover missing configured reveal camera, invalid reveal/switcher timing, missing explicit return target, and null/duplicate switcher entries. The Orbit input-binding caveat is a Warning; camera class, ControlRotation, auto-activation, policies, trigger setup, and counts are Info. It does not prove runtime ownership or fix assets.

Selecting an Actor with a reveal component displays Editor-only relationships to TargetCamera and distinct valid participants plus a concise status HUD; triggers add their EndMode/enabled/once state. A null target appears as None without a line. Deselecting removes the visualization.

Use Black Eye's public **Set Follow Actor Override** and **Set Single Look At Target Actor Override** for simple targets; Berta adds no target wrapper. `ABlackEyeShotListCineCameraActor` derives from the accepted TargetCamera base: Black Eye owns shots, Berta owns entry/return. Give a Timed reveal enough HoldTime for intended shots or Stop a Manual reveal from gameplay. The inspected Orbit camera binds Enhanced Input look actions while becoming ViewTarget; residual behavior after return needs manual verification in a target project. No Black Eye source is patched or included here.

The plugin is local-player oriented, does not replicate reveals, monitor all camera changes, replace any manager/priority stack/Shot List, or supply camera scoring, procedural algorithms, GAS/AI integration, or a global cinematic coordinator. A custom camera manager's behavior must be checked against its actual ViewTarget implementation.

For fuller Blueprint/C++ contracts and failure cases, see the plugin [README](https://github.com/nbertoa/ue5-bertadevkit/blob/main/BertaDevKitHost/Plugins/BertaBlackEyeCameraExt/README.md).
