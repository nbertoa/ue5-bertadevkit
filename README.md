# BertaDevKit

**Personal Unreal Engine 5.8 development toolbox with native DualSense input, system information, external-process integration, runtime game-window control, desktop capture, and serial communication.**

This repository is a UE 5.8 development host for seven independent plugins plus optional GAS Companion and Combo Graph extensions. It is personal R&D tooling, not a gameplay framework or commercial product.

> **Documentation:** <https://nbertoa.github.io/ue5-bertadevkit/>

## Plugins

### BertaDevKit

A general-purpose UE 5.8 toolbox with Runtime Blueprint utilities for debug output and drawing, screen stats, math, world queries, UI helpers including fullscreen video playback and real-time fades, controller feedback/output, and a focused GAS + Behavior Tree bridge for reactive conditions, waits, actions, deterministic AI snapshots, and event-driven Blackboard/Gameplay Tag tracing. Its Editor module adds conservative Asset Naming, Asset Cleaner, Asset Insights, Blueprint Usage Finder, Project Setup, Blueprint Audit, World Validation, a read-only PIE Actor GC reference graph, a manual snapshot inspector for reflected dynamic multicast delegate bindings on a selected PIE Actor and its direct components (not arbitrary native C++ delegates), and a read-only manual Tick prerequisite graph seeded from that Actor's and its direct components' primary Tick Functions.

### BertaDualSense

A Win64 Runtime input-device plugin for native Sony DualSense and DualSense Edge controllers through SDL3. It provides standard Unreal gamepad input, touchpad and sensor input, selected DualSense outputs, and Edge-specific input keys.

### BertaSystemInfo

A read-only Runtime query plugin for operating system, CPU, GPU, memory, displays, display modes, and audio input/output device information. It uses UE 5.8 platform abstractions and exposes its own C++ and Blueprint types.

### BertaProcessBridge

A Runtime C++ and Blueprint bridge for launching an executable directly, receiving asynchronous text output, sending stdin, observing completion, and canceling it. Processes are owned by a `GameInstance` and cleaned up during teardown; the plugin does not implicitly invoke a shell.

### BertaWindowTools

A stateless Runtime C++ and Blueprint API for inspecting and controlling the current `GameInstance` game window: move, client resize, display centering, maximize/minimize/restore, focus/front requests, and title changes. It never enumerates or controls unrelated operating-system windows.

### BertaDesktopCapture

A Win64 Runtime C++ and Blueprint API for enumerating displays and visible top-level windows and capturing a selected source into a live transient `UTexture2D`. Each capture belongs to one `GameInstance`; the CPU-backed BGRA8 path is independent of Unreal's active RHI.

### BertaSerial

A Win64 Runtime C++ and Blueprint API for current COM-port enumeration, asynchronous raw-byte reception, ordered queued transmission, UTF-8 write helpers, explicit disconnect errors, and deterministic `GameInstance` cleanup.

### BertaGASCompanionExt

An optional Runtime/Editor extension for projects already using GAS Companion. It adds single-ability and actor-wide readiness diagnostics, evidence-qualified effective loadout snapshots, event-driven tracing, a conservative Ability Queue + Enhanced Input bridge, Ability Set/Game Feature contract validation, target diagnostics, and a native UE 5.8 Targeting System adapter. It is disabled by default and depends on both BertaDevKit and a local GAS Companion installation that is never committed.

### BertaComboGraphExt

An optional Runtime/Editor extension for projects already using Combo Graph `1.6.3+5.8`. It adds deterministic contract validation, bounded per-execution tracing and snapshots, EffectContext compatibility inspection, an opt-in multi-target UE Targeting System task, and conservative `Triggered` pre-input buffering. It is disabled by default and depends on a legitimate local Combo Graph installation that is never committed.

### BertaUltimateGameplayCameraExt

An optional Runtime component for cycling an ordered list of Ultimate Gameplay Camera data assets on a `PlayerController`, plus read-only Editor setup auditing and preset comparison. UGC owns camera evaluation and blending. The extension is disabled by default and requires a licensed local UGC installation that is never committed.

The seven base plugins are independent siblings. BertaGASCompanionExt, BertaComboGraphExt, and BertaUltimateGameplayCameraExt are explicit optional integration plugins.

## Requirements

- Unreal Engine 5.8
- A C++ toolchain supported by Unreal Engine 5.8
- Win64 for BertaDualSense
- Windows 10 version 1903 or later and Win64 for BertaDesktopCapture
- Win64 and a Windows COM-port device/driver for BertaSerial
- A licensed local GAS Companion installation for BertaGASCompanionExt
- A licensed local Combo Graph `1.6.3+5.8` installation for BertaComboGraphExt
- A licensed local Ultimate Gameplay Camera installation for BertaUltimateGameplayCameraExt
- Git LFS for the repository host

## Install in another project

Copy any plugin independently into the matching project plugin directory:

