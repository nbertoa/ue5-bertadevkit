# BertaDevKit

BertaDevKit is a general-purpose Unreal Engine 5.8 toolbox. Its Runtime module provides reusable Blueprint-facing utilities, while its Editor module provides conservative tooling for development workflows.

## Runtime utilities

`UBertaWorldUtils::SetDelayedAction` returns `Success` and a new timer handle. Failure leaves `OutHandle` invalid. Reusing an output variable does not cancel a previously scheduled timer; retain its handle separately to cancel it with `CancelDelayedAction`.

| System | Purpose |
| --- | --- |
| `UBertaDebugUtils` | Blueprint-friendly screen and Output Log messages with context, verbosity, categories, and per-call gating. |
| `UBertaDebugDraw` | Development debug drawing for common primitives, components, strings, coordinate systems, and persistent-shape flushing. |
| `UBertaScreenStats` | Named development screen stats for common value types; updating a name replaces its displayed value. |
| `UBertaMathUtils` | Remapping, easing, angular helpers, snapping, distributions, and lightweight prediction helpers. |
| `UBertaWorldUtils` | Actor queries, traces, player/camera access, and delayed-action timer helpers. |
| `UBertaAudioUtils` | Repeated local sound playback in 2D, at a captured location, or attached to a component, with cancellable per-sequence handles. |
| `UBertaUIUtils` | Blueprint conveniences for common UI/player-input boilerplate. |
| `UBertaVideoPlayerWidget` | Self-contained fullscreen Media Framework playback with optional UI audio and gameplay pause ownership. |
| `UBertaImagePlayerWidget` | Self-contained fullscreen Texture2D display with optional linear image-opacity fades. |
| `UBertaFadeWidget` | Reusable fullscreen black fade driven by real UI elapsed time. |
| `UBertaControllerUtils` | Controller feedback, light output, and Input Device Property conveniences without PlayerController casts. |
| `UBertaBTDecorator_GameplayTag` / `UBertaBTDecorator_GameplayTagQuery` | Reactive GAS conditions controlling whether Behavior Tree branches may execute. |
| `UBertaBTTask_WaitGameplayTagQuery` | Event-driven Behavior Tree wait for a Gameplay Tag Query state without Blackboard mirroring. |
| `UBertaBTTask_ActivateGameplayAbilityAndWait` | Activates one granted Gameplay Ability and waits for that exact execution to end. |
| `UBertaBTTask_WaitGameplayEvent` | Event-driven wait for the next exact or hierarchical GAS Gameplay Event. |
| `UBertaBTDecorator_AttributeThreshold` | Reactive numeric comparison against one controlled-Pawn GAS attribute. |
| `UBertaBTTask_WaitAttributeThreshold` / `UBertaBTTask_WaitTargetAttributeThreshold` | Event-driven waits until a self or Blackboard-target GAS attribute satisfies the shared numeric condition. |
| `UBertaBTTask_WaitAbilityEnd` | Waits until no active execution remains for an exact granted ability class. |
| `UBertaBTDecorator_AbilityActive` | Reactive condition for active executions of one exact granted ability class. |
| `UBertaBTTask_CancelGameplayAbility` | Immediate cancellation request for one exact granted ability spec. |
| `UBertaBTTask_SendGameplayEvent` | Sends a compact GAS Gameplay Event payload to the controlled Pawn. |
| `UBertaBTTask_SendGameplayEventToTarget` | Sends a compact GAS Gameplay Event payload to a Blackboard target Actor. |
| `UBertaBTTask_ApplyGameplayEffectToSelf` | Applies an instant, duration, or infinite Gameplay Effect to the controlled Pawn. |
| `UBertaBTTask_ApplyGameplayEffectToTarget` | Applies an outgoing Gameplay Effect from the controlled Pawn ASC to a Blackboard target ASC. |
| `UBertaBTTask_RemoveGameplayEffectsFromSelf` / `UBertaBTTask_RemoveGameplayEffectsFromTarget` | Remove active Gameplay Effects matching a non-empty query. |
| `UBertaBTDecorator_TargetGameplayTagQuery` | Reactive Gameplay Tag Query on an Actor selected from Blackboard. |
| `UBertaBTDecorator_TargetAttributeThreshold` | Reactive attribute comparison on an Actor selected from Blackboard. |
| `UBertaBTTask_WaitTargetGameplayTagQuery` | Waits across Blackboard target replacement for a target tag-query state. |
| `UBertaBTTask_ActivateGameplayAbilityWithTarget` | Triggers one granted ability with a Blackboard Actor in its Gameplay Event context. |
| `UBertaBTTask_WaitGameplayEffectApplied` | Waits for the next matching Gameplay Effect application, including instant effects. |
| `UBertaBTTask_WaitGameplayEffectRemoved` | Waits until no active Gameplay Effect matching a query remains. |
| `UBertaBTDecorator_GameplayEffectQuery` | Reactive condition for active Gameplay Effects matching a query. |
| `UBertaBTDecorator_CanActivateAbility` | Side-effect-free check of an exact granted ability's current activation rules. |
| `UBertaBTTask_WaitAbilityReady` | Bounded periodic wait for arbitrary Gameplay Ability readiness logic. |
| `UBertaGASDebugUtils` | Deterministic text snapshot of an Actor's current GAS state. |
| `UBertaGASAbilityUtils` | Read-only cooldown, cost, and activation inspection for granted Gameplay Abilities. |
| `UBertaGameplayTagDebugUtils` | Sorted tag/query summaries, exact container diffs, and Actor tag-source inspection. |
| `UBertaBlackboardDebugUtils` | Deterministic Blackboard snapshot using UE's native key-value descriptions. |
| `UBertaBehaviorTreeDebugUtils` | Public Runtime Behavior Tree execution snapshot, including active node/path descriptions. |
| `UBertaAIDebugUtils` | Combined AI, Behavior Tree, Blackboard, and GAS snapshot. |
| `UBertaBTTask_DebugGASState` | Logs the controlled Pawn's GAS snapshot at a Behavior Tree execution point. |
| `UBertaBTTask_DebugTargetGASState` | Logs a Blackboard target Actor's GAS snapshot at a Behavior Tree execution point. |
| `UBertaBTTask_DebugBlackboardState` / `UBertaBTTask_DebugAIState` | One-shot Blackboard or combined AI snapshot tasks. |
| `UBertaBTService_TraceBlackboardChanges` | Event-driven trace of selected or all Blackboard keys while a branch is relevant. |
| `UBertaBTService_TraceGameplayTags` | Event-driven count trace for explicitly selected controlled-Pawn GAS tags. |
| `UBertaBTTask_AssertGameplayTagQuery` / `UBertaBTTask_AssertAttributeThreshold` / `UBertaBTTask_AssertAbilityActive` | Immediate non-crashing GAS assertions for Behavior Tree R&D. |

