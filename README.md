# BertaDevKit

**Personal Unreal Engine 5.8 development toolbox and native DualSense input plugin.**

This repository is a UE 5.8 development host for two independent sibling plugins. It is personal R&D tooling, not a gameplay framework or commercial product.

> **Documentation:** <https://nbertoa.github.io/ue5-bertadevkit/>

## Plugins

### BertaDevKit

A general-purpose UE 5.8 toolbox with Runtime Blueprint utilities for debug output and drawing, screen stats, math, world queries, and UI helpers. Its Editor module adds conservative Asset Naming, Asset Cleaner, Asset Insights, Project Setup, Blueprint Audit, and World Validation workflows.

### BertaDualSense

A Win64 Runtime input-device plugin for native Sony DualSense and DualSense Edge controllers through SDL3. It provides standard Unreal gamepad input, touchpad and sensor input, selected DualSense outputs, and Edge-specific input keys.

The plugins are siblings: neither is a module of, nor depends on, the other.

## Requirements

- Unreal Engine 5.8
- A C++ toolchain supported by Unreal Engine 5.8
- Win64 for BertaDualSense
- Git LFS for the repository host

## Install in another project

Copy either plugin independently into the matching project plugin directory:

| Plugin | Copy from | Copy to |
| --- | --- | --- |
| BertaDevKit | `BertaDevKitHost/Plugins/BertaDevKit/` | `<YourProject>/Plugins/BertaDevKit/` |
| BertaDualSense | `BertaDevKitHost/Plugins/BertaDualSense/` | `<YourProject>/Plugins/BertaDualSense/` |

Target UE 5.8, regenerate project files if needed, build, and enable the copied plugin in Unreal's Plugins window. BertaDualSense is disabled by default and stages its SDL3 dependency from its own plugin directory.

BertaDualSense's copied-plugin README remains available at [`BertaDevKitHost/Plugins/BertaDualSense/README.md`](BertaDevKitHost/Plugins/BertaDualSense/README.md). It includes hardware support, installation, and important Windows input-backend coexistence guidance.

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
        └── BertaDualSense/
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

## About

Created by **Nicolás Bertoa** as a personal Unreal Engine R&D toolkit focused on C++, prototyping, tooling, and reusable development workflows.

[Portfolio](https://nbertoa.wordpress.com) · [Demo Reels](https://nbertoa.wordpress.com/demo-reels/) · [GitHub](https://github.com/nbertoa)
