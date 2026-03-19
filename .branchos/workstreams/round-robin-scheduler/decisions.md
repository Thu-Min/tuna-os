# Decisions

### Deferred reap in schedule()

**Phase:** 1
**Context:** A dead task cannot free its own stack because it's still executing on that stack when `task_exit` runs. The cleanup must happen after context switch.
**Decision:** Add a reap pass at the top of `schedule()` that walks the circular list, removes any TASK_DEAD entries, and frees their resources before selecting the next task. This is safe because `schedule()` always runs on the current (live) task's stack after a switch.
**Alternatives considered:**
- Separate `reap_dead_tasks()` called from the idle loop — adds complexity, delays cleanup.
- Lazy removal (just skip dead tasks forever) — leaks memory, violates AC-3.

---

### No interrupt guards for list mutation

**Phase:** 1
**Context:** On a single-CPU system with non-reentrant IRQ handling (PIT IRQ is masked until EOI), `schedule()` cannot be interrupted by another `schedule()` call.
**Decision:** No `cli`/`sti` guards needed around circular list operations. The PIC won't deliver another IRQ0 until the current handler returns.
**Alternatives considered:**
- Wrap all list mutations in `cli`/`sti` — unnecessary overhead for single-CPU, but would be needed if SMP support is added later.

---
