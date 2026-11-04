# Week 4 - Interactive Deadlock Simulation System

## Team modules
- Member 1: `gui.py` - Tkinter GUI, input validation, matrix display, graph drawing, actions and log.
- Member 2: `banker.c` - Need calculation, Banker's safety algorithm, safe request/rollback, deadlock detection, recovery.
- Member 3: `graph.c`, `main.c`, `deadlock.h` - RAG edge-list model, shared C interface, test harness and integration.

## Exact DSA concepts implemented
1. **2D arrays / matrices:** Allocation, Maximum, Need and Request matrices.
2. **1D arrays / vectors:** Available and Work vectors, Finish flags, safe sequence, deadlocked flags.
3. **Directed graph / adjacency-by-edges:** Resource Allocation Graph. Allocation edges point Resource -> Process; waiting edges point Process -> Resource.
4. **Iterative worklist-style traversal:** Banker's safety algorithm repeatedly scans unfinished processes whose Need <= Work. It uses repeated array scans, not a literal queue.
5. **State rollback / transaction pattern:** resource request is tentatively applied, tested for safety, then committed or reverted.
6. **Resource release:** recovery adds the terminated process's allocation back to Available and clears its allocation/request row.

## Build and run (Ubuntu/WSL)
```bash
sudo apt update
sudo apt install build-essential python3 python3-tk make
make clean
make
python3 gui.py
```
Or `chmod +x run.sh && ./run.sh`.
For WSL, GUI display requires WSLg (Windows 11) or a configured X server.

## Sample data
Available: `3 3 2`
Allocation rows:
```text
0 1 0
2 0 0
3 0 2
2 1 1
0 0 2
```
Maximum rows:
```text
7 5 3
3 2 2
9 0 2
2 2 2
4 3 3
```
Expected safe sequence: `P1 -> P3 -> P4 -> P0 -> P2` (one valid sequence).

## Important scope note
The GUI provides two request paths: "Submit safe request" grants only after a safety check; "Queue blocked request" records a request that cannot currently be met in the outstanding Request matrix. Detection consumes this matrix, and the RAG draws waiting edges from it. Recovery demonstrates process termination and resource release, not victim-cost selection.

Limits: 20 processes, 10 resource types. All values are non-negative integers; Allocation cannot exceed Maximum.
