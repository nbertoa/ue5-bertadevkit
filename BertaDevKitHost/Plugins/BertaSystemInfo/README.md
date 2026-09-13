# BertaSystemInfo

BertaSystemInfo is an independent, read-only Unreal Engine 5.8 Runtime plugin for querying the operating system, CPU, GPU, memory, connected displays, available display modes, and audio input/output devices. It is stateless: every function queries current UE/platform state and does not cache devices, tick, poll, or register hotplug callbacks.

Complete technical documentation: <https://nbertoa.github.io/ue5-bertadevkit/bertasysteminfo/>

## Requirements and installation

- Unreal Engine 5.8
- A supported C++ toolchain when building from source

Copy this directory into a UE 5.8 project:

```text
BertaDevKitHost/Plugins/BertaSystemInfo/
→ <YourProject>/Plugins/BertaSystemInfo/
```

Build the project and enable **BertaSystemInfo**. The plugin is disabled by default and has no dependency on BertaDevKit or BertaDualSense. It explicitly enables Unreal's built-in **Audio Capture** plugin so `AudioCaptureCore` can discover the platform microphone backend; it does not open a capture stream or record audio.

## Public API

All functions are available from C++ and `UBertaSystemInfoBlueprintLibrary`.

| Area | Function | Result |
| --- | --- | --- |
| System | `GetOperatingSystemInfo` | `FBertaOperatingSystemInfo` with UE platform name, numeric OS version, version labels, and host architecture. |
| System | `GetCPUInfo` | `FBertaCPUInfo` with brand, vendor, physical cores, and logical cores. |
| System | `GetGPUInfo` | `FBertaGPUInfo` with best available adapter, RHI, driver, IDs, and dedicated-memory data. |
| System | `GetMemoryInfo` | `FBertaMemoryInfo` with current physical, virtual, and process memory byte counts. |
| Displays | `GetDisplays` | One `FBertaDisplayInfo` per monitor reported by `FDisplayMetrics`. |
| Displays | `GetPrimaryDisplay` | Finds the display marked primary by UE. |
| Displays | `GetAvailableDisplayModes` | Queries width, height, and refresh-rate combinations for one returned display ID. |
| Audio | `GetAudioInputDevices` | Current microphone/input devices reported by `AudioCaptureCore`. |
| Audio | `GetAudioOutputDevices` | Current output devices for the supplied world's active Audio Mixer device. |

## Data contracts

### Operating system and CPU

`PlatformName` is UE's current platform name. `Version` is the numeric/version-style value returned by the platform API; `VersionLabel` and `SubVersionLabel` are the platform's descriptive labels. CPU physical and logical core counts remain separate.

### GPU

`AdapterName` retains UE's native primary-GPU fallback when initialized RHI details are unavailable. `bHasRHIInfo` indicates that an initialized RHI supplied detailed adapter state. Driver strings can be empty or `Unknown`. `DedicatedVideoMemoryBytes == 0` means unknown or unreported, not necessarily zero VRAM.

### Memory

All memory properties are bytes. Total and available fields describe the values reported by `FPlatformMemory::GetStats`; `ProcessUsed*` and `ProcessPeakUsed*` are this process's usage, not whole-system used memory. Values exceeding Blueprint's signed `int64` range saturate at `MAX_int64`.

### Displays and modes

Display position and size come from UE desktop/work-area rectangles, so positions may be negative. `NativeResolution` and `MaxResolution` are exactly the values UE provides; a zero value means unavailable. In UE 5.8 on Windows, `MaxResolution` is not populated while running the Editor.

Display IDs are opaque lookup values returned by UE. Do not parse them or treat them as permanent identity across OS, driver, or hardware changes. `GetAvailableDisplayModes` resolves an exact ID, resets output before failure, preserves distinct refresh rates, removes exact duplicates, and sorts by width, height, then refresh rate. UE 5.8 only implements a truly per-monitor query for D3D11 and D3D12, so the function returns `false` for other RHI backends rather than silently returning global modes.

### Audio devices

Audio device IDs are opaque and should not be treated as global or permanent identity. `GetAudioInputDevices` performs fresh enumeration without opening or starting a microphone stream. No devices or an unavailable backend produces an empty array and is a normal result.

`GetAudioOutputDevices` requires a valid `WorldContextObject` and the world's active Audio Mixer backend. It returns `false` for an invalid context, unavailable audio device/mixer, or a failed platform query; a successful query may return an empty array. Current-device matching uses `DeviceId` when available, with a name fallback only when the backend does not expose usable IDs.

## Limits and verification status

- Read-only queries only: no display, window, monitor, audio-device, or HDR mutation.
- No microphone recording, playback, audio analysis, device switching, caching, polling, or hotplug events.
- Microphone recording is deliberately out of scope. Returned microphone information can be paired with a dedicated capture solution such as Runtime Audio Importer, but BertaSystemInfo has no dependency on it.
- No user, machine, login, network, MAC, installation, advertising, or fingerprinting identifiers.
- Win64 compilation is verified through the repository host. Runtime behavior on Win64 and all behavior on other platforms still require manual verification.