Debug-facing Blueprint nodes use Unreal's `DevelopmentOnly` metadata where appropriate. This signals intended development use; it is not a blanket claim about all Runtime code or runtime cost.

`UBertaControllerUtils` plays native dynamic vibration either uniformly or per motor, returning a handle for stopping only its own actions. It also delegates controller light color/reset and asset-based `ForceFeedbackEffect` play/stop to `APlayerController`, preserving Unreal's native client-RPC behavior for the effect calls. Input Device Property activation uses the resolved controller's Platform User and lets Unreal select that user's default input device; a controller does not identify one unique physical device. Properties can be queried or removed by handle. **Remove All Input Device Properties (Global)** removes active properties for every local Platform User, so use it only when global cleanup is intended.

## Repeated audio

`UBertaAudioUtils` exposes **Play Repeated Sound 2D**, **Play Repeated Sound at Location**, and **Play Repeated Sound Attached**. All accept a `USoundBase` asset and `FBertaRepeatedSoundOptions`, with hidden World Context. Concurrency is advanced on all three nodes; attenuation is advanced only on the spatial nodes.

A typical Blueprint call needs no Delay, loop, counter, or custom timer:

```text
Play Repeated Sound 2D
    Sound = S_Click
    Options:
        Repeat Count = 3
        Interval = 0.5
        Interval Variance = 0.1
        Play Immediately = true
```

| Option | Default | Meaning |
| --- | --- | --- |
| Repeat Count | 1 | Total scheduled playback attempts, including rejected occurrences; must be positive. |
| Interval / Interval Variance | 0.5 / 0 seconds | Each gap between attempts independently samples Interval ± Variance and clamps to non-negative. |
| Play Immediately | true | Attempts the first playback synchronously; false waits one independently sampled gap first. |
| Volume Multiplier / Volume Variance | 1 / 0 | Each attempt independently samples target volume ± variance, clamped to non-negative. |
| Pitch Multiplier / Pitch Variance | 1 / 0 | Independently sampled per attempt, then clamped using the audio device's configured native pitch range. |
| Use Fade In / Fade In Duration | false / 0.2 seconds | Positive enabled duration starts inaudibly and uses a native linear fade to sampled volume. |
| Playback Duration | 0 seconds | Positive values force a per-playback deadline; <= 0 allows natural completion or authored looping. |
| Use Fade Out / Fade Out Duration | false / 0.2 seconds | With a positive Playback Duration, fades to zero by its deadline. |

Repeat Count 3, Interval 0.5, Variance 0, Play Immediately true schedules approximate attempts at 0, 0.5, and 1.0 seconds. Each attempt consumes one repetition even if native component creation fails or concurrency rejects the voice. There is no retry, and an individual rejection does not cancel the remaining attempts. Intervals are measured **between attempts**, allowing overlapping playback; they never wait for the previous sound to end. Interval 0.5 with Variance 0.1 independently generates each gap in 0.4–0.6 seconds. Attempts follow actual timer dispatch, so frame delays can shift them; there is no catch-up burst. A zero sampled gap uses a minimum positive one-shot timer and advances at most once per subsequent TimerManager tick, without recursive immediate calls.

Every public float must be finite. Intervals, variances, multipliers, and fade durations must be non-negative; invalid inputs return null with a `LogBertaDevKit` warning. Extremely large volume/interval samples saturate at the largest finite float rather than overflowing. Pitch uses UE's audio device clamp (normally 0.4–2.0, configurable through native audio settings). Native sound asset variation, attenuation, concurrency, virtualization, and audio-device availability still apply: a scheduled occurrence is an attempt, not a guarantee that a sound will be audible.

Components are created **before** playback, then started with native `FadeIn` or `Play`. There is no full-volume playback before a positive fade-in. Zero fade-in duration starts at target volume immediately. A positive Playback Duration schedules a hard stop at the deadline. Enabled fade-out uses `EffectiveFadeOutDuration = min(FadeOutDuration, PlaybackDuration)` and begins at `FadeStartTime = PlaybackDuration - EffectiveFadeOutDuration`. For lifetime 2 / fade 0.5, it begins at 1.5; for lifetime 1 / fade 2, the effective fade is 1 second and begins immediately. EditCondition metadata and Blueprint tooltips communicate that automatic fade-out requires positive Playback Duration. If fade-out covers the entire lifetime, it starts immediately and takes precedence over fade-in; combined with an initial zero fade-in this can leave the sound inaudible. Zero fade-out duration stops at the deadline. Without a positive Playback Duration there is no automatic fade-out deadline, and asset duration is never inferred. Natural early completion cancels that component's remaining timers.

- **2D:** non-spatialized UI sound, with no attenuation pin.
- **At Location:** captures Location/Rotation once; subsequent starts reuse that world transform.
- **Attached:** each playback attaches with native relative component/socket Location/Rotation. It follows the target and stops when its owner is destroyed. Losing the component cancels the session, including active sounds; future starts validate the target and a game-time cleanup sweep detects invalid targets within approximately 0.1 seconds.

The return value is an opaque `UBertaRepeatedSoundHandle`. The GameInstance retains each session, so Blueprint may ignore the return value and the complete sequence still runs. Retain it only when cancellation or activity inspection is needed. Audio components are tracked weakly; the native audio/world system owns their lifetime. Native play-state callbacks cover natural completion, explicit stops, and backend failures; the sweep also cleans up destroyed components. The session remains bound to its originally resolved World and never migrates to a later World, even when both share a GameInstance/TimerManager. It observes `OnWorldBeginTearDown` and `OnWorldCleanup` directly; the cleanup signal also covers `DestroyWorld` paths without a prior teardown signal. Teardown establishes terminal state, cancels every session/per-instance timer, removes audio and lifecycle delegates, stops sounds immediately without a teardown fade, clears tracked references, and releases registration. Finalization is idempotent; converging callbacks cannot restart the session or act on components it no longer tracks, and `SetReadyToDestroy` is called once.

**Stop Repeated Sound** cancels future starts and replaces individual forced-lifetime schedules. Its **Fade Out Active Sounds Duration** defaults to 0, stopping immediately; positive values use native linear fades and release the session when they finish. Repeated Stop calls and null/finished handles are harmless. **Is Repeated Sound Active** is true while future starts or tracked active/fading sounds remain, including a Stop fade; it becomes false when the session has finished.

Scheduling, deadline timers, and cleanup use normal **world/game time**, following pause and time dilation. Fades use UE's native audio fader, with no custom clock or pause policy; native 2D UI playback may continue while the world is paused. The feature is local and **does not replicate**. It intentionally does not provide music/playlist management, asset selection, global volume control, async loading, persistence, or unlimited repetition. Authored looping sounds with no positive Playback Duration remain active until stopped.

