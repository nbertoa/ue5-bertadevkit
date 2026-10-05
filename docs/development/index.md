# Development

## Repository architecture

`BertaDevKitHost` is the development and verification harness. It contains seven independent base plugins and four optional integration plugins:

```text
BertaDevKitHost/
├── BertaDevKitHost.uproject
└── Plugins/
    ├── BertaDevKit/
    ├── BertaDualSense/
    ├── BertaSystemInfo/
    ├── BertaProcessBridge/
    ├── BertaWindowTools/
    ├── BertaDesktopCapture/
    ├── BertaSerial/
    ├── BertaGASCompanionExt/
    ├── BertaComboGraphExt/
    ├── BertaUltimateGameplayCameraExt/
    └── BertaBlackEyeCameraExt/
```

All eleven tracked plugins target Unreal Engine 5.8. The seven base plugins remain independent siblings. The four optional integrations are disabled by default and require legitimate local installations of GAS Companion, Combo Graph, Ultimate Gameplay Camera, and Black Eye Camera respectively; those third-party directories are Git-ignored. The GAS Companion integration also depends on BertaDevKit. `BertaBlackEyeCameraExt` preserves Runtime/Editor separation and works through standard local ViewTarget transitions; see its [plugin page](../bertablackeyecameraext/index.md). BertaDualSense, BertaDesktopCapture, and BertaSerial are Win64-only. BertaSystemInfo, BertaProcessBridge, and BertaWindowTools compile without a platform allowlist; their runtime behavior outside Win64 has not been verified.

## Local validation runner

From the repository root, use Windows PowerShell 5.1 or later with no external modules:

```powershell
.\Scripts\Validate-BertaDevKit.ps1
```

If Windows PowerShell blocks script execution under your local policy, use a process-only override: `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Scripts\Validate-BertaDevKit.ps1`.

This builds `BertaDevKitHostEditor Win64 Development` with `-WaitMutex` (incremental, without `-Rebuild`), then launches `UnrealEditor-Cmd.exe` with `-NullRHI` to run the Runtime and Editor Automation Tests belonging to BertaDevKit. It waits for `Automation Test Queue Empty`; it does not issue an early `Quit`. Launching Unreal, including through this runner, requires explicit authorization when working with Codex.

Engine resolution uses `-EngineRoot` first, then `UE_5_8_ROOT`, then the Windows Epic Launcher installation manifest (`%ProgramData%\Epic\UnrealEngineLauncher\LauncherInstalled.dat`, exact `UE_5.8` entry). The selected engine must contain `Build.bat`, `UnrealEditor-Cmd.exe`, and a `Build.version` reporting major 5, minor 8. An invalid explicit or environment path fails instead of silently selecting another engine. To override discovery:

```powershell
.\Scripts\Validate-BertaDevKit.ps1 -EngineRoot 'D:\Epic Games\UE_5.8'
# Alternatively, set $env:UE_5_8_ROOT to your UE 5.8 installation.
```

The test filter joins `StartsWith:BertaDevKit.<branch>` for the plugin's current top-level test branches. A bare `StartsWith:BertaDevKit` would also select the optional sibling `BertaGASCompanionExt`, whose tests share that namespace. Update the runner's branch list if BertaDevKit adds a new top-level test branch.

Artifacts are written to `BertaDevKitHost/Saved/Validation/BertaDevKit/`: `Build.log` (also streamed to the console), `Automation.log`, and `Automation/index.json` plus exported report files. Each invocation clears only this runner's artifact directory before building, so an earlier report cannot produce a pass. Run one validation at a time; copy artifacts elsewhere before the next run if needed.

Exit code `0` means engine validation, build, and test validation all succeeded. Any setup/build/process failure, missing or invalid report, zero completed tests, failed tests, not-run tests, or tests still in process returns non-zero. Tests that succeed with warnings are counted in the summary and do not fail validation. A failed build never starts Automation Tests. Manual build commands and historical results remain below and in the [2026-09-28 validation record](validation-2026-09-28.md).

## Build the host

The primary Editor development target is:

```text
BertaDevKitHostEditor Win64 Development
```

For example:

```text
<UE_5.8>/Engine/Build/BatchFiles/Build.bat BertaDevKitHostEditor Win64 Development -Project="<repo>/BertaDevKitHost/BertaDevKitHost.uproject" -WaitMutex
```

Run verification appropriate to the change. A successful C++ build alone does not prove Editor, Blueprint, visual, runtime-device, or packaged behavior. See the [2026-09-28 validation record](validation-2026-09-28.md) for exact commands, results, and limitations.

The base build covers the seven enabled sibling plugins. Optional commercial integrations need their licensed third-party plugins and explicit enablement for separate compilation. GitHub Actions currently builds and publishes documentation only; hosted runners do not include UE 5.8 or those commercial plugins, so a green documentation workflow is not a C++ gate. Automation Tests require a separately authorized Unreal session to execute.

## Documentation site

Install the pinned documentation dependency and build the site from the repository root:

```text
python -m pip install -r requirements-docs.txt
mkdocs build --strict
```

For local authoring:

```text
mkdocs serve
```

The generated `site/` directory is ignored. GitHub Actions builds `site/` with the same strict command and deploys its artifact to GitHub Pages; it does not use a `gh-pages` branch.
