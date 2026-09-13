# BertaProcessBridge

BertaProcessBridge is an independent Unreal Engine 5.8 Runtime plugin for launching one external executable per process object. Its C++ and Blueprint API provides asynchronous text output, text stdin, completion results, and cancellation without introducing a shell.

Complete technical documentation: <https://nbertoa.github.io/ue5-bertadevkit/bertaprocessbridge/>

## Requirements and installation

- Unreal Engine 5.8
- A supported C++ toolchain when building from source

Copy this directory into a UE 5.8 project:

```text
BertaDevKitHost/Plugins/BertaProcessBridge/
→ <YourProject>/Plugins/BertaProcessBridge/
```

Build the project and enable **BertaProcessBridge**. The plugin is disabled by default, contains no third-party libraries, and has no dependency on BertaDevKit, BertaDualSense, or BertaSystemInfo.

## Public API

`Launch External Process` takes a world context and `FBertaProcessLaunchOptions`, then returns a running `UBertaProcess` or an explicit `EBertaProcessLaunchError`. `ExecutablePath` must be non-empty. `Arguments` is passed unchanged as UE's raw process argument string. A non-empty `WorkingDirectory` must name an existing directory; an empty value passes `nullptr` to UE and therefore uses the platform process API's current/default working directory. `bHidden` maps to UE's hidden-process option where the target platform supports it.

`UBertaProcess` exposes:

- `OnOutput`: non-empty text chunks from one combined stdout/stderr pipe. Chunks are not guaranteed to be complete lines, and their boundaries are implementation details.
- `OnFinished`: one terminal `FBertaProcessResult` with `Completed` or `Canceled`, duration, and an optional exit code.
- `IsRunning`, `GetState`, `GetDurationSeconds`, and `TryGetResult`.
- `SendString`: queues exactly the supplied text, encoded as UTF-8 bytes, without adding a newline. Empty text is an accepted no-op while the process accepts input.
- `SendLine`: calls the same input path after appending exactly one UE platform line terminator. It does not normalize a terminator already present in the supplied string.
- `Cancel`: requests termination and optionally asks UE to kill the process tree. A duplicate or terminal cancellation request returns `false`.

`Completed` means the process ended without an accepted BertaProcessBridge cancellation; it does not mean `ExitCode == 0`. `bHasExitCode` is the only indication that `ExitCode` is valid. A canceled process can still expose a platform termination code when UE can retrieve one.

## Ownership and threading

Every successfully launched process is strongly owned by the `UBertaProcessSubsystem` for its `GameInstance` until it becomes terminal. Native output and terminal events are serialized through a small ordered queue and delivered on the Game Thread, so output observed before native termination is delivered before `OnFinished`. Ending the `GameInstance`, including ending PIE, suppresses user callbacks, requests kill-tree cancellation for remaining children, waits for their worker threads, and releases UE process and pipe resources.

The implementation uses UE 5.8's RAII `FProcess`, `FProcessStartInfo`, `FInputPipe`, and `FOutputPipe` abstractions. Output decoding follows UE's platform `ReadPipe` behavior. On Win64, stdout and stderr are intentionally connected to the same pipe and cannot be distinguished. This is textual process I/O, not a binary stream, PTY, or terminal emulator.

## Direct executable and explicit-shell examples

A direct executable example on Win64 is:

```text
ExecutablePath: C:\Windows\System32\where.exe
Arguments:      cmd.exe
```

The plugin launches `where.exe` itself; it does not parse shell syntax.

A caller may explicitly choose a shell when shell behavior is wanted:

```text
ExecutablePath: C:\Windows\System32\cmd.exe
Arguments:      /C "echo Hello from BertaProcessBridge"
```

Here `cmd.exe` is explicitly selected by the caller. BertaProcessBridge never adds `cmd.exe /C`, PowerShell, `sh -c`, or another shell and does not provide shell escaping guarantees.

## Scope and verification status

BertaProcessBridge does not provide detached or fire-and-forget processes, process enumeration, attachment, elevation, environment overrides, timeouts, priorities, resource monitoring, file redirection, separate stderr, binary I/O, PTY support, terminal emulation, networking, or remote execution. Processes are not intended to survive their owning `GameInstance`.

Win64 compilation is verified through the repository host. Runtime behavior in Editor, PIE, packaged Win64, and other platforms requires manual verification; no cross-platform runtime claim is made.
