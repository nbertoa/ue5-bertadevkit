# BertaWindowTools

BertaWindowTools is an independent Unreal Engine 5.8 Runtime plugin for inspecting and controlling the game window associated with a supplied world context. Its stateless C++ and Blueprint API can move, resize, center, maximize, minimize, restore, focus, and retitle that window without enumerating or controlling unrelated operating-system windows.

Complete technical documentation: <https://nbertoa.github.io/ue5-bertadevkit/bertawindowtools/>

## Requirements and installation

- Unreal Engine 5.8
- A supported C++ toolchain when building from source

Copy this directory into a UE 5.8 project:

```text
BertaDevKitHost/Plugins/BertaWindowTools/
→ <YourProject>/Plugins/BertaWindowTools/
```

Build the project and enable **BertaWindowTools**. The plugin is disabled by default, contains no third-party libraries, and has no dependency on any other Berta plugin.

## API and units

Every function resolves `WorldContextObject` through its exact `UWorld`, `UGameInstance`, and `UGameViewportClient`; it never falls back to a global viewport. Calls are Game-Thread-only and return `false` with an `EBertaWindowError` when their contract cannot be satisfied.

`Get Game Window Info` returns title, outer desktop position and size, drawable client size, DPI scale, window mode, visibility/activity/foreground state, minimized/maximized state, and the associated display when determinable. Positions and sizes are integer desktop pixels in UE's virtual desktop coordinate space, including negative monitor coordinates. Display rectangles use the same coordinate space. A minimized Win64 window reports its meaningful restored outer rectangle rather than Win32's possible iconic sentinel position.

`Set Window Position`, `Set Window Client Size`, `Center Window On Display`, and `Center Window On Primary Display` require a restored `Windowed` window. They never restore or change window mode implicitly. Client-size values are already DPI-scaled pixels, matching UE 5.8's `SWindow::Resize` contract; BertaWindowTools does not divide or multiply them by the window DPI. Project and platform size constraints may still clamp a resize request.

Centering preserves the current outer size and can use the display work area or full display rectangle. If the window is larger than the chosen area, its outer top-left is placed at the area's top-left. An unknown display ID returns `DisplayNotFound` and never falls back to the primary display.

`Maximize Window`, `Minimize Window`, and `Restore Window` issue native requests for `Windowed` windows. A successful return means the request was issued, not that the platform completed the transition synchronously. `Bring Window To Front` uses UE's non-forced front and focus APIs, refuses a minimized window instead of restoring it implicitly, and cannot guarantee overriding operating-system foreground policies. `Set Window Title` accepts empty text.

## Display IDs and BertaSystemInfo

Display IDs are opaque UE `FMonitorInfo::ID` values and must be matched exactly. When BertaSystemInfo is also installed, an `FBertaDisplayInfo::Id` can be passed directly to `Center Window On Display`. This is semantic compatibility only: neither plugin depends on the other.

The associated display is chosen by greatest outer-window intersection area. Ties keep UE's deterministic monitor order. If the window intersects no display, the nearest display rectangle to the window center is used. If UE reports no monitors, `bHasDisplay` is false and `DisplayId` is empty.

## Editor safety and scope

UE 5.8's `UGameViewportClient::GetWindow()` returns the `SWindow` containing the viewport. In embedded PIE that can be the main Unreal Editor window, and Runtime APIs do not reliably identify the distinction. BertaWindowTools therefore rejects every query and mutation while `GIsEditor` is true with `EmbeddedViewportUnsupported`. Use a separate-process Standalone game or a packaged runtime for manual window testing. This restriction also intentionally rejects PIE in New Editor Window.

The implementation uses UE Runtime `SWindow`, `FGenericWindow`, and `FDisplayMetrics` APIs. On Win64, UE's default native-bordered game window reports cached Slate geometry in client coordinates, so a private native correction reads and moves that already-resolved game window by its outer rectangle. No native handle is exposed, cached, enumerated, or accepted from callers.

BertaWindowTools does not create windows, control external windows, change fullscreen/resolution settings, persist layouts, implement Always On Top, confine the cursor, change OS window styles, or manage DPI policy. Use `UGameUserSettings` for fullscreen and resolution changes.

Win64 compilation is verified through the repository host. Runtime window behavior, packaged behavior, and behavior on other platforms require manual verification; no cross-platform runtime claim is made.
