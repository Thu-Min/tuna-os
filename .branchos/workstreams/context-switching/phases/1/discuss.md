# Phase 1 Discussion

## Goal

Implement a low-level context switch that saves and restores CPU register state to multiplex the CPU between kernel threads created by the TCB subsystem (F-009). Add a simple round-robin `schedule()` function and integrate with the PIT timer to drive preemptive switching.

## Requirements

1. **Assembly `switch_to(old, new)` routine** — saves all callee-saved registers (RBX, RBP, R12-R15) and RSP into the old task's TCB, restores them from the new task's TCB, and `ret`s into the new task's instruction stream.
2. **`schedule()` function** — walks the circular task list to find the next READY task, transitions states (RUNNING → READY, READY → RUNNING), and calls `switch_to`.
3. **Timer-driven switching** — hook `schedule()` into the PIT IRQ0 handler so context switches happen on every N-th tick (e.g., every 10 ticks = 100 ms at 100 Hz).
4. **Two-thread serial demo** — create two kernel threads that print distinct markers to serial in a loop, demonstrating interleaved output as proof of working context switches.
5. **Dead task skipping** — `schedule()` must skip tasks in TASK_DEAD state when scanning for the next runnable task.

## Assumptions

1. The TCB stack layout from F-009 is already compatible with the context switch — 6 callee-saved registers are pre-pushed, with the entry function as the `ret` target. No changes to `task.c` stack setup should be needed.
2. The boot/idle task (task 0) participates in round-robin scheduling like any other task. Its RSP field gets populated on first switch-away.
3. Only callee-saved registers need explicit save/restore. The System V AMD64 ABI guarantees that caller-saved registers (RAX, RCX, RDX, RSI, RDI, R8-R11) are already clobbered across function calls, and the context switch is entered via a function call.
4. No FPU/SSE state saving is needed — kernel threads don't use floating point.
5. Interrupts are disabled during the context switch to prevent re-entrant scheduling from a nested timer IRQ.
6. `switch_to` is a standalone assembly file (`switch.S`) following the same pattern as `gdt_flush.S` and `interrupts.S`.

## Unknowns

1. **Tick frequency for scheduling** — should we switch every tick (10 ms) or every N ticks? Starting with every 10 ticks (100 ms) is simple and makes interleaved output easy to observe. Can tune later.
2. **Dead task reaping** — `task_exit()` currently halts. Should `schedule()` reap dead tasks (call `task_destroy`) or leave that for the scheduler feature (F-011)? Leaning toward skip-only for now, defer reaping to F-011.

## Decisions

### Context switch saves only callee-saved registers

**Phase:** 1
**Context:** Need to decide what register state to save/restore during a context switch.
**Decision:** Save/restore only the 6 callee-saved registers (RBX, RBP, R12-R15) plus RSP. The switch is always called from C code, so caller-saved registers are already handled by the compiler.
**Alternatives considered:**
- Save all general-purpose registers (unnecessary overhead, the ABI handles caller-saved)
- Use `iret`-based switching (more complex, needed for user-mode but not kernel-to-kernel)

### Timer-driven preemptive scheduling via PIT hook

**Phase:** 1
**Context:** Need a mechanism to trigger context switches without cooperative yielding.
**Decision:** Call `schedule()` from the PIT IRQ handler every N ticks. This reuses existing PIT infrastructure and provides preemptive multitasking.
**Alternatives considered:**
- Cooperative-only yielding (simpler but doesn't satisfy AC-4's interleaved output requirement without explicit yield calls in loops)
- Separate timer for scheduling (unnecessary complexity, PIT is already running)

### Separate switch.S assembly file

**Phase:** 1
**Context:** The `switch_to` function must be in assembly for direct register manipulation.
**Decision:** Create `kernel/src/switch.S` as a new assembly file, matching the project convention of dedicated `.S` files for assembly routines.
**Alternatives considered:**
- Inline assembly in C (fragile, hard to control register allocation for this use case)
- Add to existing `interrupts.S` (unrelated concern, better separation)
