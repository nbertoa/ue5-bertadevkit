# Desktop capture 1080p/30: reproducible profiling procedure

## Question and current status

The Windows backend copies each accepted Windows Graphics Capture D3D11 frame into a staging texture, maps it for CPU readback, packs BGRA rows, and calls `UTexture2D::UpdateTextureRegions` for an Unreal texture. This is a real GPU → CPU → GPU path. Code inspection establishes the transfers, but does **not** establish their cost. No 30-second capture measurement or acceptable-cost conclusion has been recorded yet.

The source now contains `TRACE_CPUPROFILER_EVENT_SCOPE` markers for frame arrival, readback/packing, accepted frames, rate-limited frames, mailbox overwrites, upload submission, and upload completion. They add observation without changing the transfer algorithm. The existing `BertaDesktopCapture.FrameCopy.RespectsRowPitch` test verifies row packing, not performance.

## Setup

1. Build `BertaDevKitHostEditor Win64 Development` with UE 5.8, then open `BertaDevKitHost.uproject` in **UnrealEditor.exe** with the machine's real D3D11 or D3D12 RHI. Do not use `-NullRHI` for this run. Open `/Game/Maps/DebugMap` and run PIE or Standalone with a real `GameInstance`.
2. Use a 1920×1080 display or a visible 1920×1080 window whose contents visibly change throughout the run. A static or occluded source may not produce 30 frame callbacks per second. Record source type, actual `Get Frame Size`, RHI, GPU, resolution, and whether the captured source includes the Unreal viewport (self-capture changes the workload).
3. In a temporary Level Blueprint or test actor, call `Get Display Capture Sources` (or `Get Window Capture Sources`), select the exact 1920×1080 source, then call `Start Desktop Capture` with `MaxFrameRate = 30`. Keep the returned session in a variable. Assign `Get Texture` to a visible material/UMG image so the render/upload path is exercised. Stop the session after the run. Do not add this temporary setup to the repository unless it becomes a durable test.
4. Warm up for about five seconds. Measure **30 seconds**, and repeat three times. Run a matching 30-second baseline with the same animated source and scene but without starting capture. Avoid changing viewport resolution, graphics settings, or source content between passes.

## Trace and memory commands

UE 5.8 implements `Trace.File`, `Trace.Bookmark`, and `Trace.Stop` in `TraceAuxiliary.cpp`. From the Editor console, start a file trace just after warm-up:

```text
Trace.File cpu,gpu,frame,bookmark
Trace.Bookmark BertaCaptureBegin
```

After approximately 30 seconds:

```text
Trace.Bookmark BertaCaptureEnd
Trace.Stop
```

The trace is written to Unreal's profiling trace directory; note the `.utrace` path reported by the console/log. Open it with `<UE_5.8>/Engine/Binaries/Win64/UnrealInsights.exe`. The Blueprint can issue these commands through `Execute Console Command` and two `Delay` nodes (5 seconds warm-up, 30 seconds sample) to keep the interval consistent. Use the same sequence for the baseline.

In a separate PowerShell window, sample the Unreal process during the same interval. Select the actual Editor PID if more than one instance exists:

```powershell
$editorProcess = Get-Process UnrealEditor | Sort-Object StartTime -Descending | Select-Object -First 1
1..60 | ForEach-Object {
    $sample = Get-Process -Id $editorProcess.Id
    [pscustomobject]@{
        Time = (Get-Date).ToString('o')
        WorkingSetBytes = $sample.WorkingSet64
        PeakWorkingSetBytes = $sample.PeakWorkingSet64
        PrivateBytes = $sample.PrivateMemorySize64
    }
    Start-Sleep -Milliseconds 500
} | Export-Csv -NoTypeInformation "$env:TEMP\BertaCaptureMemory.csv"
```

`WorkingSetBytes` and `PrivateBytes` are process-wide, so compare their baseline and capture deltas. `PeakWorkingSetBytes` is a process-lifetime high-water mark, not necessarily a peak only within the 30-second window. For allocation attribution, run a **separate** trace with `Trace.File cpu,frame,bookmark,Memory_Light`; allocation tracing itself may perturb timing.

## Read the result

In Unreal Insights, restrict the Timing view to the two bookmarks and record:

| Metric | Where and interpretation |
| --- | --- |
| Game Thread, Render Thread, RHI Thread | Frame tracks: median/p95/p99 and number of frames above 33.3 ms. Record “RHI Thread not active” when absent. Compare with baseline. |
| Readback/packing cost | Count and duration distribution of `BertaDesktopCapture_ReadbackAndPack` on the Windows capture callback thread. Its scope includes the readback, row packing, and final frame publication, so it is an upper bound for readback alone. A long `Map` can include a GPU wait. |
| Accepted versus uploaded frames | Counts of `BertaDesktopCapture_AcceptedFrame` and `BertaDesktopCapture_UploadComplete`; roughly 900 completed uploads in 30 seconds would sustain 30 FPS. Boundary frames can shift the count slightly. |
| Plugin-side dropped work | Counts of `BertaDesktopCapture_MailboxOverwrite`; these identify accepted CPU frames replaced before upload. `BertaDesktopCapture_RateLimited` counts intentional drops **before** readback when the source presents faster than the 30 FPS cap; do not label those overload. Windows may drop frames before the callback, which these markers cannot count. |
| Upload path | `BertaDesktopCapture_UploadFrame` marks Game Thread submission; inspect render/RHI/GPU tracks around the same times for texture update cost. |
| Memory | Baseline/capture process working set and private-byte deltas, sample maximum, and any sustained climb across the 30 seconds. A single BGRA8 1080p frame is 8,294,400 bytes before row padding and staging/texture copies. |

Record all three passes and the baseline with the hardware and exact trace file paths. A measured frame-time regression, persistent mailbox overwrite, failure to approach the 30 FPS target, or sustained memory growth would justify investigating the pipeline. If those do not appear and the cost is acceptable for the intended use, record that result and close the optimization question. Do not replace the CPU bridge based only on its presence in the code.

## Why this sprint has no numeric result

The checkout has no preconfigured capture scene or automated 1920×1080 moving source. The installed machine reports a 1920×1080 primary display, but that alone does not establish a controlled 30 FPS source or a representative render workload. The procedure and trace markers are ready; a timed capture pass is still required before drawing a performance conclusion.
