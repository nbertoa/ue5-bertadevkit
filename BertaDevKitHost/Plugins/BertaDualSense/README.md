# BertaDualSense

BertaDualSense is an independent Unreal Engine 5.8 Runtime input-device plugin for native Sony DualSense and DualSense Edge controllers on Win64. It uses SDL3 to deliver normal Unreal input, selected DualSense hardware features, and standard UE output paths without a dependency on BertaDevKit.

Complete technical documentation: <https://nbertoa.github.io/ue5-bertadevkit/bertadualsense/>

## Requirements and installation

- Unreal Engine 5.8
- Win64
- A supported C++ toolchain when building from source

Copy this directory into a UE 5.8 project:

```text
BertaDevKitHost/Plugins/BertaDualSense/
→ <YourProject>/Plugins/BertaDualSense/
```

Build the project and enable **BertaDualSense** in the Plugins window. The plugin is disabled by default. It owns SDL3 3.4.14 and stages/loads its DLL from the plugin deployment, so no system-wide SDL installation is required.

## Supported hardware

BertaDualSense accepts only Sony controllers with these USB vendor/product identifiers:

| Controller | VID | PID |
| --- | ---: | ---: |
| DualSense | `0x054C` | `0x0CE6` |
| DualSense Edge | `0x054C` | `0x0DF2` |

Other SDL gamepads are ignored. USB and Bluetooth DualSense connections are supported through SDL3.

## Input

### Standard Unreal gamepad input

Face buttons, D-pad, shoulders, thumbstick clicks, sticks, analog triggers, trigger-threshold buttons, and derived stick-direction buttons are delivered through standard Unreal `Gamepad_*` keys. Cross, Circle, Square, and Triangle use positional face-button mappings. Create maps to `Gamepad_Special_Left`; Options maps to `Gamepad_Special_Right`.

Because these are normal Unreal input events, Enhanced Input can bind standard controls without a Berta-specific API.

### Touchpad and special buttons

Touch contact and physical controls are separate:

| Physical control | Unreal key | Notes |
| --- | --- | --- |
| Touch contact | `Gamepad_Special_Left_X`, `Gamepad_Special_Left_Y`, `Gamepad_Special_Left_Touched` | One stable primary touch stream; X/Y are SDL normalized coordinates. |
| Touchpad click | `BertaDualSense_TouchpadClick` | Physical touchpad depression only. |
| Secondary touch | `BertaDualSense_Touchpad2_X`, `BertaDualSense_Touchpad2_Y`, `BertaDualSense_Touchpad2_Touched` | Second physical contact; primary remains Unreal's `Gamepad_Special_Left_*`. |
| PS button | `BertaDualSense_PSButton` | Separate from Create and Options. |
| Microphone/mute button | `BertaDualSense_MicrophoneButton` | Input only; it does not change the microphone LED. |
| Create | `Gamepad_Special_Left` | Standard gamepad key. |
| Options | `Gamepad_Special_Right` | Standard gamepad key. |

The three `BertaDualSense_*` special keys are registered as gamepad keys and can be bound through Enhanced Input. Touchpad click is intentionally not an alias for touch contact or Create.

### DualSense Edge inputs

For PID `0x0DF2` only, BertaDualSense registers these gamepad keys:

| SDL control | Unreal key |
| --- | --- |
| Right rear paddle | `BertaDualSense_Edge_RightPaddle` |
| Left rear paddle | `BertaDualSense_Edge_LeftPaddle` |
| Right Fn button | `BertaDualSense_Edge_RightFn` |
| Left Fn button | `BertaDualSense_Edge_LeftFn` |

Edge-specific input is implemented but remains hardware-unverified in this project.

## Sensors

When SDL reports support, the plugin enables the gyroscope and accelerometer. It sends gyroscope radians/second as Unreal Rotation Rate and accelerometer data, divided by standard gravity, as Unreal Gravity through the standard motion-input path.

