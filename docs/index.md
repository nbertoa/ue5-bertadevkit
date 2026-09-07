# BertaDevKit

BertaDevKit is a personal Unreal Engine 5.8 repository for R&D, prototyping, debugging, tooling, and validation. It contains a development host with two independent sibling plugins:

- [BertaDevKit](bertadevkit/index.md) — Runtime Blueprint utilities and Editor development tools.
- [BertaDualSense](bertadualsense/index.md) — Native Sony DualSense and DualSense Edge input support for Win64 through SDL3.

Neither plugin is a module of or dependency of the other. Copy either plugin independently into a UE 5.8 project.

## Quick installation

| Plugin | Copy from | Copy to |
| --- | --- | --- |
| BertaDevKit | `BertaDevKitHost/Plugins/BertaDevKit/` | `<YourProject>/Plugins/BertaDevKit/` |
| BertaDualSense | `BertaDevKitHost/Plugins/BertaDualSense/` | `<YourProject>/Plugins/BertaDualSense/` |

Build the target project and enable the copied plugin in Unreal's Plugins window. BertaDualSense is Win64-only; BertaDevKit supports the UE 5.8 Runtime/Editor split described in its documentation.

See [Development](development/index.md) for the host project, build target, and documentation commands.
