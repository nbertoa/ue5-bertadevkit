# BertaDesktopCapture

BertaDesktopCapture is an independent Unreal Engine 5.8 Win64 Runtime plugin that captures a physical display or visible top-level desktop window into a live transient `UTexture2D` for UMG, materials, dashboards, visualization, and R&D.

Complete technical documentation: <https://nbertoa.github.io/ue5-bertadevkit/bertadesktopcapture/>

## Requirements and installation

- Unreal Engine 5.8
- Win64
- Windows 10 version 1903 (build 18362) or later, where programmatic Windows Graphics Capture interop is available
- A supported C++ toolchain when building from source

Copy this directory into a UE 5.8 project:

```text
BertaDevKitHost/Plugins/BertaDesktopCapture/
→ <YourProject>/Plugins/BertaDesktopCapture/
```

Build the project and enable **BertaDesktopCapture**. The plugin is disabled by default, contains no third-party libraries, and has no dependency on any other Berta plugin.

## API

`Is Desktop Capture Supported` performs a runtime Windows Graphics Capture capability check. `Get Display Capture Sources` rebuilds UE display metrics; display IDs are exact opaque `FMonitorInfo::ID` values. `Get Window Capture Sources` returns current visible, titled, non-cloaked top-level windows. Window IDs are opaque, process-local, ephemeral values and must never be parsed or persisted. Window titles are returned locally only for selection and are not logged or saved.

`Start Desktop Capture` takes a source and `MaxFrameRate` from 1 through 60, creates a `UBertaDesktopCaptureSession`, and returns a transient SDR `PF_B8G8R8A8` sRGB texture. The frame rate is a maximum processing/upload cadence, not a source-rate guarantee. Use `Get Texture` and `Get Frame Size` to consume the current output. `Stop Capture` is terminal and reports `UserStopped` once through `OnStopped`.

Each active session is strongly owned by the supplied world context's `GameInstance`. Source closure reports `SourceClosed`; an unrecoverable backend/readback error reports `CaptureFailed`. GameInstance teardown suppresses user callbacks and closes all native capture resources. `OnTextureChanged` fires only when a resize replaces the texture, never per frame. Rebind the new texture after that event.

Blueprint window flow:

```text
Get Window Capture Sources → select source → Start Desktop Capture
→ Get Texture → assign to UMG/material → Stop Capture
```

Display flow is identical, beginning with `Get Display Capture Sources`.

When BertaSystemInfo is also installed, `FBertaDisplayInfo::Id` semantically matches a Display source `Id`; neither plugin depends on the other. BertaDesktopCapture also has no dependency on BertaWindowTools and never moves, focuses, resizes, or retitles external windows. Self-capture and recursive visuals remain the caller's responsibility.

## Backend and limitations

The backend uses programmatic Windows Graphics Capture (`CreateForWindow` / `CreateForMonitor` and a free-threaded frame pool) with a dedicated D3D11 device. Frames are copied to a reused staging texture, read row-by-row into tightly packed BGRA8 CPU memory, and uploaded through UE's public Runtime texture API. This keeps capture independent of whether Unreal runs D3D11 or D3D12, at the cost of a GPU→CPU→GPU transfer.

Only the latest complete CPU frame is retained; older pending frames are dropped, and rate limiting happens before expensive readback. High resolutions and frame rates can still be costly, especially 4K. Source resizes recreate the frame pool, staging resource, and Unreal texture without stretching old content.

The MVP is SDR BGRA8 only. It has no audio, recording, file export, scaling, cropping, per-frame Blueprint event, input control, remote desktop, HDR processing, or protected-content bypass. HDR content may look clipped, washed out, or otherwise inaccurate. Protected surfaces may appear black or refuse capture according to Windows policy.

Win64 compilation is verified through the repository host. Runtime/visual behavior and packaged behavior require manual verification; Unreal is not launched by the implementation workflow.