## Localization helpers

`UBertaLocalizationUtils` adds three pure nodes under **BertaDevKit | Localization**:

- **Is Current Language** compares UE-resolved culture names exactly. `en-US` matches `EN_us`, but not `en`.
- **Is Current Language Compatible With** accepts an exact match or a culture in UE's prioritized fallback chain for the current language. `es-AR` matches `es`, but not `es-ES`; `es` does not match `es-AR`. Native culture remapping, script inference, and allowed-culture policy apply to the fallback chain.
- **Canonicalize Culture Name** uses `FCulture::GetCanonicalName`; `en_US` becomes `en-US`. It normalizes rather than strictly validates or checks culture availability. Empty input returns empty; on UE 5.8 with ICU, malformed input such as `!` or whitespace returns `en-US-POSIX`.

Both predicates require a native-resolvable name whose canonical form differs only in casing or `-`/`_` separators. Empty, whitespace, sanitized/malformed names, and aliases requiring substitution return false, even when the current language is invariant. An explicit `en-US-POSIX` name remains valid. These stricter predicate rules are separate from the permissive canonicalization node.

Language selects localized text; locale controls regional formatting; asset-group cultures are independent. Use Unreal's native **Get Default Language** / **Get Default Locale** nodes (`UKismetSystemLibrary`) for platform defaults. Continue using native Internationalization nodes for current language/locale getters and setters, culture lists, display names, and suitable-culture selection.

Use **Get Engine Subsystem** with `BertaLocalizationSubsystem` to bind **On Language Changed** (`PreviousLanguage`, `CurrentLanguage`) and **On Locale Changed** (`PreviousLocale`, `CurrentLocale`). One listener follows the engine lifetime across all worlds and PIE sessions. It caches both names on initialization without emitting events, observes native changes regardless of their caller, and emits only the values that changed. Asset-group-only changes do not emit either event. Listeners that change language/locale again are dispatched after the current transition's events. Deinitialization removes the native subscription and checks internationalization availability during shutdown.

Prefer changing internationalization on the game thread. Notifications received on another thread queue observation of the latest native state on the game thread through a weak subsystem reference. Intermediate off-thread changes may coalesce; stale snapshots cannot reverse a newer transition, and pending observation is ignored after deinitialization. This does not add synchronization to native internationalization setters. Widgets should unbind when destroyed because the subsystem outlives a world or widget. No persistence, setters, or world/game-instance ownership is added.

Automation coverage is under `BertaDevKit.Localization`. Manual verification pending: bind both events in Blueprint, change locale only, language only, both together, and an asset-group culture using native APIs; check payloads and repeat across PIE sessions with proper unbinding. The tests restore the complete native culture snapshot, including asset groups.

## Video playback widget

`UBertaVideoPlayerWidget` is a focused Runtime convenience layer over UE 5.8 Media Framework. It builds its own fullscreen `UImage`, transient `UMediaPlayer`, and transient `UMediaTexture`, so a separate Widget Blueprint or Media Texture asset is not required. Configure a `UMediaSource` and `FBertaVideoPlaybackOptions` on **Create Widget**, optionally bind the events, and then call **Add to Viewport**; native construction, including construction as an embedded child, is the activation point.

```text
Create Widget (BertaVideoPlayerWidget)
→ set Media Source and Options
→ bind On Playback Started / Completed / Failed as needed
→ Add to Viewport
```

The options control autoplay, gameplay pause, removal after terminal natural completion, audio, video looping, and optional start/end transitions. `bLoop` defaults to false; `bRestartExternalAudioOnLoop` defaults to true. With autoplay disabled, the source opens and remains ready until `Play` is called. `Play` also safely queues the request while an asynchronous open is in progress. `Close` is terminal and idempotent: it closes the player, detaches and releases the audio/texture resources, releases only the pause reason acquired by this widget, and removes the widget immediately. External removal cancels transitions and performs the same resource cleanup but leaves a nonterminal widget reusable if it is later added again.

`bUseStartFade` and `bUseEndFade` default to false. `StartFadeDuration` and `EndFadeDuration` default to 0.5 seconds and specify the real-time duration of **each** half of the respective transition; zero completes a half immediately. With start fade enabled, the level stays visible while the source opens, then `Play` fades the level to black **before** starting media playback. Once Media Framework confirms playback, the black layer fades away to reveal the video. With end fade enabled and a usable media duration, the final `EndFadeDuration` seconds fade the still-playing video to black. At natural completion, the widget hides the video, releases playback resources and any pause it owned, broadcasts completion, then fades black back to the level. If the duration is unavailable or the end arrives early, it first forces black before hiding the video. Removal after completion occurs only after the level is fully revealed. Audio continues during the video fade and stops at natural completion. Loop boundaries do not trigger the end fade, and the start fade runs only on the first playback start. Both fades use a fullscreen, `HitTestInvisible` black layer inside the Video Player and continue while gameplay is paused.

Media open and playback completion are delegate-driven rather than polled. `OnPlaybackStarted` fires on a new run after Media Framework confirms playback, not on transport Resume or loop restarts, `OnPlaybackCompleted` is reserved for a terminal natural end and fires after the widget releases its media/audio resources and any pause it owned, and `OnPlaybackFailed` carries a concise error after cleanup and before removal. With `bLoop` enabled, each end seeks back to the beginning and resumes playback, retaining the widget, resources, and owned pause without repeating Started or Completed events. Looping requires a seekable source/backend; a rejected seek or playback request emits failure and cleans up. **Remove on Completion** only determines whether the now-closed widget remains in its parent. Explicit `Close` and failures do not broadcast natural completion.

Audio routing is selected on activation. When `bPlayAudio` is false, neither audio path is created. Otherwise, a valid optional `ExternalAudio` (`USoundBase`, exposed on Create Widget) replaces embedded audio using a widget-owned, non-spatial `UAudioComponent`; without it, the existing `UMediaSoundComponent` plays embedded audio. Native media audio output stays disabled, so the two paths never play together. Both paths use UI sound to continue during gameplay pause. External audio starts only after confirmed video playback, including when autoplay is disabled. Close, failure, destruction, and terminal completion stop and release it. With loop restart enabled, the external track restarts from zero only after the video returns to the beginning and resumes. With restart disabled, loop boundaries leave the external track untouched: if it ends, it stays ended. The widget never enables independent audio looping; any looping authored inside a Sound Wave, Sound Cue, or MetaSound remains in effect. The restart option has no effect on embedded audio.

Pause acquisition uses an authoritative `AGameModeBase` pause delegate tied to this widget. If the world was already paused, the widget records no ownership and therefore never unpauses it. If another pause delegate still blocks unpause, releasing the video pause leaves the world paused. Consequently, **Pause Game** requires an owning Player Controller and authoritative Game Mode; client-only playback should disable that option and leave network pause policy to the game.

