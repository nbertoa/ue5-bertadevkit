# Validation record — 2026-09-28

## Final closure validation

- Verified commit: `c37b4dffc62e313346c8c68ac3df7ffb113dc697` on `main`. This source commit contains the Audio Thread helper correction and the Memory Insights procedure correction. Its working tree was clean during validation.
- Clean rebuild: **Succeeded**. UBT cleaned `BertaDevKitHostEditor` binaries and compiled **88 actions**; no compiler or linker errors.
- Automation Tests: **5 succeeded, 0 with warnings, 0 failed, 0 not run**, from `BertaDevKitHost/Saved/Automation/SprintClosureCode/index.json` (checked after the Editor exited). All five tests listed below passed on this commit.
- Environment: UE **5.8.2**, changelist **56702186** (`++UE5+Release-5.8`); MSVC **14.51.36256**; Windows SDK **10.0.22621.0**; Win64 Development Editor.

The exact clean-rebuild command was:

```powershell
& 'E:\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' BertaDevKitHostEditor Win64 Development '-Project=E:\Unreal Projects\ue5-bertadevkit\BertaDevKitHost\BertaDevKitHost.uproject' -WaitMutex -Rebuild
```

The exact test command was:

```powershell
& 'E:\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'E:\Unreal Projects\ue5-bertadevkit\BertaDevKitHost\BertaDevKitHost.uproject' '-ExecCmds=Automation RunTests StartsWith:BertaSystemInfo+StartsWith:BertaDesktopCapture+StartsWith:BertaProcessBridge' '-TestExit=Automation Test Queue Empty' -unattended -nop4 -nosplash -NullRHI '-ReportExportPath=E:\Unreal Projects\ue5-bertadevkit\BertaDevKitHost\Saved\Automation\SprintClosureCode'
```

The commit adding this record changes documentation only. Its own SHA cannot be stored inside its tracked contents without changing that SHA. The recorded source commit above is the immutable code/documentation state used for the first clean rebuild and test run; the record-only HEAD is also rebuilt and tested with its separate `SprintClosure` report before the sprint is closed.

## Intermediate sprint validation and environment

- Base commit: [`490c7a7796c2335362c7730e41b979b6abe699c6`](https://github.com/nbertoa/ue5-bertadevkit/commit/490c7a7796c2335362c7730e41b979b6abe699c6) on `main`, initially clean. The base was rebuilt before this sprint's source/documentation edits. Subsequent incremental builds and tests covered the resulting working tree before integration; the commits following this base contain those changes.
- Before integration, `origin/main` advanced through [`4f0aaec`](https://github.com/nbertoa/ue5-bertadevkit/commit/4f0aaec) and [`e9c93be`](https://github.com/nbertoa/ue5-bertadevkit/commit/e9c93be). The merged working tree was rebuilt incrementally and retested before the merge commit.
- Unreal Engine: installed UE **5.8.2**, changelist **56702186** (`++UE5+Release-5.8`); Win64 Development Editor target. MSVC **14.51.36256**, Windows SDK **10.0.22621.0**.
- Local ignored third-party plugin installations were present. Their warnings were observed in the full rebuild; this record does not claim a separate dependency-free clone build or redistribution of those plugins.

## Reproduce

From the repository root in PowerShell, substitute the local UE 5.8 installation path:

```powershell
& '<UE_5.8>\Engine\Build\BatchFiles\Build.bat' BertaDevKitHostEditor Win64 Development '-Project=<repo>\BertaDevKitHost\BertaDevKitHost.uproject' -WaitMutex -Rebuild
```

The intermediate `-Rebuild` ran UBT's clean phase and compiled 88 actions on the base commit. Result: **Succeeded**, no compiler or linker errors. After the SystemInfo fix and DesktopCapture trace markers, the same command without `-Rebuild` compiled the affected modules and returned **Succeeded**. It also returned **Succeeded** for the merged working tree, compiling the new remote ProcessBridge and Editor changes. These earlier results are not the final closure validation above.

Run the related Automation Tests with the native command-line Editor:

```powershell
& '<UE_5.8>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' '<repo>\BertaDevKitHost\BertaDevKitHost.uproject' '-ExecCmds=Automation RunTests StartsWith:BertaSystemInfo+StartsWith:BertaDesktopCapture+StartsWith:BertaProcessBridge' '-TestExit=Automation Test Queue Empty' -unattended -nop4 -nosplash -NullRHI '-ReportExportPath=<repo>\BertaDevKitHost\Saved\Automation\SprintMerged'
```

Inspect `Saved/Automation/SprintMerged/index.json`, not only the process exit code. This intermediate merged-tree report recorded **5 succeeded, 0 warnings, 0 failed, 0 not run**; the final source-commit report above recorded the same five passes:

| Test | Result |
| --- | --- |
| `BertaDesktopCapture.FrameCopy.RespectsRowPitch` | Passed |
| `BertaProcessBridge.Launch.InvalidContextClearsOutputs` | Passed |
| `BertaProcessBridge.Lifecycle.InvalidOptionsAndShutdownRejectLaunch` | Passed |
| `BertaProcessBridge.Lifecycle.BlockedInputOwnerShutdown` | Passed; exercises a native child blocked on stdin during owner teardown. |
| `BertaSystemInfo.Audio.OutputDevicesRejectAudioThreadBeforeWorldLookup` | Passed; the test starts an actual Audio Thread context if the command-line Editor did not start one, then restores the prior setting. |

The SystemInfo test first proves its context spy observes a Game Thread `GetWorld()` call. It then calls the API in the Audio Thread context and checks rejection, cleared output, and unchanged world-lookup count.

## Warnings and limits

- UBT reported MSVC 14.51 as newer than its preferred 14.50 toolchain. Compilation succeeded in the intermediate and closure rebuilds.
- The clean rebuild emitted UE 5.8 deprecation warnings from engine headers and warnings from locally installed, Git-ignored commercial plugin code, including assignment-as-condition and float narrowing. These did not stop the build and were outside this sprint's scope.
- Unreal platform validation reported Win64 SDK valid; other platform SDKs were unavailable. Only Win64 was built.
- The Automation Tests used `-NullRHI`. They establish the tested contracts, including native process teardown, but not real display capture, audio-device enumeration, packaged behavior, or GPU/RHI performance. The DesktopCapture 1080p/30 profiling procedure is [separate](../bertadesktopcapture/benchmark.md).
