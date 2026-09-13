# BertaWindowTools

BertaWindowTools is an independent Unreal Engine 5.8 Runtime plugin for inspecting and controlling the game window belonging to a supplied world context. It exposes a stateless C++ and Blueprint API and does not enumerate, accept handles for, or manipulate unrelated operating-system windows.

## Installation

Copy the plugin into a UE 5.8 project's plugin directory:

```text
BertaDevKitHost/Plugins/BertaWindowTools/
→ <YourProject>/Plugins/BertaWindowTools/
```

Build the project and enable **BertaWindowTools**. It is disabled by default, contains no third-party libraries, and has no dependency on another Berta plugin.

## Target resolution and safety

Each call resolves `WorldContextObject → UWorld → UGameInstance → UGameViewportClient → SWindow` on demand. There is no subsystem, cached window pointer, global viewport fallback, tick, or lifecycle state. C++ calls must occur on the Game Thread; off-thread calls fail with `OperationUnsupported` instead of being queued.

In UE 5.8, `UGameViewportClient::GetWindow()` returns the `SWindow` containing the viewport. Embedded PIE can therefore resolve the main Unreal Editor window. Runtime APIs cannot reliably distinguish that window from a dedicated PIE window without an Editor dependency, so BertaWindowTools rejects all operations—including `Get Game Window Info`—when `GIsEditor` is true. The error is `EmbeddedViewportUnsupported`. Manual tests must use a separate-process Standalone game or a packaged build; PIE in New Editor Window is intentionally rejected as well.

## Window information

`Get Game Window Info` returns `FBertaWindowInfo`:

| Field | Meaning |
| --- | --- |
| `Title` | Current window title. |
| `DesktopPosition` | Top-left of the outer window in virtual desktop pixels. |
| `WindowSize` | Outer window size in desktop pixels. |
| `ClientSize` | Drawable client-area size in pixels. |
| `DPIScale` | Native ratio of pixels to Slate units. |
| `WindowMode` | `Unknown`, `Windowed`, `WindowedFullscreen`, or `Fullscreen`. |
| State booleans | Visibility, Slate activity, native foreground, minimized, and maximized state. |
| `bHasDisplay`, `DisplayId` | Whether a monitor association was derived and its opaque UE ID. |

Coordinates share UE `FDisplayMetrics` virtual-desktop pixel space, so monitors left of the primary display retain negative coordinates. Client-size inputs are also pixels. UE 5.8's `SWindow::Resize` expects an already-DPI-scaled client size, so the plugin performs no additional Slate-unit conversion and avoids compounding DPI scale. For a minimized Win64 window, the reported outer rectangle is its meaningful restored geometry rather than Win32's possible iconic sentinel position.

UE's default Win64 game window uses a native OS border. Its cached `SWindow` position and size correspond to client geometry, while this API promises outer geometry. The implementation therefore uses a small private Win64 correction to read and move only the native window already resolved from the caller's `GameInstance`. All remaining behavior uses portable UE Runtime APIs, and no native handle crosses the plugin API.

## Blueprint API

All nodes reset `OutError` and return a `Success` boolean.

| Node | Contract |
| --- | --- |
| `Get Game Window Info` | Resets and fills `FBertaWindowInfo` for the resolved game window. |
| `Set Window Position` | Moves the outer top-left to an absolute virtual-desktop pixel position. |
| `Set Window Client Size` | Requests a positive drawable size in pixels; project/platform constraints may clamp it. |
| `Center Window On Display` | Exact opaque display-ID match; centers in work area or full display rect. |
| `Center Window On Primary Display` | Finds UE's primary monitor and centers without requiring BertaSystemInfo. |
| `Maximize Window` | Issues the platform maximize request. |
| `Minimize Window` | Issues the platform minimize request. |
| `Restore Window` | Issues the platform restore request. |
| `Bring Window To Front` | Issues non-forced UE front/focus requests; a minimized window must be restored explicitly first. |
| `Set Window Title` | Sets the title through `SWindow`; empty text is allowed. |

Position, resize, and center operations require `Windowed` mode and a window that is neither minimized nor maximized. They return `WindowNotWindowed` or `WindowNotRestored` rather than silently changing mode or restoring. Maximize, minimize, and restore also reject fullscreen modes. Native state and focus transitions are requests: success does not guarantee synchronous completion or override operating-system foreground restrictions.

When centering, an outer window larger than the chosen destination area is placed at that area's top-left without being resized. Unknown IDs return `DisplayNotFound`; there is no primary-display fallback for an invalid ID.

## Display interoperability

Display IDs are opaque UE `FMonitorInfo::ID` values and must not be parsed. BertaSystemInfo exposes the same native value in `FBertaDisplayInfo::Id`, so callers with both plugins installed can pass that field directly to `Center Window On Display`. This is optional semantic interoperability, not a module or plugin dependency.

For `FBertaWindowInfo`, the monitor with the greatest outer-window intersection area wins. Positive-overlap ties use UE's existing monitor order. With no overlap, the display rectangle nearest to the window center wins. With no monitor data, `bHasDisplay` remains false.

## Deliberate limits and verification

BertaWindowTools does not create custom windows, control external windows, mutate topmost state, set window mode or resolution, apply `UGameUserSettings`, hide/show windows, change styles, confine the cursor, persist layouts, monitor hotplug, override DPI, or implement platform taskbar/tray behavior.

The repository verifies `BertaDevKitHostEditor Win64 Development` compilation. Runtime and packaged Win64 behavior and behavior on every other platform remain manual verification work; no all-platform runtime support is claimed.