The widget accepts `UMediaSource` assets rather than raw file paths and intentionally provides no playlist, subtitle, skip, URL, or playback-rate API. Actual codec/container availability remains determined by the selected Media Framework backend and target platform. Runtime visual/audio behavior still requires manual Unreal verification; the repository verification compiles the feature without launching Unreal.

### Video transport controls

The Blueprint API under **BertaDevKit | UI | Video** is **Play**, **Pause**, **Resume**, **Stop**, **Set Media Source**, and **Close**.

| Control | Contract |
| --- | --- |
| Play | Queues playback while Opening, starts when Ready, succeeds harmlessly while playing, and uses Resume while paused. After Stop, reopens the configured source from the beginning with current Options/External Audio and an explicit playback request, even if Auto Play is false. |
| Pause | Returns true only when native playback accepts Pause. Preserves position, resources, parent, loop configuration and gameplay pause ownership; freezes media reveal/end fades. Opening, the pre-play level-to-black fade, stopped/closed and already-paused calls return false without failure events. |
| Resume | Returns true when a transport-paused run accepts native Play, or processes a queued natural-end boundary. Continues the same player/audio and fade progress, without reopening, seeking, another Started event or another start fade. Unpaused calls return false. Play while paused uses this same path. |
| Stop | Cleans up media/audio/delegates, cancels transitions/loop state and releases only gameplay pause owned by this widget. Retains the parent, source, Options, External Audio and Blueprint event bindings. Remains stopped until explicit Play or source replacement; repeated Stop is harmless. |
| Set Media Source | Cancels/restarts on the same instance according to current Options, including while paused/stopped. Same-source assignment restarts; null clears to reusable inactive state. |
| Close | Terminal cleanup/removal. Neither Play, Stop nor source assignment revives an explicitly closed widget. |

Transport requests do not manufacture playback events. Pause/Resume never repeat Started; Stop never emits completion or failure. Only a terminal natural media end emits Completed, including a queued natural-end notification processed on Resume; ordinary invalid transport timing is not a playback failure. Play after natural completion remains terminal unless a source setter or Stop explicitly resets the run.

Transport Pause uses native Media Framework rate 0, and Resume uses native rate 1; supported behavior depends on the backend. Embedded media audio follows that rate. External audio uses SetPaused on the same component, then unpauses on Resume; Stop stops and releases it. Video transport Pause is **independent of Pause Game**: it retains an acquired gameplay pause until Stop, Close, replacement, failure, destruction or natural completion releases it.

While transport-paused, UI ticks neither advance media-related fades nor start an end fade. Resume shifts the monotonic fade start clock by the paused real-time interval, preserving visible progress. An EndReached notification already queued when Pause succeeds is retained until Resume processes that natural boundary; Stop/replacement discard it. The level-to-black pre-play fade is not pausable transport. A reveal is established before Started listeners run, so a listener may Pause, Stop or replace the source without an old continuation restarting its transition. Stop/replacement invalidate the run generation and unbind old media callbacks before closing the player.

For an embedded child (configure an initial valid source or intentionally clear through the setter before construction):

```text
VideoPlayerChild -> Set Media Source(Video A)
VideoPlayerChild -> Pause
VideoPlayerChild -> Resume
VideoPlayerChild -> Stop
VideoPlayerChild -> Play
```

`BertaDevKit.UI.VideoPlayer.TransportContracts` covers transport bookkeeping, audio pause flags, fade clock suspension, cleanup/late callbacks, stopped activation and event reentrancy without opening a media backend. Automation tests are compiled but require Unreal to execute. Actual position/audio continuity, Blueprint interaction, backend pause support and visual fade behavior remain pending manual verification.

## Image playback widget

`UBertaImagePlayerWidget` constructs a fullscreen, `HitTestInvisible` native `UImage` inside an exclusively owned, fill-aligned overlay. Blueprint subclasses may configure properties and events, but their Designer root/content is not rendered: preserving another texture-bearing image as a sibling would bypass the native fade. No Widget Blueprint, Widget Animation, material, or auxiliary asset is required. It fades the image's own `RenderOpacity`.

```text
Create Widget (BertaImagePlayerWidget)
→ set Texture and Options
→ bind On Display Completed as needed
→ Add to Viewport
```

`Texture` (`UTexture2D`) and `FBertaImagePlaybackOptions` are exposed on **Create Widget**. Defaults are Auto Play and Remove On Completion enabled, both fades enabled at 0.5 seconds, and Display Duration at 3 seconds. With autoplay disabled, the native image remains transparent until `Play`. Call `Play` after construction, normally after **Add to Viewport**. A call before the native visual exists returns false with a warning and can be retried after construction.

A positive fade-in starts its real-time clock on the first Slate update and keeps that first update at opacity zero. Construction and viewport setup before the first rendered frame cannot consume the fade duration. The native image starts transparent before Slate construction, and its opacity is synchronized before the texture brush is assigned. Disabled or zero-duration fade-in remains immediate.

The sequence is linear fade-in (`0 → 1`), fully visible display (`1`), linear fade-out (`1 → 0`), then completion. **Display Duration excludes both fades**: 1 second fade-in, 4 seconds display, and 1 second fade-out take approximately 6 seconds. Disabled fades skip immediately; enabled fades with zero duration reach their endpoint immediately. Zero display immediately proceeds to fade-out or completion. Negative durations clamp to zero. Non-finite display or enabled fade durations fail safely.

Timing uses `FPlatformTime::Seconds()` from Slate-driven `NativeTick`, independently of world pause, delta time, and time dilation. Transitions are observed on UI ticks. The full display interval starts on the tick that sets opacity to exactly 1, so a late fade-in tick never consumes the fully visible hold. Each later stage starts when its preceding stage finishes; frame sampling or a stalled UI can lengthen the total sequence. Options and the brush texture are captured when `Play` starts. Changing Options alone does not alter an active image sequence; use **Set Texture** to restart with current Options.

`Play` returns true when it starts or is already playing, and false after natural completion or `Close`; it never restarts an active or terminal sequence. All-zero durations may broadcast completion synchronously from `Play` or autoplay construction, so bind the delegate before adding the widget. Natural completion first sets terminal state and final opacity, emits `OnDisplayCompleted(Widget)` exactly once, then removes the widget if the captured Remove On Completion option is enabled. If completion or Close occurs inside construction, only removal waits until the first Slate tick, after UE has attached the viewport container. With removal disabled, fade-out leaves the image transparent; disabling fade-out leaves it fully visible.

`Close` is terminal and idempotent, clears the image brush, makes it transparent, and removes the widget without emitting natural completion. An invalid texture returns false, logs a `LogBertaDevKit` warning, and closes the widget. External destruction cancels a nonterminal sequence without completion and clears the visual; as with the Video Player, re-adding that nonterminal instance may start a fresh sequence. Completed instances can be reused through **Set Texture** while still constructed; explicitly closed instances remain terminal.