The plugin deliberately sends zero Tilt and Acceleration. SDL accelerometer data includes gravity, and BertaDualSense does not perform sensor fusion or fabricate linear acceleration.

## Output

| Feature | UE path or API | Behavior |
| --- | --- | --- |
| Standard rumble | `SetChannelValue` / `SetChannelValues` | Maps UE large channels to SDL low-frequency rumble and small channels to high-frequency rumble. |
| Lightbar RGB | `SetLightColor`, `ResetLightColor`, and light-color device properties | Uses the SDL RGB LED capability when available. Reset sets black. |
| Player LEDs | Automatic on connection | Uses the UE platform user's local index when SDL reports player-LED support. |
| Microphone LED | `UBertaDualSenseBlueprintLibrary::SetMicrophoneLed(int32 ControllerId, bool bEnabled)` | Explicit Off/On output independent of the microphone button. |
| Adaptive triggers | UE trigger device properties | Supports Reset, Feedback, Resistance, and Vibration for the selected trigger mask. |

Legacy `ControllerId` output callbacks are routed to every connected BertaDualSense assigned to that resolved UE platform user. They cannot select one exact `FInputDeviceId` when several controllers share the same platform user.

`GetConnectedDualSenseDevices` reports model, wired/wireless state, SDL power state, battery percentage (`-1` when unavailable), firmware integer, serial when SDL provides it, and output/sensor capabilities. Its opaque device handle targets one currently connected device for lightbar and microphone LED output. A reconnect reuses the handle during the plugin lifetime when SDL provides the same serial; without a serial it receives a new handle.

### Adaptive-trigger note

BertaDualSense uses SDL3's PS5 effect path for trigger output, not trigger rumble. Reset, Feedback, and Vibration map to their supported SDL PS5 effect modes. UE's generic Resistance property includes start/end positions and strengths; SDL's available feedback mode cannot faithfully encode the complete interval. The current Resistance mapping starts at `StartPosition` and uses the mean of `StartStrengh` and `EndStrengh`; `EndPosition` is not represented. Treat it as an approximation rather than a linear resistance curve.

## Connection behavior

BertaDualSense polls SDL gamepads explicitly and does not use SDL's event queue. On connection it registers and maps a UE input device. An SDL serial, when available, is the persistent signal used to reuse its `FInputDeviceId` on reconnect during the plugin's lifetime. Without a serial, a reconnect receives a new UE device ID to avoid falsely identifying another controller.

Disconnect and shutdown release held buttons, clear analog/touch state, clear active trigger output, stop rumble, disable enabled sensors, unmap the UE device, and close the SDL gamepad. A reconnect starts with neutral cached input and output state.

## Windows coexistence and duplicate input

BertaDualSense filters only the native Sony IDs above; it does not claim a device exclusively or suppress other UE input backends. Keep another backend from processing the same physical Sony controller if it produces duplicate gamepad events. In particular, configure GameInputWindows gamepad processing deliberately when it is enabled; UE's GameInput settings warn that simultaneous gamepad backends can duplicate input.

XInput can continue to handle Xbox/XInput controllers. Steam Input, DS4Windows, and similar software can expose a DualSense as a separate virtual Xbox/XInput controller; BertaDualSense cannot suppress that virtual device. Disable or configure the emulator's virtual controller when duplicate input is not wanted.

## Limits

- Win64 and UE 5.8 only.
- Only the listed Sony VID/PID pairs are accepted.
- No custom touch gestures, touchpad pressure, speaker/audio, or custom HID transport API.
- The PS and microphone buttons are custom keys, not standard Unreal gamepad aliases.
- Packaged-project behavior and DualSense Edge hardware behavior still require project-specific manual verification.

The normal DualSense path has been manually exercised during development for standard input, touchpad, sensors, LEDs, rumble, adaptive-trigger behavior, and the three special buttons. This is not a substitute for verifying a target project's mappings and input-backend configuration.
