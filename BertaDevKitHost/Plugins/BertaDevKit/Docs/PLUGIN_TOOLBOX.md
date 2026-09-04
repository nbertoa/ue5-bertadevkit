# Personal Plugin Toolbox

This is Nicolás's personal catalog of optional external Unreal Engine plugins. It helps projects and AI assistants avoid rebuilding a well-covered solution inside BertaDevKit. These plugins are not BertaDevKit dependencies unless a future task explicitly decides otherwise.

When evaluating a new need:

1. Check UE 5.8 native functionality.
2. Check whether BertaDevKit already solves it.
3. Check this personal plugin toolbox.
4. Implement new BertaDevKit functionality only when a concrete reusable gap remains.

Do not introduce a hard dependency on a listed plugin without explicit authorization.

## Managed by Project Setup

Project Setup may inspect installation and enablement for these plugins. Only Blueprint Assist has managed preferred settings.

### Blueprint Assist

Use for Blueprint graph productivity, formatting, navigation, and reducing graph-editing friction. Project Setup manages these preferred settings:

- `Parameter Style = LeftSide`
- `Apply Comment Padding = false`
- `Auto Formatting = Never`

### Electronic Nodes

Plugin ID: `ElectronicNodes`.

Use for Blueprint, material, and other graph visual readability, including cleaner node connections. Project Setup may audit or enable it when installed; it does not assume additional Electronic Nodes settings.

## Reminder-only toolbox

These entries are suggestions, not Project Setup audits. They must never be reported missing, auto-enabled, or counted as a Project Setup change.

### LE Extended Standard Library

Use for generic Blueprint utility extensions such as regex, array/object/struct helpers, encoding and bytes, clipboard access, latent actions, and other Blueprint gaps. Before adding a generic Blueprint helper to BertaDevKit, check UE 5.8 native functionality and LE first. LE remains optional.

- Source: <https://github.com/LowEntry/lowentry-ue-extendedstandardlibrary>
- Fab: <https://www.fab.com/listings/0aadd41b-c02d-4f63-9009-bffad0070ebc>

### UForge — HTTP & JSON Blueprint Utility

Status: Owned by Nicolás.

Use for Blueprint-heavy R&D involving HTTP/REST (`GET`, `POST`, `PUT`, `PATCH`, `DELETE`), request headers and bodies, JSON parsing/building and field access, Blueprint Struct ↔ JSON, file I/O, supported uploads/downloads and streaming workflows, WebSockets, and external services such as Spotify, OpenAI/ChatGPT, or custom APIs.

Do not implement a generic HTTP, JSON, or WebSocket subsystem in BertaDevKit unless a demonstrated gap remains after considering UE 5.8 native APIs and UForge. For C++, UE native HTTP/JSON/WebSocket APIs may still be preferable to adding a plugin dependency.

- Docs: <https://doc.uforge.co.uk/>
- JSON docs: <https://doc.uforge.co.uk/json-pro>

Do not assume undocumented OAuth/PKCE/SSE abstractions.

### GAS Companion

Use for projects using Gameplay Ability System and its specialized workflows/tooling. This is project-specific and remains reminder-only.

### Combo Graph

Use for combo/action flow systems where a specialized combo graph is appropriate. This is project-specific and remains reminder-only.

### PCG Extended Toolkit / PCGEx

Historically used plugin ID: `PCGExtendedToolkit`.

Use for PCG projects that need advanced procedural-generation workflows such as mesh collections, filters, paths, or distribution. Do not enable it for a project that does not use PCG.

### GLS — Game Logging System

Use for specialized runtime/development logging, structured log inspection/filtering, and gameplay debugging or observability. Do not recreate GLS inside BertaDevKit without a concrete gap.

### Ninja Input

Use for Enhanced Input-related workflows and input tooling when useful for a specific project. It remains reminder-only.

### Animation Tools Bundle — Attaku

Use this bundle as one reminder-only toolbox entry for animation editing workflows.

- Fab bundle: <https://www.fab.com/listings/56d5121b-0acd-4096-82da-3ff7c178a530>
- **RM Fix Tool:** create, remove, or fix root motion; root cleanup, direction correction, offsets, root/pelvis transfer, IK bone snapping, and root-motion cleanup. Fab: <https://www.fab.com/listings/2aba8225-dcf7-4425-b171-b11f17cafae7>
- **ANIM MOD TOOL:** reverse, mirror, loop, blend animations or static poses, manipulate time, and repeat or remove frames.
