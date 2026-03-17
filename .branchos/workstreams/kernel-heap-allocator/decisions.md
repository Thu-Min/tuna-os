# Decisions

### Free-list first-fit allocator

**Phase:** 1
**Context:** Need to choose a heap allocation strategy that's simple to implement and debug in a freestanding kernel.
**Decision:** Use a singly-linked free-list with first-fit search. Each free block stores its size and a next pointer in a header. Split blocks when allocation is smaller than the block. Coalesce adjacent free blocks on `kfree`.
**Alternatives considered:**
- Buddy allocator — more complex, better for page-sized allocations but overkill for general-purpose kmalloc
- Slab allocator — great for fixed-size objects but we need variable-size allocation first
- Bump allocator — too simple, no freeing capability

---

### Heap placement at 2 MiB virtual address

**Phase:** 1
**Context:** Need a virtual address range for the heap that doesn't conflict with the kernel image (1 MiB - ~1.1 MiB).
**Decision:** Start the heap at virtual address 0x200000 (2 MiB). This provides clear separation from the kernel image and is naturally page-aligned.
**Alternatives considered:**
- Immediately after `_kernel_end` — too tight, no buffer for kernel growth
- Higher-half addresses (e.g., 0xFFFF800000000000) — requires non-identity mapping, adds complexity not needed yet

---

### Module naming: `kheap` (kernel heap)

**Phase:** 1
**Context:** Need a module name following the project's naming convention.
**Decision:** Use `kheap.c`/`kheap.h` with `kheap_` prefix for init, and standard `kmalloc`/`kfree` names (universally recognized in kernel development).
**Alternatives considered:**
- `heap.c` — less specific, could conflict with user-space heap later
- `alloc.c` — too generic

---
