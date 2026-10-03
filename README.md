# RISC-V OS Kernel

A small multithreaded operating-system kernel for 64-bit RISC-V, written in C++ and RISC-V assembly. It provides threads with preemptive time-sharing, counting semaphores, sleeping, a heap allocator and interrupt-driven console I/O, exposed through three interface layers: a system-call ABI, a C API and a C++ API.

> Coursework for **Operating Systems 1** at the School of Electrical Engineering, University of Belgrade (2025/26). The kernel is built as a library: it is statically linked with the user application into a single image that runs on an emulated RISC-V machine, much like firmware on an embedded system.

## Features

- **Threads**: create, exit and voluntary yield. User threads run in user mode, kernel threads (idle, console output) in supervisor mode.
- **Preemptive round-robin scheduling**: the timer interrupt counts ticks, and a thread is preempted once it uses up its time slice.
- **Sleeping**: `time_sleep` backed by a delta list, so a timer tick only has to update the head of the list.
- **Counting semaphores**: `wait`/`signal`, plus `wait_n`/`signal_n` that acquire or release several units at once.
- **Memory allocator**: first-fit over an address-ordered free list, with block splitting and coalescing of neighbouring free blocks.
- **Console I/O**: buffered, interrupt-driven input and a dedicated kernel thread that drains output.
- **Clean shutdown**: the kernel waits until all user threads have finished and all console output is flushed, then halts the emulator.

## Architecture

### Traps and system calls

Every trap enters `supervisorTrap` (`src/supervisorTrap.S`), which swaps `sp` with `sscratch` to switch onto the running thread's **kernel stack**, saves all general-purpose registers and calls `RiscV::handleSupervisorTrap`. The handler dispatches on `scause`:

| `scause` | Cause | Handling |
|---|---|---|
| `8`, `9` | `ecall` from user / supervisor mode | System call; the code is in `a0`, arguments in `a1`–`a4`, the result goes back in `a0` |
| `0x8000…0001` | Software interrupt (timer) | Advance sleeping threads; preempt when the time slice is used up |
| `0x8000…0009` | External interrupt (PLIC) | Console receive interrupt |

System call codes:

| Code | Call | | Code | Call |
|---|---|---|---|---|
| `0x01` | `mem_alloc` | | `0x23` | `sem_wait` |
| `0x02` | `mem_free` | | `0x24` | `sem_signal` |
| `0x11` | `thread_create` | | `0x25` | `sem_wait_n` |
| `0x12` | `thread_exit` | | `0x26` | `sem_signal_n` |
| `0x13` | `thread_dispatch` | | `0x31` | `time_sleep` |
| `0x21` | `sem_open` | | `0x41` | `getc` |
| `0x22` | `sem_close` | | `0x42` | `putc` |

### Context switching and scheduling

`Thread::dispatch` puts the current thread back on the ready queue (unless it has finished), takes the next one and calls `contextSwitch` (`src/contextSwitch.S`), which saves and restores `ra`, `sp` and the callee-saved registers `s0`–`s11`. Each thread also keeps its own `sepc` and `sstatus`, so it resumes in the right place and privilege mode.

A new thread starts in `threadWrapper`, which sets up its kernel stack and drops into the thread's privilege mode with `sret` before calling the thread body. The ready queue is FIFO; when it is empty, an idle thread runs `wfi`. Finished threads are moved to a zombie list and freed during the next dispatch.

### Sleeping

Sleeping threads are kept in a **delta list** (`inc/SleepList.hpp`): each entry stores its wake-up time relative to the entry before it. A timer tick therefore decrements only the head of the list, and every thread whose remaining time reaches zero is moved back to the ready queue.

### Semaphores

Semaphores support multi-unit operations. If `wait(n)` cannot be satisfied, the thread records how many units it still needs and blocks. `signal(n)` then hands units to the blocked threads in FIFO order. Closing a semaphore releases all blocked threads, whose `wait` then returns an error. Handles passed in from user code are checked against a registry of open semaphores before use.

### Memory allocator

The heap is managed in fixed-size blocks (`MEM_BLOCK_SIZE`). Each free fragment carries a small header (`prev`, `next`, `size`), and the free list is kept sorted by address. Allocation is **first-fit** and splits a fragment when the remainder is at least one block. Freeing reinserts the fragment in address order and **coalesces** it with its free neighbours on both sides. The C layer converts byte sizes to block counts, so the ABI works in blocks.

### Console

Input and output each go through a 256-byte ring buffer, guarded by a pair of semaphores in a producer/consumer arrangement. Received characters are copied into the input buffer from the console's external interrupt. Output is drained by a dedicated kernel thread, which yields the CPU while the console controller is busy instead of spinning.

### Interface layers

```
user code ──► C++ API  (Thread, Semaphore, PeriodicThread, Console)   inc/syscall_cpp.hpp
          ──► C API    (thread_create, sem_wait, mem_alloc, ...)      inc/syscall_c.h
          ──► ABI      (ecall with a code in a0)                      src/syscall_c.cpp
          ──► kernel   (RiscV::handleSupervisorTrap)                  src/RiscV.cpp
```

The C++ layer also overloads `operator new`/`delete` on top of `mem_alloc`/`mem_free`, so user code can use ordinary C++ allocation.

## Project structure

```
inc/                     src/
  RiscV.hpp                RiscV.cpp           trap handler, system-call dispatch
  Thread.hpp               Thread.cpp          threads, dispatch, sleeping
  Scheduler.hpp            Scheduler.cpp       FIFO ready queue
  Semaphore.hpp            Semaphore.cpp       counting semaphores
  MemoryAllocator.hpp      MemoryAllocator.cpp first-fit heap allocator
  Console.hpp              Console.cpp         buffered console I/O
  List.hpp                 List.cpp            generic linked list (nodes on the kernel heap)
  SleepList.hpp            SleepList.cpp       delta list for sleeping threads
  syscall_c.h              syscall_c.cpp       C API → ecall
  syscall_cpp.hpp          syscall_cpp.cpp     C++ API
                           main.cpp            kernel initialisation and shutdown
                           supervisorTrap.S    trap entry/exit
                           contextSwitch.S     context switch
                           registersUtil.S     register save/restore helpers
```

About 1,600 lines of C++ and RISC-V assembly.

## Building and running

This repository contains only my own source code. Building it requires the **course development kit**, which is not mine to publish and is not included:

- hardware and console support libraries (`hw.lib`, `console.lib`) and their headers, expected in `lib/`
- the `Makefile` and linker script
- the test application, which provides `userMain()`, the entry point of the user program
- the development VM with the RISC-V toolchain and the emulator

With the kit in place, copy `src/` and `inc/` into the project directory and build and run it with the course `Makefile`. On start-up, `main()` installs the trap handler, creates the main and idle threads and calls `userMain()`. When all user threads have finished and the output is flushed, the kernel stops the emulator.
