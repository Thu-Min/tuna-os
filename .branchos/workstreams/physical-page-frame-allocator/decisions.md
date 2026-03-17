# Decisions

### Static bitmap with fixed maximum

**Phase:** 1
**Context:** Need to decide how to size the bitmap without dynamic allocation.
**Decision:** Use a statically-allocated bitmap in `.bss` sized to `PMM_MAX_FRAMES` (e.g., 32,768 for 128 MiB). Mark all bits as "used" initially, then clear bits for usable RAM regions during init. This "default used" approach is safer — unknown memory is never accidentally allocated.
**Alternatives considered:**
- Dynamically compute bitmap size from memory map — adds complexity, no real benefit at this stage
- Linked free list — more memory-efficient for sparse memory but harder to implement and debug

---

### Module naming: `pmm` (physical memory manager)

**Phase:** 1
**Context:** Need a module name following the project's `module_function()` convention.
**Decision:** Use `pmm.c`/`pmm.h` with functions prefixed `pmm_` (e.g., `pmm_init`, `pmm_alloc_frame`, `pmm_free_frame`).
**Alternatives considered:**
- `frame.c` / `frame_alloc()` — less descriptive of the subsystem's scope
- `palloc.c` — less conventional

---
