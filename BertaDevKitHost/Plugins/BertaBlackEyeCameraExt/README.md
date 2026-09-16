# BertaBlackEyeCameraExt

An optional Unreal Engine 5.8 Runtime/Editor extension for prototyping temporary reveals with Black Eye cameras. It coordinates entering and leaving a camera, input, generic participants, diagnostics, and Editor setup. Black Eye remains responsible for camera behavior, Follow, LookAt, Orbit, Shot Lists, procedural composition, and its optional camera manager/priority stack.

## Requirements and installation

Install a legitimate Black Eye Camera 2.0 copy separately in the target UE 5.8 project. Copy `BertaDevKitHost/Plugins/BertaBlackEyeCameraExt/` to `<YourProject>/Plugins/BertaBlackEyeCameraExt/`, enable **Black Eye** (`Black_Eye`) and **BertaBlackEyeCameraExt**, then build the C++ project. The extension is disabled by default and declares `Black_Eye` as a plugin and Runtime module dependency. This repository does not distribute Black Eye source, assets, or binaries. The locally inspected API is Black Eye `2.0.7` for UE 5.8; other Black Eye versions have not been verified.

Reveals and the switcher use the existing local `PlayerController`, its `PlayerCameraManager`, and UE `SetViewTargetWithBlend`. They do not require `ABlackEyeCameraManager`. A custom manager must still honor the standard ViewTarget contract; this plugin cannot guarantee the behavior of every external camera system. The plugin is local-player oriented and does not replicate reveal sessions.

## Quick start

**Triggered reveal:** place a Black Eye cine camera and `ABertaBlackEyeCameraTrigger`. On the trigger's `RevealComponent`, assign `TargetCamera`, keep `EndMode = Timed`, and set BlendIn/Hold/BlendOut times. Enter its Box with the Pawn controlled by a local player. The trigger requests the reveal, holds, then returns.

**Scripted reveal:** add `UBertaBlackEyeCameraRevealComponent` to an Actor Blueprint, assign `TargetCamera`, and call `StartCameraReveal` from a gameplay event—for example, a boss reaching 50% health. Choose `DurationMode = Timed` for an automatic exit, or `Manual` and call `StopCameraReveal` when gameplay is ready. C++ can use `StartCameraRevealForController(LocalController, Mode)` for a particular local controller.

## Reveal component

`TargetCamera` accepts `ABlackEyeCineCameraActorBase` and its derived Black Eye cine cameras in the same world. Start rejects a missing/destroyed/cross-world target, unusable local controller or camera manager, busy/ending component, and negative or non-finite timing/exponents. The Blueprint `StartCameraReveal()` returns whether a session was accepted. It searches for the first usable local controller in its world; use the C++ controller-specific entry point when that choice must be explicit.

An accepted session validates and captures the current or pending `ViewTarget`, current Pawn, and `ControlRotation`; applies its input contributions; notifies configured participants and gameplay hooks; requests BlendIn; fires CameraReached after the requested blend-in duration; enters Hold or Manual Active; then exits through return or external-camera cleanup. `GetRevealState` exposes `Idle`, `BlendingIn`, `Holding` (Timed), `Active` (Manual), and `BlendingOut`. `IsRevealActive` remains true until cleanup finishes. `StopCameraReveal` requests a controlled exit from BlendIn, Hold, or Active. `ResetCameraReveal` is the same request, not a hard cancel or a reset of a trigger's one-shot state.

### Settings and presets

`InlineSettings` configures the reveal when there is no valid `RevealPreset`. A preset is a `UBertaBlackEyeRevealPreset` Data Asset containing only `FBertaBlackEyeRevealSettings`: it shares transition feel and input policy, never a camera or trigger. Effective settings are copied when a session starts, so edits to a preset during a reveal affect later sessions.

`BlendInTime`, `HoldTime`, and `BlendOutTime` are requested durations; zero is valid. `HoldTime` applies only to Timed sessions. `BlendInFunction`/`BlendOutFunction`, their `BlendInExponent`/`BlendOutExponent`, and `bLockOutgoingOnBlendIn`/`bLockOutgoingOnBlendOut` are passed to `SetViewTargetWithBlend` for the corresponding transition. The active `PlayerCameraManager` determines the actual blend and outgoing-POV-lock behavior. Runtime rejects negative or non-finite times and exponents.