Source replacement coverage is under `BertaDevKit.UI.ImagePlayer.SourceReplacement` and `BertaDevKit.UI.VideoPlayer.SourceReplacement`. It covers source configuration before construction, same-source restart, null/reuse, current Options, cleanup, preserved parent/event bindings, terminal Close, constructed Slate opacity and completion/start/failure reentrancy. Media-resource tests do not open a backend. Contract tests are under `BertaDevKit.UI.ImagePlayer.PlaybackContracts`. They cover ordering and linear opacity, the full display interval including late fade ticks, captured configuration, disabled fades, zero/negative durations, invalid texture, repeated Play/Close, reentrant completion, deferred removal during construction, and destruction. The additional `BertaDevKit.UI.ImagePlayer.ConstructedHierarchy` test builds actual UMG/Slate children through `RebuildWidget` and `TakeWidget`, including template roots with duplicate textures or colliding names, autoplay/manual startup, Slate opacity/brush synchronization, a delayed first update, and reconstruction. Tests require Unreal to execute. Visual behavior, Blueprint pin presentation, removal in the viewport, and playback during world pause/time dilation remain pending manual verification.

### Replacing sources in embedded player widgets

**Set Media Source** (`UMediaSource`) and **Set Texture** (`UTexture2D`) are the actual Blueprint property setters. Normal Blueprint assignment invokes the same behavior as calling these nodes. Both properties retain Expose On Spawn: UE 5.8 applies the assignments before native construction, so they configure the source without starting against missing visuals. In C++, use the setter methods; direct public member assignment only changes configuration.

These nodes are particularly useful when a Video/Image Player is a child of another Widget Blueprint:

```text
Set player Options
→ Video child: Set Media Source(MS_B)
or
→ Image child: Set Texture(T_B)
```

A constructed player cancels its current sequence, clears old visuals/resources, and starts a fresh activation on the **same widget instance**, keeping its parent attachment and Blueprint event bindings. The interrupted source emits no natural completion. Assigning the same valid source also restarts from the beginning; there is no separate Replay API. No recreation or Add to Viewport is required for an already constructed child.

Each run captures current Options. Video replacement also uses current External Audio and releases/reacquires owned gameplay pause through the normal pause contract. Changing Options or External Audio alone does not restart. With autoplay false, video opens and waits in Ready for Play, while image stays inactive/transparent until Play.

Assigning null intentionally clears/cancels and leaves the player reusable/inactive, without completion, failure notification or removal. A later valid assignment works again. Initial construction with an unconfigured source retains its existing validation behavior. Image replacement still assigns its texture at opacity zero and waits for the first Slate update to anchor a positive fade-in; Display Duration continues to exclude both fades.

With Remove On Completion false, a completed player can run again through its setter. With removal enabled, an already removed/destructed player is only configured for future construction: a setter never reattaches it. **Close remains terminal and removes the widget**; source assignment does not undo explicit Close. Reentrant replacements from Started, Completed or Failed listeners invalidate the interrupted run's pending fade/removal/cleanup continuation.

## Fullscreen fade widget

`UBertaFadeWidget` is an independent Runtime fullscreen transition primitive. It constructs a solid-black native UMG layer and changes only that layer's `RenderOpacity`; no texture, material, Widget Animation, or Widget Blueprint graph is required.

```text
Create Widget (BertaFadeWidget)
→ set Fade Type, Duration, and Remove On Finished
→ optionally bind On Fade Finished
→ Add to Viewport
```

Adding the widget starts a fresh fade automatically. **Fade Out** is transparent to black (`0 → 1` opacity), while **Fade In** is black to transparent (`1 → 0`). `PlayFade` always restarts from the canonical initial opacity using the current properties, including after a completed fade that remained attached. Calling it during playback cancels the previous progression without emitting completion and starts again.

Duration is clamped to at least zero and measured with a monotonic real-time clock from Slate-driven `NativeTick`, so gameplay pause and world time dilation do not control progress. A zero-duration fade sets its final opacity and completes immediately. Natural completion first reaches the exact final opacity, then broadcasts `OnFadeFinished` once, and finally removes the widget when configured. If a listener calls `PlayFade()` during completion, the previous fade does not remove the restarted widget, even when the new fade completes immediately. External removal cancels playback without broadcasting; adding that widget instance again starts a new fade.

The black layer is `HitTestInvisible`, fills the root overlay, and is placed above any existing Widget Blueprint content. A convenient optional asset can therefore be created at `/BertaDevKit/UI/WBP_BertaFade` as an empty Widget Blueprint subclass of `UBertaFadeWidget`; it needs no graph logic or animation. The C++ class remains directly usable. Color, curves, reverse/pause controls, queues, and materials are intentionally outside this focused API. Video playback has its own optional start/end fades.

## AI / Behavior Tree Debugging

These Runtime helpers provide small, composable diagnostics rather than replacing Unreal's Gameplay Debugger, Visual Logger, GLS, Graph Printer, or GAS Companion. Snapshot functions run only when explicitly called; the trace services are event-driven and never Tick.

### Snapshots

- **Gameplay Tag Debug Utils** formats an exact `FGameplayTagContainer` in lexical order, returns UE's supported `FGameplayTagQuery` description, and computes sorted exact added/removed tags with `HasTagExact`. It does not synthesize parent tags. Actor lookup first uses `IGameplayTagAssetInterface`, then falls back to the Actor's ASC through `UAbilitySystemGlobals`; an empty provider container is a successful `None` result.
- **Blackboard Debug Utils** reports the Blackboard asset plus every inherited/local key sorted by name. Key type and current value use `UBlackboardComponent` public APIs and `DescribeKeyValue`, so built-in Object, Class, Bool, numeric, Enum, String, Name, Vector, and Rotator types retain UE's native formatting without raw-memory interpretation.
- **Behavior Tree Debug Utils** reports root/current tree, running and paused state, active node/tasks/trees, and the public runtime path description returned by `UBehaviorTreeComponent::GetDebugInfoString`. It does not access private execution stacks or provide debugger history; use Unreal's specialized debuggers when historical/visual execution analysis is required.
- **AI Debug Utils** composes the Behavior Tree, Blackboard, and existing GAS summaries with controller/Pawn identity. A missing subsystem is shown as `None` without discarding the useful remainder of a valid controller snapshot.

All generated summaries omit timestamps and pointer addresses so separate captures remain practical to diff. The Blueprint summary functions use `DevelopmentOnly` metadata as an authoring hint; the Runtime C++ classes are not claimed to be physically stripped from Shipping builds.

### One-shot Behavior Tree tasks

