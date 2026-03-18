# Phase 1 Discussion

## Goal

Define the task control block (TCB) data structure and implement kernel thread creation/destruction APIs (`task.c`/`task.h`). Each kernel thread gets a unique ID, its own kernel stack (allocated via `kmalloc`), and a saved CPU context initialized so that context switching (F-010) can later resume it at the thread's entry function.

## Requirements

1. **TCB structure** — `struct task` containing:
   - `uint64_t id` — unique task ID (monotonically increasing counter)
   - `uint64_t rsp` — saved stack pointer (for context switch)
   - `uint64_t rip` — saved instruction pointer (for initial setup)
   - `uint8_t *stack` — pointer to the base of the kernel stack allocation (for freeing)
   - `uint32_t state` — task state (`TASK_READY`, `TASK_RUNNING`, `TASK_DEAD`)
   - `struct task *next` — linked list pointer for the scheduler's task queue (F-011)
2. **`task_create(void (*entry)(void))`** — Allocates a TCB and a kernel stack via `kmalloc`. Sets up the stack so that when a context switch restores it, execution begins at `entry`. Returns a pointer to the new `struct task`.
3. **`task_destroy(struct task *t)`** — Frees the task's kernel stack and TCB via `kfree`.
4. **Stack frame setup** — The initial stack must be laid out so that the context switch code (F-010) can `pop` saved registers and `ret` into the entry function. This means pushing a fake return address and initial register values onto the stack.
5. **Task list** — Maintain a global linked list of tasks for enumeration and scheduling.
6. **Boot task** — `task_init()` wraps the current `kernel_main` execution context as "task 0" (the idle/boot task) so the scheduler can treat it uniformly.
7. **Serial diagnostics** — Log task creation (ID, entry address, stack address) and destruction to serial.

## Assumptions

- **Kernel stack size** — 8 KiB per task. Sufficient for kernel-mode execution without deep recursion.
- **No preemption yet** — Tasks are created but not scheduled in this feature. Context switching (F-010) and the scheduler (F-011) will use the TCB and task list.
- **x86_64 calling convention** — The context switch will save/restore callee-saved registers (RBX, RBP, R12-R15, RSP, RIP). The initial stack frame must match this convention.
- **All tasks are kernel threads** — Ring 0 only. User-mode tasks come in F-012.
- **Task IDs start at 0** — The boot task is ID 0, first created thread is ID 1.

## Unknowns

- **Exact context switch register set** — The saved context layout depends on how F-010 implements `switch_to()`. The TCB stores `rsp` as the key field; the rest of the context lives on the task's stack. The initial stack setup must match what the context switch code expects to pop.
- **Thread entry wrapper** — Should threads return to a cleanup function, or must entry functions never return? Decision: use a wrapper that calls `task_exit()` after the entry function returns, preventing a bare `ret` into garbage.

## Decisions

### TCB stores only RSP; full context lives on the stack

**Phase:** 1
**Context:** Need to decide how much CPU state the TCB stores directly vs. on the task's kernel stack.
**Decision:** The TCB stores only `rsp` (the saved stack pointer). All other registers (RBX, RBP, R12-R15, RIP) are saved/restored on the kernel stack by the context switch code. This is the standard approach for x86_64 kernel threading — the context switch pushes callee-saved registers, saves RSP to the old TCB, loads RSP from the new TCB, and pops registers.
**Alternatives considered:**
- Store all registers in the TCB struct — wasteful duplication since they must be on the stack anyway for `push`/`pop` to work
- Store a full `struct interrupt_frame` — too heavy, designed for interrupt entry not voluntary context switch

### Thread entry wrapper for clean exit

**Phase:** 1
**Context:** If a thread's entry function returns via `ret`, it would pop an invalid address and crash. Need a safe exit path.
**Decision:** `task_create` sets up the stack so the entry function "returns" into `task_exit()`, which marks the task as `TASK_DEAD`. The scheduler (F-011) will skip dead tasks and eventually reap them.
**Alternatives considered:**
- Require entry functions to never return (infinite loop) — fragile, easy to forget
- Use `cli; hlt` as the return address — no cleanup, leaks resources

### Module naming: `task`

**Phase:** 1
**Context:** Need a module name for the TCB and thread management code.
**Decision:** Use `task.c`/`task.h` with `task_` prefix (`task_create`, `task_destroy`, `task_init`, `task_exit`). "Task" is the standard kernel terminology for a schedulable unit.
**Alternatives considered:**
- `thread.c` — implies user-space threading, which comes later
- `tcb.c` — too implementation-specific
