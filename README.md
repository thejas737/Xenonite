# Core Process Module - Multi-Process Simulator

## Overview
This module represents the **Core Process** of the multi-process simulator architecture. Its responsibility is to handle CPU execution, Memory management, Stack operations, and Queue handling[cite: 2]. 

It acts as the central bridge in the system: it passively listens for execution commands from the UI Process, simulates the hardware operation, and asynchronously pushes the execution status to the Logging Process.

## IPC Mechanism Selection & Justification
**Selected Mechanism:** POSIX Message Queues (`<mqueue.h>`)

POSIX Message Queues were selected over standard Pipes, FIFOs, or Shared Memory for the following architectural reasons:
1. **Structured Data Transfer:** Unlike pipes which transmit continuous, unformatted byte streams, message queues allow us to transmit strictly defined C `structs` (e.g., passing an Opcode and Value together). This guarantees the simulated CPU never reads a fragmented or partial instruction.
2. **Built-in Synchronization:** Shared Memory requires complex, manual thread synchronization using Mutexes or Semaphores to prevent race conditions. POSIX queues handle synchronization automatically at the OS kernel level. `mq_receive()` cleanly blocks (sleeps) the Core process when no UI commands are available, preventing CPU starvation.
3. **True Decoupling:** The UI, Core, and Logger processes do not require a parent-child `fork()` relationship. Named queues allow them to act as fully independent, modular components.

## Communication Interfaces
For successful system integration, the surrounding processes must adhere to these interfaces:

### 1. Incoming (From UI Process)
* **Queue Name:** `/ui_to_core`
* **Core Open Mode:** `O_RDONLY` (UI must open as `O_WRONLY`)
* **Expected Data Type:** 
  ```c
  typedef struct {
      int opcode;
      int value;
  } UI_Command;
