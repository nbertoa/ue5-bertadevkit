# Personal Plugin Toolbox

This is Nicolás's personal catalog of optional external Unreal Engine plugins and engineering tools available in his Fab library. It helps Nicolás and AI assistants avoid rebuilding a well-covered solution inside BertaDevKit. These are not BertaDevKit dependencies unless a future task explicitly decides otherwise.

When evaluating a new need:

1. Check UE 5.8 native functionality.
2. Check whether BertaDevKit already solves it.
3. Check this personal plugin toolbox.
4. Implement new BertaDevKit functionality only when a concrete reusable gap remains.

Do not introduce a hard dependency on a listed tool without explicit authorization.

## Managed by Project Setup

Project Setup may inspect installation and enablement only for the following plugins. No other external tool is managed, auto-enabled, or included in Project Setup audit results.

### Blueprint Assist

Use for Blueprint editing productivity, formatting, navigation, and graph editing. Project Setup manages these preferred settings:

- `Parameter Style = LeftSide`
- `Apply Comment Padding = false`
- `Auto Formatting = Never`

### Electronic Nodes

Use for graph visual readability. Project Setup manages enablement only; it does not assume additional settings.

## General Toolbox

### UForge — HTTP & JSON Blueprint Utility

Status: Purchased / Owned by Nicolás.

Use for Blueprint-heavy HTTP/REST, JSON, Blueprint Struct ↔ JSON, headers and payloads, file I/O, supported uploads/downloads and streaming, WebSockets, and Spotify/OpenAI/custom external APIs. Do not implement generic HTTP, JSON, or WebSocket infrastructure in BertaDevKit unless a concrete gap remains; for C++, native UE APIs may still be preferable.

- Fab: <https://www.fab.com/listings/0056a97e-71c9-4870-a82c-53fe1fce94b2>
- Docs: <https://doc.uforge.co.uk/>
- JSON docs: <https://doc.uforge.co.uk/json-pro>

Do not assume undocumented OAuth/PKCE/SSE abstractions.

### LE Extended Standard Library

Available in Nicolás's personal Fab library. Check it before building generic Blueprint helpers for strings/regex, arrays and sorting, bytes/encoding, clipboard, latent helpers, and other standard-library gaps.

- Fab: <https://www.fab.com/listings/0aadd41b-c02d-4f63-9009-bffad0070ebc>
- Source: <https://github.com/LowEntry/lowentry-ue-extendedstandardlibrary>

### Graph Printer — Naotsun

Available in Nicolás's personal Fab library. Use to export or copy supported Unreal graphs as images for documentation, reviews, sharing, and AI-assisted analysis; remember it before implementing graph screenshot/export tooling in BertaDevKit.

- Fab: <https://www.fab.com/listings/2f02025a-856d-475b-8043-d3f3b0d45ba8>

### GLS — Advanced Game Logging

Available in Nicolás's personal Fab library. Use for specialized gameplay/runtime logging and inspection; do not recreate GLS as a generic BertaDevKit logging UI without a concrete gap.

- Fab: <https://www.fab.com/listings/c558f5b4-0b5f-4342-acb4-c17461e541e8>

## Strong Conditional Toolbox

These tools are especially valuable when their corresponding subsystem is already in use.

### State Tree Tools — Lunar Workshop

Available in Nicolás's personal Fab library. When a project uses StateTree, check UE 5.8 native functionality and State Tree Tools before writing a custom reusable Task or Condition. It covers recurring integrations and actions around events/delegates, properties, callbacks, Actor/Component work, animation, Niagara, audio, debugging, Enhanced Input, UMG, GAS, and gameplay/AI events.

- Fab: <https://www.fab.com/listings/9ad18b3d-9a5d-4b95-ac3b-c7ba27ab9e35>

Reverify UE 5.8 compatibility before project adoption.

### PCG Extended Toolkit / PCGEx

Available in Nicolás's personal Fab library. Use for advanced PCG path and point processing, filtering, graph/cluster operations, procedural distributions, and utility workflows before creating custom PCG nodes unnecessarily.

