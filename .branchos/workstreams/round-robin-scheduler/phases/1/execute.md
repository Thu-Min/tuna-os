# Phase 1 Execution

## Completed Tasks

### Task 1: Add dead-task reaping to schedule()
- **Status:** Complete
- **Commits:** f1ee959
- **Notes:** Added reap pass at top of `schedule()` that walks the circular list and calls `task_destroy()` on any `TASK_DEAD` entries, skipping `current_task`. Uses saved `next` pointer to avoid invalidation during list mutation.

### Task 2: Add third test task and update kernel_main demo
- **Status:** Complete
- **Commits:** f1ee959
- **Notes:** Added `test_task_c` (infinite loop printing `[C]`). Made `test_task_b` finite (20 iterations then return). Three tasks created in `kernel_main`.

### Task 3: Build and validate in QEMU
- **Status:** Complete
- **Commits:** N/A (validation only)
- **Notes:** QEMU serial output confirms all 4 ACs:
  - AC-1: Timer-driven preemption switches between tasks at PIT intervals
  - AC-2: Interleaved `[A]`, `[B]`, `[C]` output from all three tasks
  - AC-3: `[B done]` / `task: exit id=2` / `task: destroyed id=2` — dead task reaped
  - AC-4: After B exits, `[A]` and `[C]` continue alternating indefinitely through tick 1000+

## Blockers

None
