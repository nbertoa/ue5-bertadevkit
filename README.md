# BertaDevKit

**Personal Unreal Engine 5.8 toolkit for R&D, prototyping, debugging, editor automation, and everyday development.**

BertaDevKit collects reusable Unreal Engine utilities that remove recurring friction from development: debugging helpers, Blueprint-friendly C++ libraries, world queries, math utilities, asset tooling, and editor validation.

The goal is not to become a gameplay framework. The goal is to turn repeated development work into small, maintainable tools that can be reused across projects.

> **Current engine target:** Unreal Engine 5.8 only.

## What Belongs in BertaDevKit

A utility is a good fit when it:

* removes repeated boilerplate or setup;
* improves debugging, visualization, validation, or prototyping;
* is reusable without knowledge of a specific game;
* has a clear responsibility and is cheaper to maintain here than as a separate plugin.

BertaDevKit intentionally avoids speculative abstractions, project-specific gameplay systems, and compatibility layers for older Unreal versions.

## Features

### Runtime

| System              | Purpose                                                                                                                                     |
| ------------------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| `UBertaDebugUtils`  | Blueprint-friendly logging to screen and/or Output Log, with context, verbosity, named categories, and per-call gating.                     |
| `UBertaDebugDraw`   | Debug spheres, lines, boxes, capsules, points, arrows, strings, coordinate systems, component-based helpers, and persistent-shape flushing. |
| `UBertaScreenStats` | Named on-screen development stats for floats, ints, bools, strings, and vectors. Updating the same name replaces the existing value.        |
| `UBertaMathUtils`   | Remapping, easing, angular helpers, snapping, point distribution, and lightweight prediction utilities.                                     |
| `UBertaWorldUtils`  | Actor queries, traces, player/camera access, and delayed-action timer helpers.                                                              |

Debug-facing Blueprint APIs use Unreal's `DevelopmentOnly` metadata where appropriate.

### Editor

| System                 | Purpose                                                                                                                                                      |
| ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Asset Naming           | Audits asset naming conventions, reports violations through native UE Data Validation, and can rename them using BertaDevKit prefix rules.                    |
| Asset Scope Resolution | Operates on selected assets first, then the active Content Browser folder, then `/Game` as fallback.                                                         |
| Editor Tools Menu      | Provides **Run Asset Audit**, **Fix Asset Naming**, and **Run World Validation** under the Unreal Editor Tools menu.                                         |
| World Validation       | Checks the currently open level for configurable issues such as missing Static Mesh assets, world-bound violations, light mobility, and invalid actor scale. |

Asset naming actions are also exposed through the plugin's Editor Utility asset for Content Browser workflows.

## Architecture

BertaDevKit has a strict Runtime / Editor separation:

* `BertaDevKit` — Runtime utilities that may be used by game code and Blueprints.
* `BertaDevKitEditor` — Editor-only automation, asset tooling, menus, and validation.

Runtime never depends on the Editor module.

The repository also contains a host project used to develop and test the plugin:

```text
ue5-bertadevkit/
├── README.md
└── BertaDevKitHost/
    ├── BertaDevKitHost.uproject
    ├── Content/                     # Test assets and DebugMap
    └── Plugins/
        └── BertaDevKit/             # Actual plugin root
            ├── BertaDevKit.uplugin
            ├── Config/
            ├── Content/
            ├── Resources/
            └── Source/
                ├── BertaDevKit/
                └── BertaDevKitEditor/
```

`BertaDevKitHost` is the development/test harness. The distributable plugin itself starts at:

```text
BertaDevKitHost/Plugins/BertaDevKit/
```

## Configuration

Plugin settings are available under:

**Project Settings → Plugins → BertaDevKit**

Current settings include master switches for:

* Debug Log
* Debug Draw
* Screen Stats
* World Validation

World Validation also exposes configuration for its individual checks.

Settings are stored in:

```text
Config/DefaultBertaDevKit.ini
```

## Development

### Requirements

* Unreal Engine 5.8
* A C++ toolchain supported by Unreal Engine 5.8
* Git LFS

### Clone the Repository

```bash
git clone https://github.com/nbertoa/ue5-bertadevkit.git
```

Open:

```text
BertaDevKitHost/BertaDevKitHost.uproject
```

The host project currently enables `BlueprintAssist` and `ElectronicNodes` for local development. They are **not BertaDevKit dependencies**. Disable those entries in `BertaDevKitHost.uproject` if they are not installed on your machine.

### Build

The primary editor development target is:

```text
BertaDevKitHostEditor Win64 Development
```

For example:

```text
<UE_5.8>/Engine/Build/BatchFiles/Build.bat BertaDevKitHostEditor Win64 Development -Project="<repo>/BertaDevKitHost/BertaDevKitHost.uproject" -WaitMutex
```

### Tests

Development Automation Tests include the `BertaDevKit.Math.*` and `BertaDevKit.AssetNaming.*` suites.

For editor and Blueprint-facing functionality, the host project contains:

```text
/Game/Maps/DebugMap
```

Use it for smoke testing logging, debug drawing, and editor tooling when those systems change.

## Using BertaDevKit in Another Project

Copy:

```text
BertaDevKitHost/Plugins/BertaDevKit/
```

into:

```text
<YourProject>/Plugins/BertaDevKit/
```

Then:

1. make sure the project targets Unreal Engine 5.8;
2. regenerate project files when required;
3. build the project;
4. enable **BertaDevKit** in the Plugins window;
5. configure it under **Project Settings → Plugins → BertaDevKit**.

The repository root is a development host around the plugin, so cloning the entire repository directly as `<YourProject>/Plugins/BertaDevKit` is not the intended installation layout.

## Verification Standard

For meaningful plugin changes, the expected baseline is:

1. build `BertaDevKitHostEditor Win64 Development`;
2. run the affected Automation Tests;
3. smoke test `DebugMap` when Blueprint, visual, or Editor behavior changes;
4. review new warnings and module dependencies;
5. inspect the complete diff before integration.

A successful compile alone is not treated as proof of behavioral correctness.

## Design Principles

BertaDevKit favors:

* small, cohesive APIs;
* explicit ownership and lifecycle;
* C++ for stable contracts and implementation;
* Blueprint for composition and iteration;
* minimal Runtime dependencies;
* safe Editor / Runtime boundaries;
* reusable tools backed by real development needs.

It deliberately does not aim to replace specialized plugins or grow into a monolithic gameplay framework.

## Log Categories

| Category               | Scope                         |
| ---------------------- | ----------------------------- |
| `LogBertaDevKit`       | Runtime utilities and systems |
| `LogBertaDebug`        | Debug logging and drawing     |
| `LogBertaDevKitEditor` | Editor tooling and validation |

## About

Created by **Nicolás Bertoa** as a personal Unreal Engine R&D toolkit focused on C++, prototyping, tooling, and reusable development workflows.

[Portfolio](https://nbertoa.wordpress.com) · [Demo Reels](https://nbertoa.wordpress.com/demo-reels/) · [GitHub](https://github.com/nbertoa)