- Fab: <https://www.fab.com/listings/3f0bea1c-7406-4441-951b-8b2ca155f624>

### GAS Companion

Available in Nicolás's personal Fab library. Use for GAS-heavy projects before duplicating GAS authoring or tooling workflows.

- Fab: <https://www.fab.com/listings/72e2bc50-658e-43cd-bd40-535f46d2f113>

### Combo Graph

Available in Nicolás's personal Fab library. Use for specialized combo and action-flow systems.

- Fab: <https://www.fab.com/listings/b0828da5-26a2-400a-b52d-2b9fd77c2bc6>

### Ninja Input

Available in Nicolás's personal Fab library. Consider it when Enhanced Input becomes complex, modular, or closely integrated with GAS.

- Fab: <https://www.fab.com/listings/13f49ddf-2d0c-41b7-939f-4008e364c2dc>

### Animation Tools Bundle — Attaku

Available in Nicolás's personal Fab library. Its `RM Fix Tool` and `ANIM MOD TOOL` cover root-motion fixes and animation modification workflows; check them before implementing custom editor utilities.

- Fab: <https://www.fab.com/listings/56d5121b-0acd-4096-82da-3ff7c178a530>

### Surface Forge

Available in Nicolás's personal Fab library. For substantial environment/material work, check it before building another large master-material framework for layered surfacing, terrain, Nanite/displacement, POM, snow/puddles, decals, RVT, triplanar/world-aligned workflows, or vertex painting.

- Fab: <https://www.fab.com/listings/9d27e228-5ee3-4bb5-a4a2-87ba386cb53a>

### Runtime Audio Importer

Available in Nicolás's personal Fab library. Check it and UE 5.8 native audio APIs before building runtime external-audio import, decoding/transcoding, streaming, microphone capture, or voice/audio ingestion infrastructure.

- Fab: <https://www.fab.com/listings/66e0d72e-982f-4d9e-aaaf-13a1d22efad1>

## Specialized Systems — Lookup When Needed

These are reminder-only lookup entries, not Project Setup reminders or auto-enabled plugins.

### ProjectCleaner

