# BertaDevKit

**Personal Unreal Engine 5.8 development toolbox and native DualSense input plugin.**

This repository is a UE 5.8 development host for two independent plugins. It is personal R&D tooling, not a gameplay framework or commercial product.

## Plugins

### BertaDevKit

A general-purpose UE 5.8 toolbox with Runtime Blueprint utilities and Editor tools for debugging, drawing, screen stats, math, world queries, asset workflows, validation, and project setup.

### BertaDualSense

A Win64 Runtime input-device plugin for native Sony DualSense and DualSense Edge controllers through SDL3. It delivers standard Unreal gamepad input, touchpad and sensor input, DualSense outputs, and Edge-specific keys. See the [BertaDualSense README](BertaDevKitHost/Plugins/BertaDualSense/README.md) for hardware support, installation, input keys, outputs, and coexistence guidance.

The plugins are siblings: neither is a module of, nor depends on, the other.

## BertaDevKit features

### Runtime

| System | Purpose |
| --- | --- |
| `UBertaDebugUtils` | Blueprint-friendly screen and Output Log messages with context, verbosity, categories, and per-call gating. |
| `UBertaDebugDraw` | Development debug drawing for common primitives, components, strings, coordinate systems, and persistent-shape flushing. |
| `UBertaScreenStats` | Named development screen stats for common value types; updating a name replaces its displayed value. |
| `UBertaMathUtils` | Remapping, easing, angular helpers, snapping, distributions, and lightweight prediction helpers. |
| `UBertaWorldUtils` | Actor queries, traces, player/camera access, and delayed-action timer helpers. |
| `UBertaUIUtils` | Blueprint conveniences for common UI/player-input boilerplate. |

Debug-facing Blueprint nodes use Unreal's `DevelopmentOnly` metadata where appropriate. This signals intended development use; it is not a blanket claim about all Runtime code or runtime cost.

### Editor

**Asset Naming** audits BertaDevKit naming conventions and can apply a reviewed rename batch. It is available from the Tools menu and Content Browser context menus for selected assets or folders. It also participates in UE Data Validation.

**Asset Cleaner** identifies conservative unused/orphan asset candidates. Its audit is read-only; cleanup revalidates candidates and opens Unreal's native deletion workflow. It can remove safely revalidated empty project Content folders, but never force-deletes assets.

**Asset Insights** is a read-only Content Browser analysis for selected assets. It reports Saved Package Size, direct on-disk dependency/referencer counts, Texture2D and StaticMesh metrics, and conservative footprint reviews. It complements Unreal's Size Map, Reference Viewer, and Asset Audit.

**Project Setup** is an opt-in audit/apply utility for a curated allowlist of preferred project and per-project Editor defaults. It previews changes before applying them, manages Blueprint Assist and Electronic Nodes when installed, and does not mutate projects at plugin startup.

**Blueprint Audit** is a read-only, conservative static linter and code-review assistant for selected Blueprint assets or Content Browser folders. Findings require manual review and it has no Fix, Fix All, automatic refactoring, or graph-rewriting action.

**World Validation** checks the open Editor level against enabled project policy checks, including static-mesh assignment, configured world bounds, light mobility, and actor scale. It reports violations without changing actors.

The optional external tools available to Nicolás are documented in [Personal Plugin Toolbox](BertaDevKitHost/Plugins/BertaDevKit/Docs/PLUGIN_TOOLBOX.md).

## Architecture

BertaDevKit has a strict Runtime / Editor separation:

- `BertaDevKit` — Runtime utilities available to game code and Blueprints.
- `BertaDevKitEditor` — Editor-only menus, audits, automation, asset tooling, and validation.

Runtime never depends on the Editor module. BertaDualSense is a separate Runtime plugin with its own SDL3 dependency and no dependency on BertaDevKit.

```text
ue5-bertadevkit/
├── AGENTS.md
├── README.md
└── BertaDevKitHost/
    ├── BertaDevKitHost.uproject
    ├── Content/
    └── Plugins/
        ├── BertaDevKit/
        │   ├── BertaDevKit.uplugin
        │   └── Source/
        │       ├── BertaDevKit/
        │       └── BertaDevKitEditor/
        └── BertaDualSense/
            ├── BertaDualSense.uplugin
            ├── README.md
            └── Source/
```

`BertaDevKitHost` is the development and verification harness. Each directory under `Plugins` is independently distributable.

## Configuration

BertaDevKit Runtime settings are under **Project Settings → Plugins → BertaDevKit** and persist in `Config/DefaultBertaDevKit.ini`. Project Setup is a separate explicit Editor tool with its own curated allowlist.

BertaDualSense has no settings object. Its installation and Windows input-backend coexistence are documented in its [plugin README](BertaDevKitHost/Plugins/BertaDualSense/README.md).

## Development

### Requirements

- Unreal Engine 5.8
- A C++ toolchain supported by Unreal Engine 5.8
- Git LFS for the repository host
- Win64 for BertaDualSense

### Clone and build

```bash
git clone https://github.com/nbertoa/ue5-bertadevkit.git
```

Open `BertaDevKitHost/BertaDevKitHost.uproject` for the development host. The primary Editor development target is:

```text
BertaDevKitHostEditor Win64 Development
```

For example:

```text
<UE_5.8>/Engine/Build/BatchFiles/Build.bat BertaDevKitHostEditor Win64 Development -Project="<repo>/BertaDevKitHost/BertaDevKitHost.uproject" -WaitMutex
```

Run affected Automation Tests when appropriate. Editor, Blueprint, and visual changes also require manual Unreal verification; a successful C++ build alone does not prove behavior.

## Installing in another project

Copy either plugin independently into the matching project plugin directory:

| Plugin | Copy from | Copy to |
| --- | --- | --- |
| BertaDevKit | `BertaDevKitHost/Plugins/BertaDevKit/` | `<YourProject>/Plugins/BertaDevKit/` |
| BertaDualSense | `BertaDevKitHost/Plugins/BertaDualSense/` | `<YourProject>/Plugins/BertaDualSense/` |

Target UE 5.8, regenerate project files if needed, build, and enable the plugin in Unreal's Plugins window. BertaDualSense is Win64-only and stages its vendored SDL3 dependency from its own plugin directory. The repository root is a development host, so copying the repository itself into a project plugin directory is not the intended layout.

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
