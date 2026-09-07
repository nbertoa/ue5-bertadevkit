# BertaDevKit

BertaDevKit is a general-purpose Unreal Engine 5.8 toolbox. Its Runtime module provides reusable Blueprint-facing utilities, while its Editor module provides conservative tooling for development workflows.

## Runtime utilities

| System | Purpose |
| --- | --- |
| `UBertaDebugUtils` | Blueprint-friendly screen and Output Log messages with context, verbosity, categories, and per-call gating. |
| `UBertaDebugDraw` | Development debug drawing for common primitives, components, strings, coordinate systems, and persistent-shape flushing. |
| `UBertaScreenStats` | Named development screen stats for common value types; updating a name replaces its displayed value. |
| `UBertaMathUtils` | Remapping, easing, angular helpers, snapping, distributions, and lightweight prediction helpers. |
| `UBertaWorldUtils` | Actor queries, traces, player/camera access, and delayed-action timer helpers. |
| `UBertaUIUtils` | Blueprint conveniences for common UI/player-input boilerplate. |

Debug-facing Blueprint nodes use Unreal's `DevelopmentOnly` metadata where appropriate. This signals intended development use; it is not a blanket claim about all Runtime code or runtime cost.

## Editor tools

- **Asset Naming** audits naming conventions and can apply a reviewed rename batch. It also participates in UE Data Validation.
- **Asset Cleaner** identifies conservative unused/orphan asset candidates. Its audit is read-only; cleanup revalidates candidates and opens Unreal's native deletion workflow.
- **Asset Insights** is a read-only Content Browser analysis with saved-package, dependency/referencer, Texture2D, StaticMesh, and conservative footprint information.
- **Project Setup** is an opt-in audit/apply utility for a curated allowlist of preferred project and per-project Editor defaults. It previews changes and does not mutate projects at startup.
- **Blueprint Audit** is a read-only, conservative static linter and review assistant. Findings require manual review; it has no automatic graph rewriting or refactoring action.
- **World Validation** checks the open Editor level against enabled policy checks and reports violations without changing actors.

The main Editor actions are under **Tools → BertaDevKit**. Content Browser context menus provide Asset Naming, Asset Cleaner, Blueprint Audit, and Asset Insights actions for selected project assets and folders.

## Architecture and configuration

BertaDevKit maintains a strict module boundary:

- `BertaDevKit` is the Runtime module available to game code and Blueprints.
- `BertaDevKitEditor` is the Editor-only module for menus, audits, automation, asset tooling, and validation.

The Runtime module never depends on the Editor module. Runtime settings are under **Project Settings → Plugins → BertaDevKit** and persist in `Config/DefaultBertaDevKit.ini`.

The personal external-plugin catalog remains separate in [`PLUGIN_TOOLBOX.md`](https://github.com/nbertoa/ue5-bertadevkit/blob/main/BertaDevKitHost/Plugins/BertaDevKit/Docs/PLUGIN_TOOLBOX.md); it is not part of this site's primary navigation and is not a BertaDevKit dependency.

## Installation

Copy `BertaDevKitHost/Plugins/BertaDevKit/` to `<YourProject>/Plugins/BertaDevKit/`, target UE 5.8, regenerate project files if needed, build, and enable **BertaDevKit**. The repository root is a development host, not the distributable plugin directory.

## Log categories

| Category | Scope |
| --- | --- |
| `LogBertaDevKit` | Runtime utilities and systems |
| `LogBertaDebug` | Debug logging and drawing |
| `LogBertaDevKitEditor` | Editor tooling and validation |