`bDisableMoveInput` and `bDisableLookInput` add `SetIgnoreMoveInput(true)` / `SetIgnoreLookInput(true)` contributions and remove those same contributions on cleanup. `bDisablePlayerInput` pushes a temporary high-priority, binding-free `UInputComponent` blocker and later pops/destroys that component. These are owned by the reveal: cleanup does not blindly call `ResetIgnoreMoveInput`, `ResetIgnoreLookInput`, or `EnableInput`, so other systems' contributions are not deliberately reset. A controller destroyed during a reveal cannot receive restoration calls; the component still removes its own transient state where possible.

### Return and external-camera policies

`ReturnTargetPolicy` is per component and snapshotted at session start:

| Policy | Choice at exit | Fallback |
| --- | --- | --- |
| `CapturedViewTarget` (default) | ViewTarget captured before BlendIn | Captured Pawn, then controller |
| `CurrentPawn` | Controller's Pawn **at exit**, including a new possession | Captured ViewTarget, then controller |
| `ExplicitTarget` | `ExplicitReturnTarget`, which may be any valid Actor | Captured ViewTarget, current Pawn, then controller |

The explicit actor is weakly referenced by the active session. A missing or destroyed explicit target produces a warning and uses the fallback. A normal return restores the captured `ControlRotation` before asking for BlendOut and again after its requested duration, before removing the Look lock. This matters because a Black Eye camera can modify controller rotation while it is the ViewTarget.

`ExternalCameraChangePolicy` is also snapshotted. `RestoreConfiguredTarget` (default) performs the configured return even after another system changes the ViewTarget. `RespectExternalChange` checks the current/pending ViewTarget just before Berta requests its own return. If another system has taken over, Berta leaves that ViewTarget and its `ControlRotation` intact, skips BlendOut waiting, and immediately removes its own input locks, resumes participants, and removes gameplay locks. For example: reveal active → another system selects a camera → Stop/Timed exit → external camera remains. This check occurs at exit or safe `EndPlay` cleanup boundaries, without Tick or global camera arbitration. A Manual reveal can remain logically active after takeover until Stop is called. Loss of `TargetCamera` follows its separate cleanup path rather than automatically counting as external takeover.

### Events, hooks, and participants

The component's four Blueprint delegates mark: `OnRevealStarted` after acceptance/hooks and before BlendIn request; `OnRevealCameraReached` when the requested BlendIn **time** has elapsed, without measuring visual convergence; `OnRevealEnding` before the return/ownership decision; and `OnRevealFinished` after owned cleanup. `EndPlay` cleanup does not broadcast a normal Finished event.

BlueprintNativeEvent hooks `ApplyGameplayLock`/`RemoveGameplayLock` and `OnCinematicStarted`/`OnCinematicEnding`/`OnCinematicFinished` let a child add project-owned gameplay behavior. Generic C++ input locking is independent of Blueprint parent calls. A project could block boss abilities in `ApplyGameplayLock` and undo that contribution in `RemoveGameplayLock`; the extension itself has no GAS or AI dependency.

Enable `bNotifyParticipants` and list Actors implementing `IBertaCinematicParticipant`. A valid participant receives `OnCinematicPause(Source)` and later `OnCinematicResume(Source)`, where `Source` is the reveal component. Duplicate configured Actors are paused once; only Actors actually notified by that session are considered for resume, and destroyed Actors are skipped. The participant implementation owns the meaning of pause and how multiple Sources combine—for example, tracking outstanding Sources before resuming. A project's boss might stop movement/brain logic on Pause and resume it on Resume; those are project choices, not plugin behavior.

## Reveal trigger

`ABertaBlackEyeCameraTrigger` owns Box activation and forwards the component's four delegates and gameplay hooks. Configure the camera, preset, input, participants, and return policies **on `RevealComponent`**. The trigger's `EBertaBlackEyeRevealEndMode` differs from the component's `EBertaBlackEyeRevealDurationMode`:

| Trigger `EndMode` | Behavior | Component session mode |
| --- | --- | --- |
| `Timed` | Valid overlap or explicit Start; exit after HoldTime | Timed |
| `OnEndOverlap` | Valid overlap; exit when that same overlap owner leaves or is destroyed | Manual |
| `Manual` | Overlap never starts; explicit trigger Start/Stop | Manual |