**Debug Blackboard State** logs one Blackboard snapshot to `LogBertaDebug`; **Debug AI State** logs the combined AI snapshot. Both accept an optional label, complete immediately, never Tick, and generate no persistent observer state. Existing **Debug GAS State** and **Debug Target GAS State** remain useful when a smaller GAS-only capture is preferable.

### Event-driven traces

**Trace Blackboard Changes** observes configured key names while its branch is relevant. With an empty key list and **Trace All Keys When Empty** enabled, it observes all valid Blackboard entries. Each real change emits one concise native value description; missing configured keys produce one warning during registration and are skipped.

**Trace Gameplay Tags** registers `EGameplayTagEventType::AnyCountChange` for each exact configured tag on the controlled Pawn's ASC. Each event logs the callback's actual effective count and presence state. GAS propagates count changes through the changed tag's parent hierarchy, so explicitly observing a parent can also report child-driven parent-count changes. The service intentionally has no observe-all mode.

Both services are instanced per AI. They register only during branch relevance, remove every observer/delegate on cease relevance and instance destruction, and explicitly disable Tick. They observe state only and never modify Blackboard, GAS, or Behavior Tree flow.

## GAS + Behavior Tree

These Runtime nodes bridge UE 5.8 Behavior Trees to the controlled Pawn's Ability System Component without requiring a custom ASC, Pawn, or AIController. Target nodes instead resolve an Actor-compatible Blackboard key and rebind whenever that key changes. With the single documented exception of **Wait Ability Ready**, waits and reactive decorators use GAS/Blackboard delegates rather than Tick, timers, or mirrored Blackboard state. Normal Behavior Tree aborts and relevance changes unregister observers.

- **Conditions:** Gameplay Tag, Gameplay Tag Query, Attribute Threshold, Ability Active, Can Activate Ability, Gameplay Effect Query, Target Gameplay Tag Query, and Target Attribute Threshold.
- **Waits:** Wait Gameplay Tag Query, Wait Gameplay Event, Wait Attribute Threshold, Wait Target Attribute Threshold, Wait Ability End, Wait Target Gameplay Tag Query, Wait Gameplay Effect Applied, Wait Gameplay Effect Removed, and Wait Ability Ready.
- **Actions:** Activate Gameplay Ability And Wait, Cancel Gameplay Ability, Send Gameplay Event, Send Gameplay Event To Target, Apply Gameplay Effect To Self/Target, Remove Gameplay Effects From Self/Target, and Activate Gameplay Ability With Target.
- **Inspection:** Ability cooldown, cost, and full activation readiness without activation or cost side effects.
- **Debug:** GAS Debug Summary, Debug GAS State/Target GAS State, and focused tag-query, attribute, and active-ability assertions.

AI Behavior Trees normally execute on authority, but these helpers do not add RPCs or override GAS networking. Ability activation, Gameplay Event routing, Gameplay Effect application, prediction, and replicated notifications retain their native GAS authority/network semantics. In particular, the applied-effect wait uses the server-side application delegate, while active-effect state reflects the effects visible to that ASC.

### Reactive Gameplay Tag decorators

Both decorators read the controlled Pawn's `UAbilitySystemComponent` directly and use Unreal's normal Behavior Tree **Observer Aborts** setting. They register Gameplay Tag change events only while relevant, so configured `Self`, `Lower Priority`, or `Both` aborts react without a tick, timer, or Blackboard boolean.

- **Gameplay Tag** is the simple single-tag condition.
- **Gameplay Tag Query** evaluates an `FGameplayTagQuery` and automatically observes every unique tag referenced by the query. The query must be non-empty; an empty query or a controlled Pawn without an Ability System Component evaluates false.

For example, a query combining `ALL(State.Combat, Weapon.Ranged)` with `NONE(Status.Stunned)` expresses `(State.Combat && Weapon.Ranged) && !Status.Stunned`. Adding or removing any referenced tag requests immediate Behavior Tree condition re-evaluation according to the configured Observer Aborts policy. Gameplay Tag hierarchy is preserved by GAS, so a query for `State.Combat` also reacts when a child such as `State.Combat.Melee` changes the parent's effective count.

The source is specifically the controlled Pawn's Ability System Component; these decorators do not read arbitrary `IGameplayTagAssetInterface` actors.

### Gameplay Tag Query Wait Task

**Wait Gameplay Tag Query** pauses a Behavior Tree Sequence until its query either **Matches** or **Does Not Match** the controlled Pawn's Ability System Component. Unlike a decorator, which controls whether a branch may execute and can drive Observer Aborts, this latent task represents an explicit sequencing step.

The task evaluates immediately when execution begins. It succeeds without waiting when the requested state already holds; otherwise it observes every tag referenced by the query and completes when a relevant change satisfies the condition. It has no Tick, polling timer, or Blackboard boolean, and a normal Behavior Tree abort unregisters its observers.

For example:

```text
Activate or start attack
→ Wait Gameplay Tag Query
    Query: State.Attacking
    Wait Until: Does Not Match
→ Choose next action
```

An empty query or a controlled Pawn without an Ability System Component fails immediately.

### Activate Gameplay Ability And Wait

**Activate Gameplay Ability And Wait** resolves the configured exact granted ability class on the controlled Pawn's Ability System Component, requests activation, and remains latent until that execution ends. A normal end succeeds; activation rejection and cancellation fail. GAS activation, failure, and per-activation end delegates drive the task, so it does not tick.

The task listens before requesting activation, including abilities that end synchronously from `ActivateAbility`. `Cancel Ability On Abort` cancels the task-owned execution when GAS exposes an instantiated ability identity; it intentionally avoids broad spec cancellation for non-instanced abilities because that could terminate unrelated executions. `Allow Remote Activation` is passed to GAS unchanged, so authority, prediction, and remote execution remain governed by the ability's normal network policy. If GAS only sends a remote request and does not establish an observable local execution, the task fails instead of entering an unfinishable latent state; AI Behavior Trees normally run on authority, where server-executed abilities provide that identity.

### Wait Gameplay Event

**Wait Gameplay Event** waits for the next matching event received by the controlled Pawn's Ability System Component. Exact mode listens only for the configured tag; hierarchical mode uses GAS's native tag-container event routing and also accepts descendant event tags. The task is event-driven, does not queue earlier events or expose payload data, and unregisters immediately after one match or a Behavior Tree abort.

### Attribute Threshold

**Attribute Threshold** compares one attribute on the controlled Pawn's Ability System Component with a configured threshold using `<`, `<=`, `==`, `!=`, `>=`, or `>`. Equality and inequality use the configured non-negative tolerance. Missing attributes evaluate false. The decorator observes GAS's native attribute-value delegate while relevant and drives standard Observer Aborts without ticking or mirroring a Blackboard value.

### Wait Attribute Threshold

