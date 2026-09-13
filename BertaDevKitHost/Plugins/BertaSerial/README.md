# BertaSerial

BertaSerial is a Win64 Runtime plugin for asynchronous Windows COM-port communication in Unreal Engine 5.8. It is intended for Arduino, ESP32, USB-to-serial adapters, sensors, instruments, and other R&D hardware prototypes.

The plugin is an independent sibling: it has no dependency on BertaDevKit or another Berta plugin and can be copied by itself into `<YourProject>/Plugins/BertaSerial/`.

## Blueprint flow

```text
Get Serial Ports
→ select a COM port
→ Open Serial Port
→ bind On Bytes Received and On Closed
→ Write Bytes / Write String UTF8 / Write Line UTF8
→ Close
```

`Get Serial Ports` queries the current set of present `GUID_DEVINTERFACE_COMPORT` devices on every call. It returns the logical `PortName`, a best-effort friendly name and manufacturer, and informational Windows device-instance and hardware IDs. Results exclude LPT devices, are deduplicated, and sort naturally (`COM2` before `COM10`). An empty result is successful. COM numbers and device metadata can change and are not permanent device identities.

`Open Serial Port` accepts only `COM<number>` names and opens the canonical `\\.\COM<number>` communications path exclusively. It configures baud rate, 5–8 data bits, parity, stop bits, flow control, and initial DTR/RTS state. Five data bits cannot use two stop bits; one-and-a-half stop bits are only valid with five data bits.

- `None` flow control disables CTS and XON/XOFF; RTS follows `bRtsEnabled`.
- `RtsCts` enables CTS output flow and RTS handshaking; `bRtsEnabled` is ignored.
- `XOnXOff` enables software input/output flow control; RTS follows `bRtsEnabled`.
- `bDtrEnabled` selects the initial enabled/disabled DTR state. Runtime DTR/RTS mutation is outside v1.

## Data and lifetime

`OnBytesReceived` is the canonical receive API. It supplies exact raw bytes on the Game Thread. Chunks are arbitrary driver/read boundaries: a protocol message can span callbacks, and one callback can contain multiple messages. The plugin does not assume UTF-8, null termination, line framing, or any higher-level protocol.

`WriteBytes` queues an exact byte array without blocking for physical transmission. `WriteStringUtf8` encodes text as UTF-8 without a null byte or terminator. `WriteLineUtf8` appends exactly LF, CRLF, or CR before encoding; it does not normalize an existing ending.

Outgoing queued/in-flight payload and the pending Game-Thread receive backlog are each bounded to 4 MiB. A write that would exceed its bound returns `false` without partially queuing data. A receive backlog overflow closes the connection explicitly with `ReceiveBufferOverflow` instead of silently dropping protocol data.

Each successfully opened `UBertaSerialPort` is strongly owned by the current `GameInstance` subsystem until it closes. One private worker performs overlapped reads and ordered overlapped writes; no serial I/O blocks the Game Thread. `Close` cancels outstanding I/O, waits for cancellation completion while the `OVERLAPPED` storage remains alive, releases native handles, and emits one `UserClosed` event. GameInstance teardown performs the same cleanup while suppressing user delegates.

Public `UBertaSerialPort` operations are Game-Thread-only. Calls from C++ off the Game Thread fail safely rather than blocking for a thread hop.

Recognizable device-removal errors close with `ConnectionLost`; other unrecoverable driver/read/write failures use `IOFailure`. There is no automatic reconnect. Callers should enumerate and explicitly reopen because COM assignments and physical devices may change.

## Scope and verification

BertaSerial requires Unreal Engine 5.8 and Win64. It uses native Windows SetupAPI and communications APIs and introduces no third-party library. It does not provide hotplug events, a text receive parser, protocol framing, Modbus, MIDI, HID, BLE, networking, or terminal emulation.

The plugin is compile-verified with the Win64 development host. Manual hardware/runtime verification with a real adapter or paired virtual COM ports remains required.
