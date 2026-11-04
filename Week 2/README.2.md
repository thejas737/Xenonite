# Week 3 - Process Scheduling Simulator (OS + DSA)

## Objective
Simulate CPU scheduling algorithms and calculate waiting time, turnaround time,
response time, CPU utilization and idle time.

## Implemented algorithms
1. FCFS (First-Come-First-Served)
2. SJF Non-preemptive
3. SJF Preemptive (Shortest Remaining Time First / SRTF)
4. Priority Non-preemptive
5. Priority Preemptive
6. Round Robin with user-configurable time quantum

**Priority convention:** a smaller numeric value means a higher priority.
Ties are resolved by arrival time, then input order.

## Team division (three members)
- Member 1 - `main.c`: CLI, input validation, menu, algorithm selection.
- Member 2 - `scheduler.c`: scheduling algorithms, CPU timeline and metric calculations.
- Member 3 - `report.c`: Gantt chart and result/comparison reporting.
- Shared interface - `scheduler.h`: structures, constants and function declarations.

Each member should review and understand their assigned code. The shared header
must remain consistent across all modules.

## OS + DSA integration
- **OS:** CPU scheduling policies determine which process gets the CPU and when.
  Preemptive algorithms can interrupt a running process; non-preemptive ones
  keep the CPU until the selected process finishes.
- **DSA:** arrays store process records, remaining burst times and completion
  state. Round Robin uses a FIFO ready queue. The Gantt timeline is stored as
  contiguous execution segments.
- **Combined:** scheduling algorithms operate on process data structures to
  simulate CPU allocation and calculate performance metrics.

## Build and run (Ubuntu / WSL)
```bash
sudo apt update
sudo apt install build-essential make
cd "/mnt/c/Users/<WindowsUser>/Documents/<path>/week3_process_scheduling"
make clean
make
./scheduler
```
Alternatively: `chmod +x run.sh && ./run.sh`

## Input
Enter process count, then for each process: ID, arrival time, burst time and
priority. Enter a Round Robin time quantum. Use menu options to run one
algorithm or compare all six.

## Metrics
- Completion Time (CT): time at which a process finishes.
- Turnaround Time (TAT) = CT - Arrival Time.
- Waiting Time (WT) = TAT - Burst Time.
- Response Time (RT) = First CPU Start - Arrival Time.
- CPU Utilization = (sum of burst times / total elapsed time) * 100.
- Idle Time = total elapsed time - sum of burst times.

## Sample test case
Use:
| Process | Arrival | Burst | Priority |
|---|---:|---:|---:|
| P1 | 0 | 5 | 2 |
| P2 | 1 | 3 | 1 |
| P3 | 2 | 1 | 3 |

Try all six algorithms with Round Robin quantum 2. The output includes a
Gantt chart, per-process WT/TAT/RT, averages and CPU utilization.

## Important notes
- Time is simulated in integer time units.
- Lower priority number means higher priority.
- CPU utilization counts only busy CPU time over elapsed time, including idle
  periods between first arrival and final completion.
- For a report, run at least three different datasets and record the actual
  outputs. Do not invent benchmark values.
