# Decisions

### TCB stores only RSP; full context lives on the stack

**Phase:** 1
**Context:** Need to decide how much CPU state the TCB stores directly vs. on the task's kernel stack.
**Decision:** The TCB stores only `rsp` (the saved stack pointer). All other registers (RBX, RBP, R12-R15, RIP) are saved/restored on the kernel stack by the context switch code. This is the standard approach for x86_64 kernel threading.
**Alternatives considered:**
- Store all registers in the TCB struct — wasteful duplication
- Store a full `struct interrupt_frame` — too heavy for voluntary context switch

---

### Thread entry wrapper for clean exit

**Phase:** 1
**Context:** If a thread's entry function returns via `ret`, it would pop an invalid address and crash.
**Decision:** `task_create` sets up the stack so the entry function "returns" into `task_exit()`, which marks the task as `TASK_DEAD`. The scheduler will skip dead tasks.
**Alternatives considered:**
- Require entry functions to never return — fragile
- Use `cli; hlt` as the return address — no cleanup, leaks resources

---

### Module naming: `task`

**Phase:** 1
**Context:** Need a module name for the TCB and thread management code.
**Decision:** Use `task.c`/`task.h` with `task_` prefix. "Task" is the standard kernel terminology for a schedulable unit.
**Alternatives considered:**
- `thread.c` — implies user-space threading
- `tcb.c` — too implementation-specific

---