Default `CanActorTrigger` accepts a Pawn controlled by a local `PlayerController`; override it to filter overlap actors. An unrelated EndOverlap does not stop the session. `bEnabled` gates overlaps and the trigger's explicit Start; `SetTriggerEnabled(false)` also requests Stop if a reveal is active. `bTriggerOnce` consumes a successful start through the trigger wrapper. `StopCameraReveal` stops but does not clear that use. `ResetCameraTrigger` clears it immediately while idle, or requests a controlled exit and clears it after Finish. Calling the component's own `ResetCameraReveal` does not reset the trigger. Direct component calls, including development Replay, bypass the trigger's enabled/one-shot gate; they still forward lifecycle delegates while the trigger exists.

## Camera switcher

Attach `UBertaBlackEyeCameraSwitcherComponent` to a local `PlayerController` for prototype camera comparison. Configure an ordered, nonempty list of unique, valid, same-world Black Eye cine cameras. `IsCameraSelectionValid` rejects empty/null/duplicate/cross-world entries and negative/non-finite `BlendTime` or `BlendExponent`; selection also requires a usable local controller/camera manager. `BlendTime`, `BlendFunction`, `BlendExponent`, and `bLockOutgoing` go to `SetViewTargetWithBlend`.

`SelectCamera(Index)` returns false for invalid setup/index/controller; selecting the already-current camera returns true without requesting another blend. `SelectNextCamera`/`SelectPreviousCamera` wrap the list. If the current/pending ViewTarget is outside it, Next chooses the first and Previous the last. `GetSelectedCameraIndex` returns `INDEX_NONE` if there is no valid configured selection; `GetSelectedCamera` then returns null. The switcher owns no previous-target stack, automatic return, or Black Eye priority system.

## Runtime diagnostics

`UBertaBlackEyeCameraDebugLibrary::GetBlackEyeCameraDebugSummary(LocalController, OutSummary)` is an on-demand Blueprint/C++ query. In non-Shipping builds it reports the controller/manager, current ViewTarget and class, Black Eye status, ControlRotation, active reveal owners/states, captured/target actors, effective return/external policies, timing and configured input flags, phase time, multiple-active conflict, and the last replayable component. It returns false with explanatory text for an unusable local controller/manager/world. In Shipping it returns false and says diagnostics are unavailable. The summary observes source-visible state; it does not prove visual convergence or ownership inside a custom manager.

All commands below are non-Shipping and require a game world with a usable local player. `LocalPlayerIndex` may be omitted with one local player; with multiple, it is required. Invalid/ambiguous indexes are logged rather than selecting another player.

| Syntax | Purpose and required setup |
| --- | --- |
| `Berta.BlackEye.Dump [LocalPlayerIndex]` | Print the debug summary; no switcher required. |
| `Berta.BlackEye.Next [LocalPlayerIndex]` | Select the next configured camera; requires a switcher on the controller. |
| `Berta.BlackEye.Previous [LocalPlayerIndex]` | Select the previous configured camera; requires a switcher on the controller. |
| `Berta.BlackEye.Select <CameraIndex> [LocalPlayerIndex]` | Select one switcher list index; requires a switcher and valid index. |
| `Berta.BlackEye.ReplayLastReveal [LocalPlayerIndex]` | Start the latest successful reveal component for that controller in the current world. |

Commands log argument/setup/selection failures through `LogBertaBlackEyeConsole`; reveal and switcher warnings use `LogBertaBlackEyeReveal` and `LogBertaBlackEyeSwitcher`.

`Berta.BlackEye.Trace` is a **console variable**, not a command callback. Set `Berta.BlackEye.Trace 1` to emit lifecycle lines in Output Log (`LogBertaBlackEyeReveal`); set `Berta.BlackEye.Trace 0` to stop. It defaults to zero. Lines include owner, incremental session ID, and elapsed monotonic real time, so timing remains interpretable across pause/time dilation. They cover start acceptance/rejection, camera/participant/input/gameplay phases, return resolution/fallback, external takeover, losses, and cleanup. A fragment looks like `[Reveal:BP_Boss:3 +1.802] Ending entered`.

