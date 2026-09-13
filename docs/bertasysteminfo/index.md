# BertaSystemInfo

BertaSystemInfo is an independent Unreal Engine 5.8 Runtime plugin that answers: “What hardware, devices, and environment are available right now?” It exposes read-only C++ and Blueprint queries and keeps no subsystem, settings, tick, polling loop, cache, or persistent callback.

It does not depend on BertaDevKit or BertaDualSense.

## Installation

Copy the plugin into a UE 5.8 project, build, and enable it:

```text
BertaDevKitHost/Plugins/BertaSystemInfo/
→ <YourProject>/Plugins/BertaSystemInfo/
```

BertaSystemInfo is disabled by default. Its descriptor enables Unreal's built-in **Audio Capture** plugin because `AudioCaptureCore` obtains the platform microphone implementation from the backend modules loaded by that plugin. BertaSystemInfo itself never opens or starts a capture stream.

## Blueprint and C++ API

The public entry point is `UBertaSystemInfoBlueprintLibrary`.

| Category | Function | Type or behavior |
| --- | --- | --- |
| System | `GetOperatingSystemInfo` | Pure; returns `FBertaOperatingSystemInfo`. |
| System | `GetCPUInfo` | Pure; returns `FBertaCPUInfo`. |
| System | `GetGPUInfo` | Pure; returns `FBertaGPUInfo`; detailed fields may be partial before RHI initialization. |
| System | `GetMemoryInfo` | Callable because available/process memory changes at runtime. |
| Displays | `GetDisplays` | Rebuilds `FDisplayMetrics` and returns current monitors. |
| Displays | `GetPrimaryDisplay` | Returns `Found` and resets output on failure. |
| Displays | `GetAvailableDisplayModes` | Exact display-ID lookup and per-monitor RHI mode query. |
| Audio | `GetAudioInputDevices` | Fresh microphone-device enumeration without recording. |
| Audio | `GetAudioOutputDevices` | Synchronous query against the supplied world's active Audio Mixer device. |

## Public types and semantics

### `FBertaOperatingSystemInfo`

| Field | Meaning |
| --- | --- |
| `PlatformName` | Current UE platform name. |
| `Version` | Numeric/version-style OS value from UE. |
| `VersionLabel`, `SubVersionLabel` | Platform-provided descriptive OS labels. |
| `Architecture` | Host architecture reported by UE. |

### `FBertaCPUInfo`

`Brand` and `Vendor` come from `FPlatformMisc`. `PhysicalCoreCount` and `LogicalCoreCount` deliberately preserve UE's physical/logical distinction.

### `FBertaGPUInfo`

`AdapterName` uses `FPlatformMisc::GetPrimaryGPUBrand` as a fallback and initialized `FRHIGlobals::GpuInfo` when available. `ProviderName`, driver version, and driver date use UE's `FGPUDriverInfo`/RHI data and may be empty or `Unknown` where the platform does not report them. `bHasRHIInfo` indicates detailed initialized RHI state. `DedicatedVideoMemoryBytes == 0` means unknown or not reported, not necessarily zero VRAM.

### `FBertaMemoryInfo`

All values are bytes. `TotalPhysicalBytes`, `AvailablePhysicalBytes`, `TotalVirtualBytes`, and `AvailableVirtualBytes` are UE platform-memory values. `ProcessUsedPhysicalBytes`, `ProcessPeakUsedPhysicalBytes`, `ProcessUsedVirtualBytes`, and `ProcessPeakUsedVirtualBytes` describe this Unreal process, not whole-system used memory. Unsigned UE values saturate instead of wrapping when converted to Blueprint `int64`.

### `FBertaDisplayInfo` and `FBertaDisplayMode`

`DesktopPosition` and `Resolution` derive from `DisplayRect`; `WorkAreaPosition` and `WorkAreaSize` derive from `WorkArea`. Negative desktop positions are valid. Native and maximum resolutions are UE-provided values; zeros mean unavailable. UE 5.8's Windows display-metrics implementation leaves maximum resolution unset in Editor processes.

Display IDs are opaque, exact lookup values and are not durable identity across OS, driver, or hardware changes. Native monitor handles never leave the implementation.

`GetAvailableDisplayModes` resets output first, rejects unknown IDs and unavailable handles/RHI, preserves refresh rates, removes exact width/height/refresh duplicates, and sorts deterministically by width, height, then refresh rate. UE 5.8's default RHI implementation falls back to global modes; BertaSystemInfo therefore permits this query only for D3D11/D3D12, whose UE 5.8 implementations are truly per-monitor.

### Audio device types

`FBertaAudioInputDeviceInfo` owns the input `DeviceId`, name, channel count, preferred sample rate, and hardware-AEC flag mapped from `AudioCaptureCore`. Enumeration is fresh on each call and never opens a stream. No microphones or no backend returns an empty array without a BertaSystemInfo error.

`FBertaAudioOutputDeviceInfo` owns the output `DeviceId`, name, channel count, sample rate, system-default flag, and current-device flag without exposing Audio Mixer enums. `GetAudioOutputDevices` requires a valid `WorldContextObject`, resolves that world's active audio device, verifies its module is an Audio Mixer, and runs the platform enumeration on UE's audio thread. Identity comparison uses `DeviceId` when both IDs are usable, with a friendly-name fallback only when needed. Outputs reset on every failure; a successful platform query may return no devices.

Device IDs are opaque and should not be parsed or treated as globally permanent.

## Deliberate exclusions

BertaSystemInfo does not change displays, resolutions, refresh rates, HDR, windows, audio routing, or capture-device selection. It does not record microphones, play or analyze audio, publish hotplug events, poll, monitor over time, or expose privacy-sensitive user/machine/network identifiers.

For recording, callers may pair returned microphone metadata with a dedicated solution such as Runtime Audio Importer. It is neither a BertaSystemInfo dependency nor part of this plugin's capture path.

## Platform verification

The repository verifies `BertaDevKitHostEditor Win64 Development` compilation with UE 5.8. Runtime values and device behavior require manual testing. The implementation uses UE Runtime abstractions and has no platform allowlist, but runtime behavior outside Win64 has not been verified and is not claimed.
