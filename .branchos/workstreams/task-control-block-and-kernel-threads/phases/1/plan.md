# Phase 1 Plan

## Objective

Deliver the task control block (TCB) data structure and kernel thread creation/destruction APIs (`task.c`/`task.h`). Threads are created with their own kernel stack and saved register state ready for context switching (F-010). A boot task wraps `kernel_main`. Verified by creating multiple tasks and logging their IDs and stack addresses in QEMU.

## Tasks

### Task 1: Implement task module

Create `task.h` and `task.c`:

**`task.h`:**
- Task states: `TASK_READY`, `TASK_RUNNING`, `TASK_DEAD`
- `TASK_STACK_SIZE` (8192 = 8 KiB)
- `struct task`: `id`, `rsp`, `state`, `stack` (base pointer for freeing), `next`
- API: `task_init()`, `task_create(void (*entry)(void))`, `task_destroy(struct task *t)`, `task_exit()`
- `task_get_current()`, `task_get_list()` for scheduler access

**`task.c`:**
- Static `next_id` counter, `current_task` pointer, `task_list` head pointer
- `task_init()`: allocates a `struct task` for the boot task (ID 0), sets state to `TASK_RUNNING`, sets it as current and as the task list head. Does NOT allocate a stack — the boot task uses the existing boot stack.
- `task_create(entry)`:
  1. Allocates `struct task` via `kmalloc`
  2. Allocates 8 KiB kernel stack via `kmalloc`
  3. Sets up the stack with initial values for callee-saved registers (RBX, RBP, R12-R15 = 0) and a return address. The stack layout from top:
     - `task_exit` address (fake return address for when `entry` returns)
     - `entry` address (where context switch will `ret` into)
     - 6 × 0 (RBX, RBP, R12, R13, R14, R15)
  4. Sets `rsp` to point at the bottom of the pushed values
  5. Sets state to `TASK_READY`, appends to task list
  6. Logs creation to serial
- `task_destroy(t)`: frees stack and TCB via `kfree`, removes from task list
- `task_exit()`: marks current task as `TASK_DEAD`, logs to serial, enters `hlt` loop (scheduler will reap it in F-011)

#### Affected Files

- `kernel/src/task.h` (new)
- `kernel/src/task.c` (new)

#### Dependencies

- `kheap.h` (`kmalloc`/`kfree`) for TCB and stack allocation
- `serial.h` for diagnostics

#### Risks

- **Stack layout must match F-010's context switch** — the initial register push order must exactly match what `switch_to` expects to pop. Document the layout clearly in comments.
- **Stack alignment** — x86_64 ABI requires 16-byte aligned RSP at `call` instruction. The initial RSP must satisfy this after the fake `push` sequence.

### Task 2: Integrate task module into kernel and build

Add `task.c` to the Makefile's `SRCS_C` list, include `task.h` in `kernel.c`, and call `task_init()` after `kheap_init()`. Create 2 test tasks with dummy entry functions that log to serial, demonstrating task creation and enumeration.

#### Affected Files

- `kernel/Makefile`
- `kernel/src/kernel.c`

#### Dependencies

- Task 1 (task module must exist)

#### Risks

None — tasks are created but not yet switched to (that's F-010).

### Task 3: Build and verify in QEMU

Build and run. Verify serial output shows:
1. Boot task (ID 0) created
2. Test tasks created with unique IDs, distinct stack addresses
3. No crashes
4. All other subsystems still functional

#### Affected Files

(No file changes — verification only)

#### Dependencies

- Tasks 1–2 complete

#### Risks

None — no context switches happen yet, so tasks don't execute.

## Affected Files

- `kernel/src/task.h`
- `kernel/src/task.c`
- `kernel/Makefile`
- `kernel/src/kernel.c`