Replay searches existing reveal components in that world on demand and selects the highest last-successful-start time associated with the chosen controller. It invokes that component directly using its **current** TargetCamera, preset/settings, policies, and participants—not a recording of old state. The last component being active rejects replay without interruption; invalid target or rejected Start logs a failure. Destroyed components and prior worlds are not retained. A replay of a Manual component session starts Manual again and needs an explicit `StopCameraReveal` on that component. A consumed trigger's `bTriggerOnce`/`bEnabled` state is neither consulted nor reset.

## Editor tools

With the plugin enabled in Editor, **Level Editor → Tools → Berta Black Eye Camera** offers:

- **Create Berta Reveal Trigger:** select exactly one Black Eye cine camera. It spawns an undoable trigger at that camera's location, assigns `RevealComponent.TargetCamera`, and selects the trigger. It does not resize the Box around another Actor.
- **Create Berta Reveal Trigger Around Actor:** select exactly one Black Eye cine camera and one other Actor. It spawns at the other Actor's bounds center, sizes the Box from those bounds with simple padding, assigns the camera, and selects the undoable trigger. It does not configure gameplay targets or save the level.
- **Audit Black Eye Camera Setup:** read-only Output Log scan of the open Editor world. Errors cover configured reveals without valid TargetCamera, negative/non-finite reveal timing/exponents, `ExplicitTarget` without valid `ExplicitReturnTarget`, null/duplicate switcher entries, and negative/non-finite switcher blend time/exponent. Orbit input binding is a Warning to verify manually. Info reports configured return/external policies, camera class, enabled Black Eye ControlRotation/auto-activation, trigger mode/enabled/once/preset status, and counts. It does not verify runtime camera ownership, participant behavior, switcher empty/cross-world setup, or every map override; it does not change actors/assets or patch Black Eye.
- **Reveal relationship visualizer:** when an Actor/component with a registered Reveal Component is selected, the Editor viewport shows a primary owner→TargetCamera line and secondary lines to distinct valid participants. A short HUD shows target (or None), preset/inline, policies, and valid participant count; triggers also show EndMode/enabled/once. An arbitrary Actor with the component gets the same reveal relations without trigger-only fields. Deselecting removes the visualization. It does not mutate the level or draw every reveal globally.

Editor setup/audit messages use `LogBertaBlackEyeEditor`. Creating triggers uses a transaction for Undo/Redo; the Audit and visualizer are read-only.

## Black Eye integration and caveats

For “follow player, look at boss,” use Black Eye's public Blueprint methods **Set Follow Actor Override** on its Follow component and **Set Single Look At Target Actor Override** on LookAt (index 0 for a single target). The reveal selects the Black Eye camera as a whole; it does not own Follow/LookAt targets or need a Berta target wrapper.

`ABlackEyeShotListCineCameraActor` derives from the accepted target-camera base. Assign it directly as `TargetCamera`: Black Eye owns its internal shots/transitions while Berta owns entering/leaving the whole reveal. In Timed mode choose a HoldTime long enough for the intended shots; in Manual mode gameplay decides when to Stop. No Berta Shot List is involved.

The installed Black Eye Orbit camera binds Enhanced Input look actions on becoming ViewTarget. Its current source does not provide an actor `EndViewTarget` override that removes those bindings; **residual input behavior has not been reproduced here** and should be checked manually in the target project. This extension does not patch Black Eye. UE's `GetViewTarget` can name a pending destination during an existing blend; that destination is what the reveal captures or compares. A captured/explicit return Actor may be destroyed and trigger the documented fallback; loss of TargetCamera requests cleanup. `RespectExternalChange` detects only a ViewTarget difference at exit, not every internal decision made by an external camera manager.

## Build availability and non-goals

| Capability | Development/Editor | Shipping |
| --- | --- | --- |
| Reveal, trigger, presets, participants, switcher | Runtime | Runtime |
| `GetBlackEyeCameraDebugSummary` | Diagnostic text | Returns false/unavailable text |
| Dump/Next/Previous/Select/Replay commands and Trace variable | Non-Shipping only | Omitted |
| Quick Create, Audit, relationship visualizer | Editor module only | Omitted |

BertaBlackEyeCameraExt does not replace a camera manager, Black Eye priority stack, Shot List, scoring/collision/procedural camera algorithms, or supply GAS/AI policy, a global cinematic coordinator, or replication. Its scope is one local reveal session and small development/editor helpers.
