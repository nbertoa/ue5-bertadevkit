# Validation record — 2026-09-28

## Revision and environment

- Base commit: [`490c7a7796c2335362c7730e41b979b6abe699c6`](https://github.com/nbertoa/ue5-bertadevkit/commit/490c7a7796c2335362c7730e41b979b6abe699c6) on `main`, initially clean. The base was rebuilt before this sprint's source/documentation edits. Subsequent incremental builds and tests covered the resulting working tree before integration; the commits following this base contain those changes.
- Unreal Engine: installed UE **5.8.2**, changelist **56702186** (`++UE5+Release-5.8`); Win64 Development Editor target. MSVC **14.51.36256**, Windows SDK **10.0.22621.0**.
- Local ignored third-party plugin installations were present. Their warnings were observed in the full rebuild; this record does not claim a separate dependency-free clone build or redistribution of those plugins.

## Reproduce

From the repository root in PowerShell, substitute the local UE 5.8 installation path:

```powershell
& '<UE_5.8>\Engine\Build\BatchFiles\Build.bat' BertaDevKitHostEditor Win64 Development '-Project=<repo>\BertaDevKitHost\BertaDevKitHost.uproject' -WaitMutex -Rebuild
```

`-Rebuild` ran UBT's clean phase and compiled 88 actions on the base commit. Result: **Succeeded**, no compiler or linker errors. After the SystemInfo fix and DesktopCapture trace markers, the same command without `-Rebuild` compiled the affected modules and returned **Succeeded**.

Run the related Automation Tests with the native command-line Editor:

```powershell
& '<UE_5.8>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' '<repo>\BertaDevKitHost\BertaDevKitHost.uproject' '-ExecCmds=Automation RunTests StartsWith:BertaSystemInfo+StartsWith:BertaDesktopCapture+StartsWith:BertaProcessBridge' '-TestExit=Automation Test Queue Empty' -unattended -nop4 -nosplash -NullRHI '-ReportExportPath=<repo>\BertaDevKitHost\Saved\Automation\SprintFinal'
```

Inspect `Saved/Automation/SprintFinal/index.json`, not only the process exit code. The final report recorded **4 succeeded, 0 warnings, 0 failed, 0 not run**:

| Test | Result |
| --- | --- |
| `BertaDesktopCapture.FrameCopy.RespectsRowPitch` | Passed |
| `BertaProcessBridge.Launch.InvalidContextClearsOutputs` | Passed |
| `BertaProcessBridge.Lifecycle.InvalidOptionsAndShutdownRejectLaunch` | Passed |
| `BertaSystemInfo.Audio.OutputDevicesRejectAudioThreadBeforeWorldLookup` | Passed; the test starts an actual Audio Thread context if the command-line Editor did not start one, then restores the prior setting. |

The SystemInfo test first proves its context spy observes a Game Thread `GetWorld()` call. It then calls the API in the Audio Thread context and checks rejection, cleared output, and unchanged world-lookup count.

## Warnings and limits

- UBT reported MSVC 14.51 as newer than its preferred 14.50 toolchain. Compilation succeeded.
- The clean rebuild emitted UE 5.8 deprecation warnings from engine headers and warnings from locally installed, Git-ignored commercial plugin code, including assignment-as-condition and float narrowing. These did not stop the build and were outside this sprint's scope.
- Unreal platform validation reported Win64 SDK valid; other platform SDKs were unavailable. Only Win64 was built.
- The Automation Tests used `-NullRHI`. They establish the tested contracts, not real display capture, audio-device enumeration, packaged behavior, or GPU/RHI performance. The DesktopCapture 1080p/30 profiling procedure is [separate](../bertadesktopcapture/benchmark.md).