| Plugin | Copy from | Copy to |
| --- | --- | --- |
| BertaDevKit | `BertaDevKitHost/Plugins/BertaDevKit/` | `<YourProject>/Plugins/BertaDevKit/` |
| BertaDualSense | `BertaDevKitHost/Plugins/BertaDualSense/` | `<YourProject>/Plugins/BertaDualSense/` |
| BertaSystemInfo | `BertaDevKitHost/Plugins/BertaSystemInfo/` | `<YourProject>/Plugins/BertaSystemInfo/` |
| BertaProcessBridge | `BertaDevKitHost/Plugins/BertaProcessBridge/` | `<YourProject>/Plugins/BertaProcessBridge/` |
| BertaWindowTools | `BertaDevKitHost/Plugins/BertaWindowTools/` | `<YourProject>/Plugins/BertaWindowTools/` |
| BertaDesktopCapture | `BertaDevKitHost/Plugins/BertaDesktopCapture/` | `<YourProject>/Plugins/BertaDesktopCapture/` |
| BertaSerial | `BertaDevKitHost/Plugins/BertaSerial/` | `<YourProject>/Plugins/BertaSerial/` |
| BertaGASCompanionExt | `BertaDevKitHost/Plugins/BertaGASCompanionExt/` | `<YourProject>/Plugins/BertaGASCompanionExt/` |
| BertaComboGraphExt | `BertaDevKitHost/Plugins/BertaComboGraphExt/` | `<YourProject>/Plugins/BertaComboGraphExt/` |
| BertaUltimateGameplayCameraExt | `BertaDevKitHost/Plugins/BertaUltimateGameplayCameraExt/` | `<YourProject>/Plugins/BertaUltimateGameplayCameraExt/` |

Target UE 5.8, regenerate project files if needed, build, and enable the copied plugin in Unreal's Plugins window. BertaDualSense, BertaSystemInfo, BertaProcessBridge, BertaWindowTools, BertaDesktopCapture, BertaSerial, BertaGASCompanionExt, BertaComboGraphExt, and BertaUltimateGameplayCameraExt are disabled by default. The optional integration plugins additionally require their licensed third-party plugin. BertaDualSense stages SDL3 from its own directory; BertaSystemInfo enables UE's built-in Audio Capture plugin for microphone-device enumeration. BertaDesktopCapture and BertaSerial are Win64-only.

Copied-plugin guidance remains available in each plugin README, including [BertaGASCompanionExt](BertaDevKitHost/Plugins/BertaGASCompanionExt/README.md), [BertaComboGraphExt](BertaDevKitHost/Plugins/BertaComboGraphExt/README.md), and [BertaUltimateGameplayCameraExt](BertaDevKitHost/Plugins/BertaUltimateGameplayCameraExt/README.md).

## Development host

```text
ue5-bertadevkit/
├── AGENTS.md
├── README.md
├── docs/
└── BertaDevKitHost/
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
        └── BertaUltimateGameplayCameraExt/
```

`BertaDevKitHost` is the development and verification harness. The primary Editor target is:

```text
BertaDevKitHostEditor Win64 Development
```

For example:

```text
<UE_5.8>/Engine/Build/BatchFiles/Build.bat BertaDevKitHostEditor Win64 Development -Project="<repo>/BertaDevKitHost/BertaDevKitHost.uproject" -WaitMutex
```

See the [documentation site](https://nbertoa.github.io/ue5-bertadevkit/) for feature details, configuration, device behavior, and local documentation commands.

## Log categories

| Category | Scope |
| --- | --- |
| `LogBertaDevKit` | BertaDevKit Runtime utilities and systems |
| `LogBertaDebug` | BertaDevKit debug logging and drawing |
| `LogBertaDevKitEditor` | BertaDevKit Editor tooling and validation |
| `LogBertaDualSense` | BertaDualSense discovery, lifecycle, and SDL output failures |
| `LogBertaSystemInfo` | BertaSystemInfo Runtime query diagnostics |
| `LogBertaProcessBridge` | BertaProcessBridge launch and lifecycle diagnostics |
| `LogBertaWindowTools` | BertaWindowTools native window-operation diagnostics |
| `LogBertaDesktopCapture` | BertaDesktopCapture initialization and lifecycle failures |
| `LogBertaSerial` | BertaSerial open, configuration, worker, and cleanup failures |
| `LogBertaGASCompanionExt` | GAS Companion extension tracing, bridge, and targeting diagnostics |
| `LogBertaComboGraphExt` | Combo Graph extension tracing, buffering, targeting, and compatibility diagnostics |
| `LogBertaUltimateGameplayCameraExt` | UGC preset configuration and selection failures |

## About

Created by **Nicolás Bertoa** as a personal Unreal Engine R&D toolkit focused on C++, prototyping, tooling, and reusable development workflows.

[Portfolio](https://nbertoa.com/) · [Demo Reels](https://nbertoa.com/demo-reels/) · [GitHub](https://github.com/nbertoa)
