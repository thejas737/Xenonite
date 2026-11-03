# OS + DSA Multi-Process Simulator (POSIX IPC)

## Requirements
- Linux (Ubuntu, Debian, WSL2 Ubuntu, or a Linux lab machine)
- GCC and GNU Make
- POSIX message queues supported by the kernel

## Build and run
```bash
make
./run.sh
```
The launcher starts the Logger process, then Core, then UI. Use the UI menu. Select `0` to shut down cleanly. Execution logs are appended to `simulator.log`.

If message queues are unavailable on a Linux setup, try `sudo mount -t mqueue none /dev/mqueue` (only if `/dev/mqueue` is not already mounted). Do not run the program with sudo unless your lab setup requires it.

## Member ownership
- Member 1: `ui.c` — menu, input validation, request/response interaction.
- Member 2: `core.c` — CPU addition, simulated memory, stack, queue, benchmark.
- Member 3: `logger.c` — log consumer and timestamped log file.
- Shared interface: `common.h` — queue names, message structure, command identifiers.
- Benchmark baseline: `standalone.c` — single-process CPU/stack/queue workload.

## IPC architecture
```text
                 /osdsa_cmd_q
        UI ------------------------> Core
        UI <------------------------ Core
                 /osdsa_resp_q
                                      |
                                      | /osdsa_log_q
                                      v
                                    Logger ---> simulator.log
```
All three are independent OS processes. POSIX message queues carry fixed-size `Message` structures. This design was chosen because it provides message boundaries, kernel-managed buffering, and simple producer/consumer communication without shared-memory synchronization.

## Quick test plan
1. Select `1`, enter `12 30`; expected result is `42`.
2. Select `3`, push `15`; then `4`, pop; expected value is `15`.
3. Select `4` on an empty stack; expected underflow error.
4. Select `5`, enqueue `8`; then `6`, dequeue; expected value is `8`.
5. Select `2`, read slot `0`; after the addition above, expected value is `42`.
6. Select `7` and inspect memory, stack and queue.
7. Select `8`, enter `1000000`; note the reported core-loop time.
8. Select `0`, then inspect `simulator.log` for timestamped execution and error events.

## Benchmark note
Run `./standalone 1000000` for the single-process baseline. In the multiprocess version, choose menu option 8 and enter the same iteration count; note that this measures the core loop after an IPC request, not the full end-to-end UI-to-core round trip. The two workloads are not perfectly identical, so report them as indicative measurements, not a strict apples-to-apples benchmark. For a stronger report, add a repeated request/response benchmark and collect CPU/RSS using `/usr/bin/time -v` for both runs. Run each several times and compare averages.

## Integration notes
- Keep `common.h` identical for all members; changing `Message` requires recompiling all three programs.
- Compile all programs with `make` from this directory.
- Start with `./run.sh`; it starts processes in the right order and waits for shutdown.
- If the team wants to launch processes manually, start `./logger`, then `./core`, then `./ui` in three terminals. Avoid running `run.sh` at the same time as manual processes.
- If a prior run was killed with Ctrl+C, run `./run.sh` again; it attempts to remove stale queue names before starting.
