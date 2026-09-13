# BertaSerial

BertaSerial provides Win64 Runtime COM-port enumeration and asynchronous serial transport for Unreal Engine 5.8. It is an independent plugin with C++ and Blueprint APIs and no dependency on another Berta plugin.

## Install

Copy `BertaDevKitHost/Plugins/BertaSerial/` to `<YourProject>/Plugins/BertaSerial/`, regenerate project files if needed, build for Win64, and enable BertaSerial.

## Sources and opening

`Get Serial Ports` performs a fresh SetupAPI query for present COM-port interfaces. Each `FBertaSerialPortInfo` contains:

- `PortName`, such as `COM3`;
- a best-effort `FriendlyName`;
- optional `Manufacturer`;
- an informational `DeviceInstanceId`;
- optional raw Windows `HardwareIds`.

COM names and device metadata are local operational information, not stable business identifiers. No ports is a successful empty result.

`Open Serial Port` accepts a `FBertaSerialOpenOptions` and the calling world context. A successful call returns one `UBertaSerialPort` owned by that `GameInstance`. Port names must be `COM<number>`; direct arbitrary device paths are rejected.

Settings cover positive baud rates, 5–8 data bits, None/Odd/Even/Mark/Space parity, and one/one-and-a-half/two stop bits. Windows permits 1.5 stop bits only with five data bits and rejects two stop bits with five data bits.

Flow control is explicit:

- `None`: no CTS or software flow control; RTS follows `bRtsEnabled`.
- `RtsCts`: CTS output flow plus RTS handshake; `bRtsEnabled` is ignored.
- `XOnXOff`: input/output software flow control; RTS follows `bRtsEnabled`.

`bDtrEnabled` controls the initial DTR level. The MVP does not expose runtime DTR/RTS changes.

## Receive, write, and close

Bind `On Bytes Received` before exchanging data. It runs on the Game Thread and returns exact bytes. Read chunks are arbitrary and must not be treated as protocol frames. BertaSerial does not decode incoming text.

`Write Bytes` queues exact data asynchronously. `Write String UTF8` adds neither a null byte nor a line ending. `Write Line UTF8` appends exactly LF, CRLF, or CR. Empty writes are harmless while the port is open.

Both outbound queued/in-flight data and pending receive delivery are bounded to 4 MiB. Outbound overflow rejects the new write atomically. Receive overflow closes with `ReceiveBufferOverflow`; no bytes are silently discarded while claiming continued operation.

`Close` cancels outstanding overlapped I/O and emits `On Closed` exactly once with `UserClosed`. Device removal uses `ConnectionLost` when Windows reports a recognizable removal error; other unrecoverable I/O failures use `IOFailure`. Bytes accepted before a terminal event are delivered first. There is no automatic reconnect or hotplug event system.

The subsystem stops all owned workers during GameInstance teardown and suppresses callbacks into a dying world.

Public `UBertaSerialPort` operations are Game-Thread-only; off-thread C++ calls fail rather than enqueueing a hidden Game-Thread operation.

## Blueprint outline

```text
Get Serial Ports
→ choose PortName
→ Open Serial Port
→ bind On Bytes Received / On Closed
→ Write Bytes or UTF-8 helpers
→ Close
```

## Boundaries

BertaSerial is not a protocol framework, terminal emulator, Modbus implementation, MIDI/HID/BLE layer, or networking plugin. Hardware and driver behavior must be verified with the intended real or virtual COM device. The Win64 host build is verified; manual hardware/runtime and packaged verification remain pending.
