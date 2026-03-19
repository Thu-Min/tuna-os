# Phase 1 Execution

## Completed Tasks

### Task 1: Create `switch_to` assembly routine
- **Status:** Complete
- **Commits:** c04970a
- **Notes:** Created `kernel/src/switch.S` with push/pop of 6 callee-saved registers + RSP swap.

### Task 2: Implement `schedule()` and `yield()` in task module
- **Status:** Complete
- **Commits:** c04970a
- **Notes:** Added `schedule()` (round-robin, skip DEAD), `yield()`, `switch_to` extern declaration. Added `task_trampoline()` to enable interrupts for newly created tasks via R12 register passing. Fixed stack layout bug where register order didn't match switch.S pop order.

### Task 3: Hook scheduling into PIT timer IRQ
- **Status:** Complete
- **Commits:** c04970a
- **Notes:** Calls `schedule()` every 10 ticks. Sends `pic_eoi(0)` before `schedule()` to prevent timer stall when context switch doesn't return immediately.

### Task 4: Update demo tasks to loop with serial output
- **Status:** Complete
- **Commits:** c04970a
- **Notes:** Two infinite-loop tasks printing `[A]` and `[B]` with busy-wait. Moved task init after PIT init for proper ordering.

### Task 5: Add `switch.S` to build and ensure EOI ordering
- **Status:** Complete
- **Commits:** c04970a
- **Notes:** Added `src/switch.S` to `SRCS_S` in Makefile. EOI ordering handled in Task 3.

## Blockers

None

## Verification

Tested in QEMU (EFI mode, q35, TCG). Serial output shows interleaved `[A]` and `[B]` markers with consistent switching at 100ms intervals. Timer ticks continue uninterrupted through context switches (tick=100, 200, ..., 1000+ observed). No crashes or exceptions.