**Wait Attribute Threshold** uses the same comparison and tolerance semantics as the decorator. It succeeds immediately when the current value already satisfies the condition; otherwise it listens to GAS's attribute-value delegate and completes on the first satisfying change. Invalid or missing attributes fail, and Behavior Tree abort/stop removes the observer without ticking.

**Wait Target Attribute Threshold** applies that contract to an Actor-compatible Blackboard key. It observes the key even while the target is null, unbinds the previous target ASC before rebinding a replacement, and evaluates each valid target immediately. Missing targets never satisfy the condition; completion and Behavior Tree abort remove both the Blackboard and attribute observers without ticking.

### Wait Ability End

**Wait Ability End** succeeds immediately when the exact configured class is not granted or its spec is already inactive. When active, it listens for GAS ability-end events and re-queries the spec after each matching end; it completes only when no execution remains, including concurrent per-execution abilities. It never ticks and a Behavior Tree abort unregisters the observer without canceling the ability.

### Ability Active

**Ability Active** evaluates the exact granted ability spec's native active state. GAS activation and end callbacks request standard decorator condition re-evaluation, while the spec remains the source of truth so concurrent executions are handled correctly. Missing classes/specs evaluate false; no Blackboard mirror or Tick is used.

### Cancel Gameplay Ability

**Cancel Gameplay Ability** resolves the exact granted class and calls GAS's spec-handle cancellation only when it is active. An already inactive granted spec succeeds because the desired state already holds; invalid, missing-ASC, and ungranted configurations fail. The request may affect multiple active executions of that same spec, but never unrelated ability classes. Compose with **Wait Ability End** when subsequent Behavior Tree flow must wait for termination.

### Send Gameplay Event

**Send Gameplay Event** dispatches a configured tag and magnitude to the controlled Pawn's Ability System Component. The Pawn is the instigator; an optional Actor Blackboard key populates the payload target. Success means the valid event was dispatched through GAS, not that any ability consumed it or activated. Network and prediction behavior remains GAS-owned.

**Send Gameplay Event To Target** instead dispatches to the ASC exposed by a required Actor-compatible Blackboard key. Its payload uses the controlled Pawn as `Instigator`, the selected Actor as `Target`, and preserves the configured magnitude. Success only confirms that a valid target dispatch was issued; it does not imply that an ability consumed the event. The task adds no custom RPC or network policy.

### Apply Gameplay Effect To Self

**Apply Gameplay Effect To Self** builds a normal outgoing spec at the configured finite level and applies it to the controlled Pawn's Ability System Component. It uses UE 5.8's `WasSuccessfullyApplied()` result, which correctly represents both active duration/infinite effects and the special completed handle returned by successful instant effects. GAS authority and prediction rules still determine whether application is accepted.

**Apply Gameplay Effect To Target** uses the same application-result contract, but builds the spec and effect context on the controlled Pawn's source ASC and applies it to the ASC exposed by an Actor-compatible Blackboard key. Missing source/target ASCs, an invalid effect class, a non-finite level, or rejected spec/application fails synchronously. The task adds no RPC or authority override; native GAS networking rules remain in force.

### Remove Gameplay Effects

**Remove Gameplay Effects From Self** and **Remove Gameplay Effects From Target** synchronously remove all active effects matching a configured `FGameplayEffectQuery` from the controlled Pawn or Blackboard-selected Actor ASC. An empty query is rejected so an unconfigured node cannot accidentally remove every effect. Zero matches succeeds as an already-clear state; when matches exist, success requires UE to remove at least one effect. Instant effects do not participate because they are not persistent active effects. UE 5.8 performs removal only for an authoritative ASC; these tasks add no RPC or client-side override.

### Target Gameplay Tag Query

**Target Gameplay Tag Query** resolves an Actor-compatible Blackboard key and evaluates the query against that Actor's Ability System Component. It observes both the Blackboard key and every query tag on the current target. Replacing or clearing the target first unbinds the old ASC, binds the new one when available, and requests standard Observer Abort re-evaluation. Missing targets, missing ASCs, and empty queries evaluate false; no Tick is used.

### Target Attribute Threshold

**Target Attribute Threshold** applies the shared numeric comparison to an attribute on the Actor selected by Blackboard. It observes both the target key and the current target ASC's attribute delegate, rebinding without stale callbacks whenever the target changes. A null target, missing ASC, or missing attribute evaluates false; no Tick or Blackboard value mirror is used.

### Wait Target Gameplay Tag Query

**Wait Target Gameplay Tag Query** waits for `Matches` or `Does Not Match` on the current Blackboard-selected Actor's ASC. It succeeds immediately only when a valid target ASC already has the requested state. While waiting it observes both target replacement and relevant tag changes; a temporarily null target remains pending even for `Does Not Match`. Abort and completion remove both observer sets without ticking.

### Activate Gameplay Ability With Target

**Activate Gameplay Ability With Target** resolves the exact granted ability spec and triggers it through GAS with a compact Gameplay Event payload. The controlled Pawn is the instigator and the Actor from the configured Blackboard key is the target. It does not invent generic target data and does not wait for the ability to end; success means GAS accepted the trigger. Compose with **Wait Ability End** when sequencing requires completion.

### Wait Gameplay Effect Applied

**Wait Gameplay Effect Applied** observes the next matching application to the controlled Pawn's ASC. It uses UE's server-side applied delegate, which includes instant and duration effects, and matches the incoming `FGameplayEffectSpec`; it does not inspect effects that existed before the task started. Because spec matching cannot evaluate active-effect custom match delegates, queries containing native or Blueprint custom delegates fail configuration explicitly. The task is event-driven and unregisters on match or abort.

### Wait Gameplay Effect Removed

**Wait Gameplay Effect Removed** succeeds immediately when no active duration/infinite effect matches its non-empty query. Otherwise it observes all effect removals and re-runs the native query after each one, completing only when zero matches remain. This correctly handles multiple matching effects; instant effects are never active and do not participate. Abort removes the delegate and the task never ticks.

### Gameplay Effect Query

**Gameplay Effect Query** is true when at least one active duration/infinite effect on the controlled Pawn's ASC matches the configured query. Native active-effect addition and removal delegates request standard Observer Abort re-evaluation; each evaluation runs UE's query instead of maintaining a parallel count. Instant effects never become active and therefore do not make this decorator true. Empty queries evaluate false and no Tick is used.

### Can Activate Ability

**Can Activate Ability** resolves the exact granted class and calls the ability's native `CanActivateAbility` path without attempting activation or causing side effects. Invalid, ungranted, or missing-ASC configurations evaluate false.

This decorator is intentionally not advertised as reactive. A custom Gameplay Ability can base `CanActivateAbility` on arbitrary C++ or Blueprint world state for which GAS provides no universal change event. Unreal evaluates the condition whenever the Behavior Tree normally reaches or rechecks it, but configured Observer Aborts cannot be guaranteed to react immediately to every custom readiness change unless some other Behavior Tree event causes reevaluation. Use **Wait Ability Ready** when explicit periodic readiness waiting is required.

