# Page Replacement Simulator - OS + DSA
Implements FIFO, LRU and Optimal page replacement in C.

## Team division
- Member 1: `main.c` - CLI, input validation and menu.
- Member 2: `page_replacement.c` - FIFO, LRU, Optimal and simulation.
- Member 3: `report.c` - frame-by-frame output and comparison.
- Shared: `page_replacement.h` - common structures and declarations.

## OS + DSA
OS: virtual memory, physical frames and page-fault handling.
DSA: arrays store references and frames; FIFO uses queue order; LRU tracks
last-use indices; Optimal scans future references to choose a victim.
These algorithms simulate OS memory-management decisions.

## Build (Ubuntu / WSL)
```bash
sudo apt update
sudo apt install build-essential make
make clean
make
./page_simulator
```
Or `chmod +x run.sh && ./run.sh`.

## Specification sample
Reference string: `7 0 1 2 0 3 0 4 2 3`; frames: `3`.
Choose option 4 to compare all algorithms.

Hit ratio = hits / total references * 100.
Fault ratio = faults / total references * 100.

Limits: 1000 references, 100 frames. Page numbers must be non-negative.
Run at least three datasets and record actual output in the project report.
