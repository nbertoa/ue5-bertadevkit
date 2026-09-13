# BertaDesktopCapture

BertaDesktopCapture is a Win64 Runtime C++ and Blueprint plugin for capturing a physical display or a visible top-level desktop window into a continuously updated Unreal `UTexture2D`. It targets Unreal Engine 5.8 and requires Windows 10 version 1903 (build 18362) or later for programmatic Windows Graphics Capture interop.

## Install

```text
BertaDevKitHost/Plugins/BertaDesktopCapture/
→ <YourProject>/Plugins/BertaDesktopCapture/
```

Build the project and enable **BertaDesktopCapture**. It is an independent sibling plugin with no third-party libraries and no dependency on BertaDevKit, BertaSystemInfo, BertaWindowTools, or another Berta plugin.

## Sources and starting capture

`Is Desktop Capture Supported` checks the required runtime APIs. `Get Display Capture Sources` returns current UE display metrics. Display `Id` is the exact opaque UE `FMonitorInfo::ID`; size is the display rectangle and the primary flag comes from UE. When BertaSystemInfo is also installed, `FBertaDisplayInfo::Id` can be used as the semantically matching display ID, without a module dependency.

`Get Window Capture Sources` enumerates current visible, titled, non-cloaked top-level Win32 windows. Its `Name` is the current title and `Size` is the current outer size. Window IDs encode the current native handle behind a namespaced, process-local value. They are opaque, ephemeral, valid only in the current runtime session, and revalidated at capture start. Do not parse or persist them. Titles are local selection metadata only; the plugin does not log, save, transmit, or augment them with process information.

Call `Start Desktop Capture` with one source, `MaxFrameRate` in the range 1–60, and a world context. A successful call returns a `UBertaDesktopCaptureSession` already owning a transient SDR `PF_B8G8R8A8`, sRGB `UTexture2D` sized from the Windows capture item.

```text
Get Window Capture Sources → select source → Start Desktop Capture
→ Get Texture → assign to UMG/material → Stop Capture
```

For a display, begin with `Get Display Capture Sources` and use the same remaining flow.

## Session contract

- `Is Capturing` reports the terminal state.
- `Get Source` returns the launch-time source snapshot; its name and size are not live metadata.
- `Get Texture` returns the current texture.
- `Get Frame Size` returns its current pixel dimensions.
- `Stop Capture` succeeds once and produces `UserStopped`; later calls return false.
- `On Texture Changed` fires only after a source resize replaces the texture. It is not a per-frame event.
- `On Stopped` fires exactly once with `UserStopped`, `SourceClosed`, or `CaptureFailed`.

Sessions belong to the exact `GameInstance` resolved from the world context. Teardown suppresses user delegates, invalidates pending dispatch, and closes the capture item, session, frame pool, staging resources, and D3D objects.

## Frame pipeline and performance

Windows Graphics Capture uses `CreateForWindow` / `CreateForMonitor` and `Direct3D11CaptureFramePool::CreateFreeThreaded` on a dedicated D3D11 device. Every accepted frame is copied into a reused staging texture, mapped for CPU access, packed row-by-row while respecting `RowPitch`, and uploaded with UE's public `UTexture2D::UpdateTextureRegions` API. UObject and texture work occurs only on the Game Thread/render path.

This CPU bridge works independently of Unreal's D3D11 or D3D12 RHI but performs a GPU→CPU→GPU transfer. `MaxFrameRate` caps accepted processing—not the source refresh—and excess frames are dropped before readback. A bounded latest-frame mailbox replaces any older pending CPU frame, preventing backlog and growing latency. High-resolution/high-rate capture can be expensive, especially at 4K.

When source dimensions change, the frame pool and staging resource are recreated, incompatible pending pixels are discarded, and a new Unreal texture is delivered through `OnTextureChanged`. Closing a captured window ends the session with `SourceClosed`; monitor removal is reported according to the failure Windows surfaces and never switches to another display.

## Scope and limitations

The output is SDR BGRA8 only. Windows HDR content may be clipped, washed out, or color-inaccurate. The plugin does not bypass protected content; protected surfaces may be black or unavailable. It provides no audio, recorder, file exporter, remote-desktop behavior, input injection, per-frame Blueprint delegate, arbitrary window control, scaling, crop, cursor option, or HDR pipeline. Self-capture can create recursive visuals and is the caller's responsibility.

Build verification currently covers `BertaDevKitHostEditor Win64 Development`. Runtime/visual and packaged Win64 verification remain manual.
