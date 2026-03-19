# Phase 1 Discussion

## Goal

Complete the preemptive round-robin scheduler by closing the gaps between the existing task/context-switch infrastructure and the F-011 acceptance criteria. The core mechanism (circular task list, `schedule()` with round-robin selection, PIT-driven preemption via `pit_irq_handler`, assembly `switch_to`) already works. This phase hardens dead-task cleanup and adds a three-task demo to prove all ACs.

## Requirements

1. **Dead task reaping (AC-3):** When a thread finishes (`task_exit` → `TASK_DEAD`), it must be removed from the circular task list and its resources freed. Currently `schedule()` skips dead tasks but never removes them — they leak memory and bloat the walk. A deferred cleanup approach is needed since a dead task cannot free its own stack while running on it.
2. **Three-task demo (AC-2):** Replace the current two-task test (`test_task_a`, `test_task_b`) with three tasks that each print a unique ID to serial, proving all three make interleaved progress.
3. **Single-survivor correctness (AC-4):** When only one thread remains (all others finished), timer ticks must not crash. The current `schedule()` already returns early when no other READY task is found, but this needs explicit validation with a finishing-task scenario.
4. **Timer-driven preemption (AC-1):** Already implemented — PIT fires every 10 ticks (100 ms), sends early EOI, calls `schedule()`. No changes needed, just validation.

## Assumptions

- The existing `switch_to` / `schedule()` / `task_create` / `task_exit` code from F-009 and F-010 is correct and tested. This phase builds on it rather than rewriting it.
- Dead task cleanup will use a deferred reap approach: `task_exit` marks the task DEAD and yields, then the *next* `schedule()` call (or a dedicated reap pass) removes and frees dead tasks before selecting the next runnable task. This avoids freeing the stack of the currently executing task.
- The boot/idle task (id=0) is never destroyed and acts as the fallback when all user tasks finish. It sits in the idle `hlt` loop with interrupts enabled.
- 8 KiB stacks are sufficient for all test tasks.
- No priority levels or sleep/block states are needed — pure round-robin among READY tasks.

## Unknowns

- **Stack-of-dead-task problem:** When `task_exit` calls `schedule()`, the dead task's stack is still the active stack until `switch_to` swaps RSP. The reap must happen *after* the switch completes (i.e., from the new task's context). Need to confirm the deferred reap timing is safe — specifically that no stale pointer to the dead task's stack persists after `switch_to` returns on the new task.
- **Interrupt safety of list mutation:** `task_destroy` modifies the circular linked list. If a timer IRQ fires mid-mutation, `schedule()` could walk a corrupted list. Need to confirm whether `cli`/`sti` guards are needed around list operations, or whether the current single-CPU non-reentrant design makes this safe (timer IRQ calls `schedule()` which won't nest because the PIT IRQ is masked until EOI).

## Decisions

### Deferred reap in schedule()

**Phase:** 1
**Context:** A dead task cannot free its own stack because it's still executing on that stack when `task_exit` runs. The cleanup must happen after context switch.
**Decision:** Add a reap pass at the top of `schedule()` that walks the circular list, removes any TASK_DEAD entries, and frees their resources *before* selecting the next task. This is safe because `schedule()` always runs on the *current* (live) task's stack after a switch.
**Alternatives considered:**
- Separate `reap_dead_tasks()` called from the idle loop — adds complexity, delays cleanup.
- Lazy removal (just skip dead tasks forever) — leaks memory, violates AC-3.

### No interrupt guards for list mutation

**Phase:** 1
**Context:** On a single-CPU system with non-reentrant IRQ handling (PIT IRQ is masked until EOI), `schedule()` cannot be interrupted by another `schedule()` call.
**Decision:** No `cli`/`sti` guards needed around circular list operations. The PIT handler sends EOI *before* calling `schedule()`, but the PIC won't deliver another IRQ0 until the current handler returns (IRQ0 stays masked at the PIC level during servicing, and `schedule()` context-switches away before the handler returns — the handler will complete when this task is switched back in).
**Alternatives considered:**
- Wrap all list mutations in `cli`/`sti` — unnecessary overhead for single-CPU, but would be needed if SMP support is added later.
