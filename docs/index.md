# BertaDevKit

BertaDevKit is a personal Unreal Engine 5.8 repository for R&D, prototyping, debugging, tooling, and validation. It contains a development host with four independent sibling plugins:

- [BertaDevKit](bertadevkit/index.md) — Runtime Blueprint utilities and Editor development tools.
- [BertaDualSense](bertadualsense/index.md) — Native Sony DualSense and DualSense Edge input support for Win64 through SDL3.
- [BertaSystemInfo](bertasysteminfo/index.md) — Read-only Runtime system, hardware, display, and audio-device queries.
- [BertaProcessBridge](bertaprocessbridge/index.md) — Runtime external-process integration with asynchronous output, stdin, completion, and cancellation.

None of the plugins is a module of or dependency of another. Copy any plugin independently into a UE 5.8 project.

## Quick installation

| Plugin | Copy from | Copy to |
| --- | --- | --- |
| BertaDevKit | `BertaDevKitHost/Plugins/BertaDevKit/` | `<YourProject>/Plugins/BertaDevKit/` |
| BertaDualSense | `BertaDevKitHost/Plugins/BertaDualSense/` | `<YourProject>/Plugins/BertaDualSense/` |
| BertaSystemInfo | `BertaDevKitHost/Plugins/BertaSystemInfo/` | `<YourProject>/Plugins/BertaSystemInfo/` |
| BertaProcessBridge | `BertaDevKitHost/Plugins/BertaProcessBridge/` | `<YourProject>/Plugins/BertaProcessBridge/` |

Build the target project and enable the copied plugin in Unreal's Plugins window. BertaDualSense is Win64-only; the other plugins have the UE 5.8 module boundaries and verification scope described in their documentation.

See [Development](development/index.md) for the host project, build target, and documentation commands.
