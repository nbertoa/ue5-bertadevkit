# BertaProcessBridge

BertaProcessBridge is an independent Unreal Engine 5.8 Runtime plugin that launches external executables directly. It exposes asynchronous textual output, text stdin, completion results, duration, and cancellation to C++ and Blueprint while keeping process ownership tied to a `GameInstance`.

## Installation

Copy the independent plugin into a UE 5.8 project's plugin directory:

```text
BertaDevKitHost/Plugins/BertaProcessBridge/
→ <YourProject>/Plugins/BertaProcessBridge/
```

Build the project and enable **BertaProcessBridge**. It is disabled by default, uses no third-party libraries, and has no dependency on any other Berta plugin.

## Launch API

The Blueprint node `Launch External Process` and the C++ function `UBertaProcessBlueprintLibrary::LaunchProcess` accept `FBertaProcessLaunchOptions`:

| Field | Contract |
| --- | --- |
| `ExecutablePath` | Required non-empty executable name or path. BertaProcessBridge does not require `FileExists`, so UE/platform executable lookup remains available. |
| `Arguments` | Raw argument string passed to UE's process API without sanitizing, rewriting, or shell interpretation. |
| `WorkingDirectory` | Optional. A non-empty path must exist. Empty passes `nullptr`, preserving UE's platform current/default working-directory behavior. |
| `bHidden` | Requests UE's hidden-process behavior. The exact visible/hidden presentation is platform-defined. |

The function resets all outputs first. It returns `false`, a null process, useful error text, and one of `InvalidExecutable`, `InvalidWorkingDirectory`, `SubsystemUnavailable`, or `LaunchFailed` when launch cannot produce a live process. Success returns a valid `UBertaProcess` in `Running` state.

## Process object

One `UBertaProcess` represents exactly one successful launch.

| API | Behavior |
| --- | --- |
| `OnOutput` | Delivers non-empty chunks read from the combined stdout/stderr stream on the Game Thread. A chunk is not promised to be a complete line. |
| `OnFinished` | Delivers exactly one `FBertaProcessResult` on the Game Thread. |
| `IsRunning` | Reports whether the native runner remains active. |
| `GetState` | Returns `Running`, `Completed`, or `Canceled`. |
| `GetDurationSeconds` | Returns live elapsed time while active and the cached final duration afterward. |
| `TryGetResult` | Resets its output and returns `false` while running; returns the cached terminal result afterward. |
| `SendString` | Accepts the complete supplied text for queued UTF-8 delivery and adds no newline. `true` means accepted, not delivered to the child. Pending input is finite, so a message is rejected atomically with `false` when capacity is unavailable. Empty input is an accepted no-op while input is available. |
| `SendLine` | Appends exactly one UE platform line terminator, then uses the same acceptance and capacity rules. Existing terminators are not normalized. |
| `Cancel` | Accepts one active cancellation request and maps `bKillTree` to UE's native process-tree termination option. |

`FBertaProcessResult::Reason` is `Completed` when the process ended without an accepted cancellation, even when its exit code is non-zero. `Canceled` identifies explicit user or ownership teardown cancellation. `bHasExitCode` determines whether `ExitCode` is valid; canceled processes may expose a platform termination code. Duration is cached when the worker reaches its terminal path.

## Output and stdin semantics

The implementation uses UE 5.8's `FProcessStartInfo` with RAII `FProcess`, `FInputPipe`, and `FOutputPipe`. Stdout and stderr are connected to the same output pipe, so their relative text is available but their sources cannot be distinguished. Reads use UE's textual platform `ReadPipe`, so callback boundaries and decoding follow the platform implementation. On Win64, UE decodes pipe bytes through its UTF-8 text path; BertaProcessBridge does not promise arbitrary binary output or stable chunk boundaries.

Input is queued from the Game Thread and written by the process worker. BertaProcessBridge uses UE's byte pipe write after converting `FString` text to UTF-8 so `SendString` does not inherit the newline that UE 5.8's Win64 `WritePipe(FString)` adds. `SendLine` deliberately appends `LINE_TERMINATOR` itself.

Pending output is bounded. When its Game Thread delivery falls behind, the worker applies backpressure instead of silently dropping stdout/stderr; output already accepted remains ordered before `OnFinished`. Each Game Thread drain has a finite budget and continues on later ticks when needed.

## Ownership, ordering, and shutdown

`UBertaProcessSubsystem` is a `UGameInstanceSubsystem`. It strongly owns active process UObjects, so they cannot be garbage-collected while a process is running and separate PIE instances do not share process ownership.

Native work happens on a private worker thread. Worker callbacks never touch reflected UObject state or Blueprint delegates. They append output and terminal records to one ordered queue; Game Thread drains preserve output-before-finish ordering and reject late events after the terminal transition. `OnFinished` is broadcast at most once.

During `GameInstance` teardown the subsystem stops accepting launches, suppresses queued/user callbacks, terminates each remaining process tree from the teardown thread before waiting for its worker, and releases the RAII process and pipe resources. After launch, the parent's copy of the stdin read handle is closed; when the child terminates, a worker blocked in UE's synchronous pipe write can return. Detached processes are not supported and children are not intended to outlive their owner.

## Examples

Direct Win64 executable, with no shell involved:

```text
ExecutablePath: C:\Windows\System32\where.exe
Arguments:      cmd.exe
Hidden:         true
```

Explicit shell selected by the caller:

```text
ExecutablePath: C:\Windows\System32\cmd.exe
Arguments:      /C "echo Hello from BertaProcessBridge"
Hidden:         true
```

The second example has shell behavior only because the caller explicitly chose `cmd.exe`. BertaProcessBridge never wraps arguments in `cmd.exe /C`, PowerShell, `sh -c`, `bash -c`, or another shell. A string such as `dir && something` is merely an argument string unless the selected executable interprets it. The plugin does not sanitize arguments or guarantee shell escaping.

## Deliberate limits and verification

There is no synchronous Blueprint node, detached mode, process enumeration, existing-process attachment, elevation, environment override, timeout, priority control, resource monitoring, file redirection, separate stderr API, binary I/O API, PTY, terminal emulation, networking, or remote execution.

The repository verifies `BertaDevKitHostEditor Win64 Development` compilation. Runtime behavior on Win64, packaged behavior, and all behavior on other platforms remain manual verification work; no all-platform runtime support is claimed.

In a future manually authorized Editor session on Win64, run `BertaProcessBridge.Lifecycle.BlockedInputOwnerShutdown` in the Automation Test window and confirm prompt completion and no surviving child. Then use a child that reads stdin and confirm normal text delivery, output ordering, and an available exit code when the platform supplies one. These Automation Tests are compiled but are not executed by the build.
