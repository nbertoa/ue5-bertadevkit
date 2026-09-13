# BertaDevKit

**Personal Unreal Engine 5.8 development toolbox with native DualSense input, system information, external-process integration, runtime game-window control, desktop capture, and serial communication.**

This repository is a UE 5.8 development host for seven independent sibling plugins. It is personal R&D tooling, not a gameplay framework or commercial product.

> **Documentation:** <https://nbertoa.github.io/ue5-bertadevkit/>

## Plugins

### BertaDevKit

A general-purpose UE 5.8 toolbox with Runtime Blueprint utilities for debug output and drawing, screen stats, math, world queries, UI helpers, and controller feedback/output. Its Editor module adds conservative Asset Naming, Asset Cleaner, Asset Insights, Blueprint Usage Finder, Project Setup, Blueprint Audit, and World Validation workflows.

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

The plugins are independent siblings: none is a module of or dependency of another.

## Requirements

- Unreal Engine 5.8
- A C++ toolchain supported by Unreal Engine 5.8
- Win64 for BertaDualSense
- Windows 10 version 1903 or later and Win64 for BertaDesktopCapture
- Win64 and a Windows COM-port device/driver for BertaSerial
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

Target UE 5.8, regenerate project files if needed, build, and enable the copied plugin in Unreal's Plugins window. BertaDualSense, BertaSystemInfo, BertaProcessBridge, BertaWindowTools, BertaDesktopCapture, and BertaSerial are disabled by default. BertaDualSense stages SDL3 from its own directory; BertaSystemInfo enables UE's built-in Audio Capture plugin for microphone-device enumeration. BertaDesktopCapture and BertaSerial are Win64-only.

Copied-plugin guidance remains available in the [BertaDualSense README](BertaDevKitHost/Plugins/BertaDualSense/README.md), [BertaSystemInfo README](BertaDevKitHost/Plugins/BertaSystemInfo/README.md), [BertaProcessBridge README](BertaDevKitHost/Plugins/BertaProcessBridge/README.md), [BertaWindowTools README](BertaDevKitHost/Plugins/BertaWindowTools/README.md), [BertaDesktopCapture README](BertaDevKitHost/Plugins/BertaDesktopCapture/README.md), and [BertaSerial README](BertaDevKitHost/Plugins/BertaSerial/README.md).

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
        └── BertaSerial/
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

## About

Created by **Nicolás Bertoa** as a personal Unreal Engine R&D toolkit focused on C++, prototyping, tooling, and reusable development workflows.

[Portfolio](https://nbertoa.wordpress.com) · [Demo Reels](https://nbertoa.wordpress.com/demo-reels/) · [GitHub](https://github.com/nbertoa)
