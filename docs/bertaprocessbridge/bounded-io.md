# Bounded process I/O: keeping a slow Game Thread from owning unbounded output

## Problem and failure scenario

An external process can write stdout/stderr faster than an Unreal Game Thread can deliver Blueprint callbacks. The producer is a native worker; the consumer runs `OnOutput` on the Game Thread. A slow callback, a stalled Editor frame, or sustained high output can make their rates diverge for an arbitrary time. The same mismatch exists in reverse when callers submit stdin faster than the child consumes it.

The original implementation in [`458082f`](https://github.com/nbertoa/ue5-bertadevkit/commit/458082f) preserved ordered worker-to-Game-Thread delivery, but the pending output had no capacity limit. Repeated reads could accumulate strings in the pending queue for as long as the mismatch continued. A successful small-output test could not establish memory behavior under sustained load.

## Why an unlimited queue was unsafe

If the producer rate stays above the consumer rate, backlog grows approximately with `(producer bytes/s - consumer bytes/s) × time`. No finite burst test bounds that growth. In a `GameInstance`-owned process, this could raise process memory until the child exits, the Game Thread catches up, or the Unreal process runs out of memory. Simply reducing each tick's work without also limiting the backlog would make the memory problem worse.

## Alternatives considered

| Approach | Consequence |
| --- | --- |
| Unlimited queue and unlimited drain | Preserves output but can grow memory and monopolize a frame. |
| Unlimited queue with per-tick budget | Protects frame time while allowing persistent backlog growth. |
| Drop old or new output | Bounds memory but breaks the existing complete, ordered output contract. |
| Bound the queue and wait for space | Keeps ordered output and makes a fast producer wait; the child can eventually block on its OS pipe. |

The last choice fits a textual process bridge where output loss would be surprising. It deliberately trades producer throughput for bounded pending delivery.

## Implemented contract

Commit [`7cf0b9a`](https://github.com/nbertoa/ue5-bertadevkit/commit/7cf0b9a) added two independent limits in `BertaProcess.cpp`:

- Pending output: at most **64 events** and **512 KiB of counted `TCHAR` payload**. Worker output is split into chunks of at most **16 Ki characters** before enqueueing. If either limit is reached, the worker waits for the Game Thread drain to free space. The terminal event uses the same ordered queue, so accepted output precedes `OnFinished`.
- Delivery: one Game Thread drain handles at most **16 events** and starts no more events after reaching **128 KiB**. A later tick resumes the remaining queue. A single final event can take the drain slightly over the byte budget because the budget is checked before dequeue.
- Pending stdin: at most **1 MiB of queued UTF-8 bytes**. `SendString`/`SendLine` reject an entire new message with `false` when it does not fit; `true` means queued, not delivered to the child.

The output byte count is a queue-payload bound, not a strict whole-process RSS bound. `FString` capacity, event and array storage, a temporary `ReadPipe` result, conversion buffers, and native pipes consume additional Unreal-process memory; the child process has its own memory usage. A single `ReadPipe` result can be larger than one output chunk before it is split. The implementation prevents sustained growth of the pending callback queue; it does not promise a fixed peak for every transient allocation.

## Behavior under load and shutdown

When callbacks are slow, the worker pauses at the output limit. It then stops draining the OS pipe, which can block the child producer. This is intentional backpressure. It also means a child that waits for stdin while continuously writing stdout may progress slowly until the Game Thread consumes output. The bridge depends on a ticking Game Thread for output delivery.

The dispatcher wakes its waiting worker when a drain frees space. During `GameInstance` teardown it suppresses callbacks, clears queued events, wakes a blocked worker, and then cancellation and worker join can complete. `OnFinished` is delivered at most once during normal completion and is suppressed during owner teardown. No additional feature or architecture was added for this sprint.

## How to verify

1. Build `BertaDevKitHostEditor Win64 Development` and run `BertaProcessBridge.*` Automation Tests. These currently cover invalid launch context and shutdown contracts; they do **not** stress queue saturation.
2. In a disposable PIE session, launch a local child that writes substantially more than 512 KiB to stdout in 16 KiB-or-smaller writes and exits. Keep `OnOutput` deliberately slower than the producer for several seconds. Record Unreal process working set, callback byte total, callback order, and the time until `OnFinished`.
3. Verify that pending output does not continue growing throughout the stalled interval, all produced text arrives in order once callbacks resume, and `OnFinished` follows the final output. Repeat while tearing down the `GameInstance` with the worker blocked; confirm prompt shutdown and no late callbacks.
4. Separately send repeated stdin messages while the child does not read. Confirm `SendString` eventually returns `false` without accepting a partial message. Repeat after the child resumes reading.

This load scenario remains a behavioral verification item until it has been run and recorded; compilation and the current contract tests alone do not prove it.

## Reusable lesson

When Unreal delegates consume data on the Game Thread and an external producer has its own thread, the queue limit and per-tick work limit solve different problems. Bound the queue to control retained work, budget the drain to protect frames, define whether overload blocks or drops, and make shutdown wake blocked producers. State clearly which memory is actually bounded.
