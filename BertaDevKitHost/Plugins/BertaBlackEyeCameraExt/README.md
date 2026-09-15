# BertaBlackEyeCameraExt

Optional UE 5.8 gameplay, prototyping, and diagnostic utilities around a locally installed Black Eye Camera 2.0 plugin. Black Eye owns camera behavior, Follow, LookAt, Orbit, Shot Lists, and any priority stack. BertaBlackEyeCameraExt uses a local `PlayerController` and its `PlayerCameraManager` to request standard `SetViewTargetWithBlend` transitions; the Black Eye camera manager is not required. The plugin is disabled by default and is not replicated.

## Reveal

`UBertaBlackEyeCameraRevealComponent` can be added to any Actor Blueprint. Set `TargetCamera` to a Black Eye cine camera, then call `StartCameraReveal` from a boss-health event, puzzle, interactable, Level Blueprint, or script. `StopCameraReveal` ends a manual reveal; `ResetCameraReveal` requests a controlled exit. C++ can start for a specific local controller with `StartCameraRevealForController`. The component owns the camera session, timers, input locks, ControlRotation restoration, participant notifications, hooks, and delegates. A timed session blends in, holds for `HoldTime`, then blends back. A manual session remains active until stopped.

`ABertaBlackEyeCameraTrigger` contains a Box Collision and a reveal component. Configure the camera, timing, input, preset, and participants on its `RevealComponent`. The actor controls `bEnabled`, `bTriggerOnce`, `CanActorTrigger`, and `EndMode`:

| EndMode | Behavior |
| --- | --- |
| `Timed` | Valid overlap or manual start → blend in → HoldTime → blend out. |
| `OnEndOverlap` | Valid overlap → blend in → remain active → matching actor leaves the box → blend out. An unrelated EndOverlap does nothing. |
| `Manual` | Overlaps do not start a reveal. Call `StartCameraReveal`, then `StopCameraReveal`. |

The trigger retains its Blueprint methods, hooks, and four delegates as a façade. The original v1 actor fields for `TargetCamera`, blend times, blend functions, and input switches moved to `RevealComponent`. Existing Blueprint assets that assigned those fields must reassign them on the component; this avoids two editable configurations that could diverge. `bTriggerOnce` and `bEnabled` remain on the actor. The actor's `bEnabled` controls its wrapper/overlap activation; code can still call the component directly.

`FBertaBlackEyeRevealSettings` groups transition feel and input policy. `UBertaBlackEyeRevealPreset` stores only those settings, never a camera. If `RevealPreset` is set, a copy of its settings wins; otherwise `InlineSettings` is used. The copy is taken at session start, so editing a preset during an active reveal changes only future sessions.

The component captures the previous `ViewTarget` as a weak reference and captures `ControlRotation` before blending in. It restores ControlRotation before the return blend and after its duration, before releasing its Look lock. Move and Look locks use matching `SetIgnoreMoveInput` and `SetIgnoreLookInput` calls. Full input uses a temporary high-priority, binding-free `UInputComponent` blocker and removes its own component on cleanup. These locks do not reset input state contributed by other systems. A destroyed previous target falls back to the captured controller's pawn or controller. Another camera system can change the ViewTarget concurrently; this utility does not arbitrate global camera ownership.

## Participants

Actors implementing `IBertaCinematicParticipant` can receive `OnCinematicPause(Source)` and `OnCinematicResume(Source)`. `Source` is the reveal component that owns the request. Duplicate configured entries are notified once, and only participants actually paused by that session are resumed. Implementations decide how to combine multiple sources; this plugin has no AI, GAS, or global pause policy.

## Camera switcher and diagnostics

Add `UBertaBlackEyeCameraSwitcherComponent` to a local `PlayerController` for quick camera comparison. Configure a nonempty, unique list of valid Black Eye cameras. `SelectCamera(Index)`, `SelectNextCamera`, and `SelectPreviousCamera` use the controller's current ViewTarget to determine the position. When it is outside the list, Next selects the first and Previous the last. Selection does not restore an earlier camera or maintain a stack.

In non-Shipping builds, `UBertaBlackEyeCameraDebugLibrary::GetBlackEyeCameraDebugSummary` reports the local controller, camera manager, current target, ControlRotation, and any active reveal components for that controller. The following commands use the console's game world; `LocalPlayerIndex` is optional with one local player and required with multiple:

```text
Berta.BlackEye.Dump [LocalPlayerIndex]
Berta.BlackEye.Next [LocalPlayerIndex]
Berta.BlackEye.Previous [LocalPlayerIndex]
Berta.BlackEye.Select <CameraIndex> [LocalPlayerIndex]
```

Next, Previous, and Select require a switcher on the selected `PlayerController`. Commands reject invalid arguments or ambiguous players. Diagnostics scan the world only when requested.

## Editor tools

With this plugin enabled in the Editor, the Level Editor Tools menu offers:

- **Create Berta Reveal Trigger:** select exactly one Black Eye camera. The new trigger is placed at its location, assigned that camera, and selected. Undo/Redo uses an Editor transaction.
- **Create Berta Reveal Trigger Around Actor:** select exactly one Black Eye camera and one other actor. The trigger box uses that actor's bounds plus simple padding.
- **Audit Black Eye Camera Setup:** read-only Output Log checks for missing configured targets, invalid timing, Orbit input caveats, and null/duplicate switcher entries. It reports normal ControlRotation and auto-activation settings as information.

These actions do not save levels or change source control.

## Black Eye APIs that remain direct

For a quick “follow player, look at boss” prototype, use Black Eye's Blueprint methods `Set Follow Actor Override` on its Follow component and `Set Single Look At Target Actor Override` on LookAt, both at index 0. A Berta target wrapper would duplicate those direct calls.

For multiple shots, assign a Black Eye Shot List camera as the reveal's `TargetCamera`. Its `BecomeViewTarget` initializes the list and its `EndViewTarget` ends it; the reveal component only handles the eventual return to gameplay. No Berta shot sequence is needed. Set timed Hold long enough for the intended shots, or use a manual reveal and stop it from gameplay logic.

Black Eye Orbit binds Enhanced Input look actions when it becomes a ViewTarget. Its current implementation does not override `EndViewTarget` to remove those bindings; verify residual Orbit input manually in the target project. BertaBlackEyeCameraExt does not patch Black Eye source.
