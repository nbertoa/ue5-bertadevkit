# BertaDevKit

**Personal Unreal Engine 5.8 toolkit for R&D, prototyping, debugging, editor automation, and everyday development.**

BertaDevKit collects small, reusable Unreal Engine utilities for debugging, Blueprint-friendly C++ helpers, world queries, math, asset tooling, validation, and Editor workflows. It is personal, UE 5.8-only tooling—not a gameplay framework or a general-purpose commercial product.

## Features

### Runtime

| System | Purpose |
| --- | --- |
| `UBertaDebugUtils` | Blueprint-friendly screen and Output Log messages with context, verbosity, categories, and per-call gating. |
| `UBertaDebugDraw` | Development debug drawing for common primitives, components, strings, coordinate systems, and persistent-shape flushing. |
| `UBertaScreenStats` | Named development screen stats for common value types; updating a name replaces its displayed value. |
| `UBertaMathUtils` | Remapping, easing, angular helpers, snapping, distributions, and lightweight prediction helpers. |
| `UBertaWorldUtils` | Actor queries, traces, player/camera access, and delayed-action timer helpers. |
| `UBertaUIUtils` | Blueprint conveniences for common UI/player input boilerplate. |

Debug-facing Blueprint nodes use Unreal's `DevelopmentOnly` metadata where appropriate. This signals intended development use; it is not a blanket claim about all Runtime code or runtime cost.

### Editor

**Asset Naming** audits BertaDevKit naming conventions and can apply a reviewed rename batch. It is available from the Tools menu and Content Browser context menus for selected assets or folders. It also participates in UE Data Validation. After a successful rename, redirectors created by that batch can be offered for cleanup through Unreal's native redirector workflow.

**Asset Cleaner** identifies conservative unused/orphan asset candidates. Its audit is read-only; cleanup revalidates candidates and opens Unreal's native deletion workflow. It never force-deletes assets.

**Project Setup** is an opt-in audit/apply utility for a curated allowlist of preferred project and per-project Editor defaults. It previews changes before applying them, manages Blueprint Assist and Electronic Nodes when installed, and reports missing managed plugins without failing the rest of the operation. Its external plugin toolbox entries are reminder-only: they are never audited, reported missing, or auto-enabled. It does not mutate projects at plugin startup.

The optional external tools available to Nicolás are documented in [Personal Plugin Toolbox](BertaDevKitHost/Plugins/BertaDevKit/Docs/PLUGIN_TOOLBOX.md). Consult it before recreating an overlapping specialized solution.

**Blueprint Audit** is a read-only, conservative static linter and code-review assistant for selected Blueprint assets or Content Browser folders. It reports static findings such as unused variables/functions, private or protected access reviews, single-function member reviews for possible localization, const and pure recommendations, and unused function inputs. It also reports advisory maintainability metrics and reviews for unusually large event graphs/functions/macros and high conditional-decision counts; these fixed thresholds are review heuristics, not correctness errors. Findings require manual review: static analysis cannot prove runtime/reflection use, a single-function member may intentionally retain state between calls, and making a function Pure can change evaluation timing and count. Blueprint findings appear in Output Log and in the native **BertaDevKit Blueprint Audit** Message Log; Blueprint asset tokens are clickable. Blueprint Audit has no Fix, Fix All, automatic refactoring, or graph-rewriting action.

**World Validation** checks the open Editor level against enabled project policy checks, including static-mesh assignment, configured world bounds, light mobility, and actor scale. It reports violations without changing actors.

### Editor access

The main Editor actions are under **Tools → BertaDevKit**, including Asset Naming audit/fix, World Validation, and the Project Setup audit/apply submenu. Content Browser right-click menus provide Asset Naming, Asset Cleaner, and Blueprint Audit submenus for project assets and folders.

## Architecture

BertaDevKit has a strict Runtime / Editor separation:

* `BertaDevKit` — Runtime utilities available to game code and Blueprints.
* `BertaDevKitEditor` — Editor-only menus, audits, automation, asset tooling, and validation.

Runtime never depends on the Editor module.

The repository contains a host project for development and verification:

```text
ue5-bertadevkit/
├── README.md
└── BertaDevKitHost/
    ├── BertaDevKitHost.uproject
    └── Plugins/
        └── BertaDevKit/             # Actual distributable plugin root
            ├── BertaDevKit.uplugin
            ├── Config/
            ├── Content/
            ├── Resources/
            └── Source/
                ├── BertaDevKit/
                └── BertaDevKitEditor/
```

`BertaDevKitHost` is the development/test harness. The distributable plugin starts at `BertaDevKitHost/Plugins/BertaDevKit/`.

## Configuration

Runtime plugin settings are under **Project Settings → Plugins → BertaDevKit** and persist in `Config/DefaultBertaDevKit.ini`. They configure debug systems and World Validation, including its individual checks.

Project Setup is separate: it is an explicit Editor tool that applies its own curated allowlist, rather than a collection of `UBertaDevKitSettings` options.

## Development

### Requirements

* Unreal Engine 5.8
* A C++ toolchain supported by Unreal Engine 5.8
* Git LFS

### Clone and build

```bash
git clone https://github.com/nbertoa/ue5-bertadevkit.git
```

Open `BertaDevKitHost/BertaDevKitHost.uproject` for the development host.

The primary Editor development target is:

```text
BertaDevKitHostEditor Win64 Development
```

For example:

```text
<UE_5.8>/Engine/Build/BatchFiles/Build.bat BertaDevKitHostEditor Win64 Development -Project="<repo>/BertaDevKitHost/BertaDevKitHost.uproject" -WaitMutex
```

Automation coverage includes Runtime math/world helpers and Editor Asset Naming/Asset Cleaner behavior. Run affected Automation Tests when appropriate. Editor, Blueprint, and visual changes also require manual verification in Unreal; a successful C++ build alone does not prove behavior.

## Using BertaDevKit in another project

Copy:

```text
BertaDevKitHost/Plugins/BertaDevKit/
```

into:

```text
<YourProject>/Plugins/BertaDevKit/
```

Then target UE 5.8, regenerate project files if needed, build, enable **BertaDevKit** in the Plugins window, and configure runtime settings under **Project Settings → Plugins → BertaDevKit**.

The repository root is a development host around the plugin; cloning the repository directly as `<YourProject>/Plugins/BertaDevKit` is not the intended installation layout.

## Log categories

| Category | Scope |
| --- | --- |
| `LogBertaDevKit` | Runtime utilities and systems |
| `LogBertaDebug` | Debug logging and drawing |
| `LogBertaDevKitEditor` | Editor tooling and validation |

## About

Created by **Nicolás Bertoa** as a personal Unreal Engine R&D toolkit focused on C++, prototyping, tooling, and reusable development workflows.

[Portfolio](https://nbertoa.wordpress.com) · [Demo Reels](https://nbertoa.wordpress.com/demo-reels/) · [GitHub](https://github.com/nbertoa)