### Wait Ability Ready

**Wait Ability Ready** resolves the exact granted ability class and calls its native `CanActivateAbility` immediately. It succeeds at once when ready; otherwise it uses the Behavior Tree's native interval-tick support to recheck at the configured cadence until ready. The interval defaults to `0.1` seconds and is clamped to at least `0.01` seconds. Invalid, ungranted, removed, or missing-ASC abilities fail, and abort clears the per-AI wait state.

This is the campaign's deliberate polling exception: arbitrary custom `CanActivateAbility` logic has no generic GAS change delegate. The task does not create a world timer and does not attempt activation.

### Ability Inspection

`UBertaGASAbilityUtils` provides three Game-Thread-only, side-effect-free Blueprint calls for an exact ability class granted to an Actor's ASC. `Success` means the Actor, ASC, granted spec, actor info, and ability object were available for inspection; the result struct contains the gameplay answer.

- **Get Ability Cooldown Info** reports the ability's native cooldown state separately from general activation readiness, plus finite remaining/duration seconds and a clamped `0..1` remaining fraction. Abilities without cooldown return a ready zeroed result; an active cooldown without finite timing can still report `bIsOnCooldown=true` with zero timing fields.
- **Check Ability Cost** invokes `CheckCost` only. It never calls `ApplyCost` or mutates attributes.
- **Check Ability Activation** invokes `CanActivateAbility` with the granted spec handle and current actor info. It never attempts activation.

Cost and activation `FailureTags` are best-effort diagnostics produced by GAS or the ability. Custom logic may return false with an empty container; BertaDevKit does not fabricate or reinterpret failure tags. These calls retain the ASC's native authority, prediction, cooldown, cost, and custom ability semantics.

### GAS Debug Summary

**Get GAS Debug Summary** is a `DevelopmentOnly` Blueprint-callable snapshot for logs, bug reports, and R&D inspection. Given an Actor exposed through the normal GAS interface, it reports the Actor and ASC identity followed by sorted owned tags, granted abilities with active state and level, active effects with stack/timing information, and available attributes with current values. Empty sections are explicit, output contains no pointer addresses, and invalid Actors or Actors without an ASC return false.

The utility is a compact copy/paste diagnostic, not a logging UI or a replacement for GAS Companion's specialized tooling.

### Debug GAS State

**Debug GAS State** is an immediate Behavior Tree task that resolves the controlled Pawn, reuses **Get GAS Debug Summary**, writes one snapshot to `LogBertaDebug` with an optional label, and succeeds. It fails when there is no valid controlled Pawn/ASC, never ticks, and is intended for development and R&D checkpoints rather than continuous telemetry.

**Debug Target GAS State** applies the same one-shot diagnostic to an Actor-compatible Blackboard target. It reuses the exact same summary formatter, adds an optional label, and fails cleanly for a missing target or ASC. It has no Tick, observer, or persistent logging state.

### GAS Assertion Tasks

**Assert Gameplay Tag Query**, **Assert Attribute Threshold**, and **Assert Ability Active** are immediate R&D tasks for validating controlled-Pawn GAS state at a precise Behavior Tree execution point. A satisfied expectation succeeds silently. Invalid configuration, missing GAS state, or a mismatch emits one bounded `LogBertaDebug` diagnostic with the optional label and returns `Failed`; these nodes never use crash-style C++ assertions.

The tag assertion uses native query matching, the attribute assertion reuses `FBertaGameplayAttributeCondition`, and the ability assertion uses the same exact granted-spec `IsActive()` state as the active-ability decorator. They do not Tick, wait, register delegates, or automatically dump the full GAS summary.

## Editor tools

- **Asset Naming** audits naming conventions and can apply a reviewed rename batch. Both Content Browser actions and public C++ fix entry points restrict renames to valid `/Game` assets, check destination conflicts before execution, and verify final object paths afterward. It also participates in UE Data Validation.
- **Asset Cleaner** identifies conservative unused/orphan asset candidates. Its audit is read-only; cleanup revalidates candidates and opens Unreal's native deletion workflow.
- **Asset Insights** is a read-only Content Browser analysis with saved-package, dependency/referencer, Texture2D, StaticMesh, and conservative footprint information.
- **Project Setup** is an opt-in audit/apply utility for a curated allowlist of preferred project and per-project Editor defaults. It previews changes and does not mutate projects at startup.
- **Blueprint Audit** is a read-only, conservative static linter and review assistant. Findings require manual review; it has no automatic graph rewriting or refactoring action.
- **Blueprint Usage Finder** is a read-only Content Browser action for one project asset. It reports exact Blueprint graph nodes that reference the asset and offers node navigation; it is not a universal reference search.
- **Gameplay Tag Usage Finder** is a read-only Editor Blueprint API that scans selected assets, an explicit `/Game/...` root, or all `/Game` on demand. It recursively inspects reflected `FGameplayTag`, containers, and queries, including Blueprint CDO defaults. Exact matching is the default; parent-or-child matching is a separate explicit mode.
- **World Validation** checks currently loaded actors in the Editor world against enabled policies without changing actors. In World Partition maps, unloaded actors are outside this action's scope.

The main Editor actions are under **Tools → BertaDevKit**. Content Browser context menus provide Asset Naming, Asset Cleaner, Blueprint Audit, and Asset Insights actions for selected project assets and folders.

## Architecture and configuration

BertaDevKit maintains a strict module boundary:

- `BertaDevKit` is the Runtime module available to game code and Blueprints.
- `BertaDevKitEditor` is the Editor-only module for menus, audits, automation, asset tooling, and validation.

The Runtime module never depends on the Editor module. Runtime settings are under **Project Settings → Plugins → BertaDevKit** and persist in `Config/DefaultBertaDevKit.ini`.

The personal external-plugin catalog remains separate in [`PLUGIN_TOOLBOX.md`](https://github.com/nbertoa/ue5-bertadevkit/blob/main/BertaDevKitHost/Plugins/BertaDevKit/Docs/PLUGIN_TOOLBOX.md); it is not part of this site's primary navigation and is not a BertaDevKit dependency.

## Installation

Copy `BertaDevKitHost/Plugins/BertaDevKit/` to `<YourProject>/Plugins/BertaDevKit/`, target UE 5.8, regenerate project files if needed, build, and enable **BertaDevKit**. The repository root is a development host, not the distributable plugin directory.

## Log categories

| Category | Scope |
| --- | --- |
| `LogBertaDevKit` | Runtime utilities and systems |
| `LogBertaDebug` | Debug logging and drawing |
| `LogBertaDevKitEditor` | Editor tooling and validation |
