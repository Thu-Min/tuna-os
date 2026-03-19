# Phase 1 Plan

## Objective

Complete the preemptive round-robin scheduler by adding dead-task reaping to `schedule()` and a three-task demo to `kernel_main`, satisfying all four F-011 acceptance criteria.

## Tasks

### Task 1: Add dead-task reaping to schedule()

Add a reap pass at the top of `schedule()` that walks the circular task list, removes any `TASK_DEAD` entries, and frees their resources via `task_destroy()`. This must happen before the round-robin selection so dead tasks are never considered.

The reap must skip `current_task` — a dead current task will be reaped on the *next* `schedule()` call after `switch_to` has moved execution to a different stack.

#### Affected Files

- `kernel/src/task.c`

#### Dependencies

None — builds on existing `task_destroy()` and `schedule()`.

#### Risks

- Walking and mutating the circular list in the same loop requires care to avoid invalidating the iteration pointer. Use a `next` pointer saved before destroying.

### Task 2: Add third test task and update kernel_main demo

Replace the two-task demo (`test_task_a`, `test_task_b`) with three tasks (A, B, C) that each print a unique ID to serial in a loop. At least one task should be finite (exits after N iterations) to exercise dead-task reaping and the single-survivor scenario.

Layout:
- Task A: infinite loop, prints `[A]`
- Task B: finite loop (e.g., 20 iterations), prints `[B]`, then returns (triggering `task_exit`)
- Task C: infinite loop, prints `[C]`

This proves AC-1 (timer-driven preemption), AC-2 (three tasks making progress), AC-3 (finished task removed), and AC-4 (remaining tasks continue after B exits).

#### Affected Files

- `kernel/src/kernel.c`

#### Dependencies

Task 1 (dead-task reaping must work for the finite task to be cleaned up).

#### Risks

- None significant. The test is straightforward serial output verification.

### Task 3: Build and validate in QEMU

Build the kernel and boot in QEMU. Verify serial output shows:
1. Interleaved `[A]`, `[B]`, `[C]` output (AC-1, AC-2)
2. `[B]` stops appearing after its iterations complete (AC-3)
3. `[A]` and `[C]` continue indefinitely after B exits (AC-4)
4. No crashes or hangs

#### Affected Files

- `kernel/Makefile`

#### Dependencies

Tasks 1 and 2.

#### Risks

- QEMU timing differences vs real hardware could mask race conditions, but acceptable for single-CPU validation.

## Affected Files

- `kernel/src/task.c`
- `kernel/src/kernel.c`
- `kernel/Makefile`
