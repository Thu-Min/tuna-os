# Phase 1 Plan

## Objective

Implement a low-level context switch (`switch_to` in assembly) and a round-robin `schedule()` function, integrated with the PIT timer IRQ to preemptively multiplex the CPU between kernel threads. Demonstrate with two threads producing interleaved serial output.

## Tasks

### Task 1: Create `switch_to` assembly routine

Implement `switch_to(uint64_t *old_rsp, uint64_t new_rsp)` in a new assembly file. The routine:
1. Pushes callee-saved registers (RBX, RBP, R12-R15) onto the current stack
2. Saves RSP into `*old_rsp`
3. Loads RSP from `new_rsp`
4. Pops callee-saved registers from the new stack
5. `ret`s into the new task's instruction stream

This matches the stack layout already set up by `task_create()` in F-009.

#### Affected Files

- `kernel/src/switch.S`

#### Dependencies

None — standalone assembly, no imports.

#### Risks

Register ordering must exactly match the push order in `task_create()` (RBX, RBP, R12, R13, R14, R15 bottom-to-top). A mismatch will corrupt registers or crash.

### Task 2: Implement `schedule()` and `yield()` in task module

Add `schedule()` to `task.c`:
1. Walk the circular task list from `current_task->next`
2. Skip TASK_DEAD tasks
3. If a READY task is found, transition current from RUNNING→READY, next from READY→RUNNING, call `switch_to`
4. If no other runnable task exists (only current is alive), return without switching

Add `yield()` as a public wrapper that simply calls `schedule()` — provides a cooperative yield point for future use.

Declare `switch_to` as `extern void switch_to(uint64_t *old_rsp, uint64_t new_rsp)` in `task.h`.

#### Affected Files

- `kernel/src/task.c`
- `kernel/src/task.h`

#### Dependencies

Task 1 (switch_to assembly must exist).

#### Risks

Must handle the case where current_task is the only runnable task (no-op). Must handle circular list correctly to avoid infinite loop if all tasks are dead except current.

### Task 3: Hook scheduling into PIT timer IRQ

Modify `pit.c` to call `schedule()` every N ticks (10 ticks = 100 ms at 100 Hz). Add a `#include "task.h"` and call `schedule()` from `pit_irq_handler` when `tick_count % 10 == 0`.

#### Affected Files

- `kernel/src/pit.c`

#### Dependencies

Task 2 (schedule function must exist).

#### Risks

Context switch from inside an IRQ handler means we switch stacks while the IRQ is in progress. This is safe because: (a) interrupts are already disabled during the IRQ handler (x86 clears IF on interrupt entry), (b) each task has its own kernel stack, and (c) the PIC EOI is sent before dispatch returns for the new task. Must verify EOI timing — the PIC EOI must be sent before `schedule()` is called, or the timer will stop firing for the switched-to task.

### Task 4: Update demo tasks to loop with serial output

Replace the current one-shot `test_task_a` and `test_task_b` in `kernel.c` with infinite loops that print a marker string and busy-wait, so interleaved output is visible:

```c
static void test_task_a(void) {
    for (;;) {
        serial_write("[A] ");
        for (volatile int i = 0; i < 500000; i++);
    }
}
```

Also move `task_init()` and `task_create()` calls to after PIT init so the scheduler is ready before tasks are created, and ensure interrupts are enabled before entering the idle loop.

#### Affected Files

- `kernel/src/kernel.c`

#### Dependencies

Tasks 1-3 (full context switch pipeline must be in place).

#### Risks

Busy-wait loop iteration count may need tuning for QEMU speed. Too fast → output floods serial; too slow → no visible interleaving within a reasonable test window.

### Task 5: Add `switch.S` to build and ensure EOI ordering

Add `src/switch.S` to `SRCS_S` in the Makefile. Also ensure PIT sends EOI before calling `schedule()` — reorder `pit_irq_handler` so `pic_eoi(0)` happens before `schedule()`, or verify the existing ISR stub sends EOI after dispatch (check `interrupts.S` flow).

#### Affected Files

- `kernel/Makefile`
- `kernel/src/pit.c`

#### Dependencies

Task 1 (switch.S must exist to compile).

#### Risks

If EOI is sent by `isr_common_stub` after `isr_dispatch` returns, and `schedule()` switches away before returning, EOI never fires for that interrupt — the timer stops. May need to send EOI explicitly in the PIT handler before calling `schedule()`.

## Affected Files

- `kernel/src/switch.S`
- `kernel/src/task.c`
- `kernel/src/task.h`
- `kernel/src/pit.c`
- `kernel/src/kernel.c`
- `kernel/Makefile`