Use as a specialized cleanup/reference tool when BertaDevKit Asset Cleaner does not cover the required workflow; BertaDevKit remains preferred for its implemented use cases. [Fab](https://www.fab.com/listings/d9892568-d368-4332-bc42-2421d0151949)

### Hyper Attribute Manager v4

Consider for a lightweight general attribute system when GAS would be excessive; do not replace a chosen GAS/GAS Companion architecture. [Fab](https://www.fab.com/listings/e0e9cad6-cfdf-48d7-bce1-8883ed8c31e9)

### Hyper Outliner and Symbol System v4

Use for gameplay outlines, symbols, and markers around interactables, enemies, or objectives—not primarily World Outliner organization. [Fab](https://www.fab.com/listings/640d4c6c-4142-419f-804e-91a8efd61e84)

### Auto Size Comments

Use if Blueprint graph comment resizing or presets become friction; do not duplicate it in BertaDevKit without a concrete gap. [Fab](https://www.fab.com/listings/fdb7e77d-be37-4feb-a6c9-60e317c10adf)

### Fluid Ninja VFX Tools

Use for specialized fluid/VFX work such as fluid simulation, flow maps, volumetrics, and Niagara workflows; reverify UE 5.8 compatibility before project adoption. [Fab](https://www.fab.com/listings/90266972-0597-4404-a54a-8c0b7e00a005)

### Voyager: Cover System

Remember when a project needs an actual cover system instead of writing one from scratch. [Fab](https://www.fab.com/listings/122a897d-af7e-4e07-bad8-327d95569900)

### Advanced Grid Inventory System

Remember when a project needs grid-based inventory. [Fab](https://www.fab.com/listings/16b82fb0-7ea5-4627-adcc-95f23a387b61)

### Ultimate Difficulty Scaling

Remember when a project needs configurable or dynamic difficulty scaling.

### Ultimate Interaction Manager

Remember when a project needs a reusable interaction framework.

### Customizable Interaction Plugin

An alternative interaction-system option already available in the library.

### Quest Editor Plugin

Remember before building a custom quest editor or system. [Fab](https://www.fab.com/listings/0fac6d97-ed85-4f38-86ae-bea5e828f045)

### Universal Camera

Remember when a project needs a reusable camera framework beyond straightforward native camera setup. [Fab](https://www.fab.com/listings/5a3a096c-787f-4cb5-9d1c-98d847ec5d76)

### LXR — Light Detection

Remember for gameplay systems that need light or exposure detection. [Fab](https://www.fab.com/listings/5ac2e509-9918-4eeb-90f8-f257e5cd230f)

### Did it hit — Trace Detection Plugin

Remember for specialized trace/hit-detection workflows. [Fab](https://www.fab.com/listings/2c6d8470-b6b9-43c8-b607-b9fce26d7df4)

### Advanced Universal Spawner

Remember for reusable spawning workflows before building another spawning framework. [Fab](https://www.fab.com/listings/c1ea4a26-ea15-448a-9650-5ba91caa52bc)

### Advanced AI Spawn System

Remember for AI-specific spawning systems. [Fab](https://www.fab.com/listings/7f630cee-a5e1-4ab6-85cd-c111c57eda8f)

### Advanced Flock System

Remember for flocking or schooling behavior, especially fish and large reactive groups. [Fab](https://www.fab.com/listings/70626312-656b-4037-8e69-4cba921d04d4)

### Crowd Simulation System Pro

Remember for large NPC/crowd simulation before building custom crowd infrastructure. [Fab](https://www.fab.com/listings/6212ccdd-1352-44e8-a545-7646042f0887)

### Physical Interaction System

Remember for physics-heavy object-interaction workflows. [Fab](https://www.fab.com/listings/2365e7ba-5d8d-4f0e-904f-84ba15d23421)

### Replicated Grab System

Remember for networked grab/hold interactions. [Fab](https://www.fab.com/listings/a9d62d71-8976-4055-99f6-3eef67e9d1db)

### Easy Combo Buffering

A lightweight combo/input-buffer option; evaluate Combo Graph first when it is already appropriate. [Fab](https://www.fab.com/listings/be3fdfd2-34bf-4c1c-8873-21f941c6511b)

### MySQL and MariaDB Integration

Remember only when a concrete project genuinely needs direct database integration. [Fab](https://www.fab.com/listings/1454a193-0e4b-49f9-a9af-591b37a0f18a)

### AzSpeech — Voice and Text

Remember when speech, voice, or text-service integration is needed. [Fab](https://www.fab.com/listings/2b5f3edc-2d9a-4da2-b956-75e6c53d8b91)

### DLC in Blueprints

Remember for downloadable-content workflows when a project actually needs DLC handling. [Fab](https://www.fab.com/listings/600aea4e-c67c-43a3-8312-c11f02df52fb)

## Evaluated / Usually Prefer Something Else

Ownership does not make these the default recommendation.

### Debug Logging Library

Available but usually redundant because Nicolás already has BertaDevKit debugging/logging and GLS; do not recommend first without a concrete unique advantage. [Fab](https://www.fab.com/listings/743a3052-9868-407f-9464-6a4dc154f43e)

### VaRest

Available, but UForge is Nicolás's preferred Blueprint HTTP/JSON solution; do not recommend VaRest first for new work.

### HttpGPT

Available, but UForge is the more general external-API foundation. Prefer UForge unless HttpGPT has a concrete OpenAI-specific workflow that materially saves work. [Fab](https://www.fab.com/listings/3edf406f-6a87-4f2f-bfdb-b0039f285541)

### Plugin Builder

Available, but its Marketplace/multi-engine packaging workflow is not central to UE 5.8-only, personal-first BertaDevKit. Keep it as a lookup option if distribution requirements change. [Fab](https://www.fab.com/listings/5f5b357f-c6c0-4466-b616-25db02071c8d)

### Visual Studio Integration Tool

Do not treat as a meaningful recommendation: Nicolás's current C++ workflow uses Rider/Codex.
