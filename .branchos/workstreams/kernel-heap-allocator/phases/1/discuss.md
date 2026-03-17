# Phase 1 Discussion

## Goal

Implement a kernel heap allocator (`kheap.c`/`kheap.h`) providing `kmalloc()`/`kfree()` for arbitrary-sized dynamic memory allocation. The heap occupies a dedicated virtual address range above the kernel image, backed by physical frames allocated via PMM and mapped via VMM on demand.

## Requirements

1. **Heap region** — Reserve a fixed virtual address range starting at a page-aligned address above `_kernel_end` (e.g., `KHEAP_START = 0x200000` — 2 MiB). The heap grows upward as more memory is needed.
2. **Initial mapping** — `kheap_init()` maps an initial set of pages (e.g., 64 KiB = 16 pages) into the heap region using `pmm_alloc_frame()` + `vmm_map_page()`.
3. **Free-list allocator** — Use a simple linked free-list (first-fit) algorithm. Each free block has a header with size and a pointer to the next free block. Minimum allocation alignment is 8 bytes.
4. **`kmalloc(size)`** — Walks the free list for the first block that fits. Splits the block if there's enough leftover. Returns a pointer past the header. If no block fits, expands the heap by mapping more pages.
5. **`kfree(ptr)`** — Marks the block as free and inserts it back into the free list. Coalesces adjacent free blocks to reduce fragmentation.
6. **Heap expansion** — When `kmalloc` can't find a large enough free block, allocate more physical pages via PMM, map them into the heap region via VMM, and add the new region to the free list.
7. **Serial diagnostics** — Log heap init (start address, initial size) and expansion events to serial.

## Assumptions

- **Identity-mapped heap** — Since the kernel is identity-mapped and the heap start is within the first 1 GiB, the VMM identity-maps heap pages (virt == phys). This simplifies pointer arithmetic.
- **No concurrency** — Single-threaded kernel, no locking needed on the free list.
- **Block header overhead** — Each allocation has a small header (16 bytes: size + flags). This is acceptable for a kernel heap.
- **No page-level freeing** — `kfree` returns memory to the free list but does not unmap pages back to PMM. This avoids complexity and is standard for kernel heaps.
- **Heap start at 2 MiB** — Above the kernel image (which starts at 1 MiB and currently ends around 0x10D000). 2 MiB provides ample gap and is page-aligned.

## Unknowns

- **Maximum heap size** — For now, allow the heap to grow up to a configurable limit (e.g., `KHEAP_MAX_SIZE = 16 MiB`). If exceeded, `kmalloc` returns NULL.
- **Alignment requirements** — 8-byte alignment is sufficient for most kernel structures. 16-byte alignment could be added later if needed.

## Decisions

### Free-list first-fit allocator

**Phase:** 1
**Context:** Need to choose a heap allocation strategy that's simple to implement and debug in a freestanding kernel.
**Decision:** Use a singly-linked free-list with first-fit search. Each free block stores its size and a next pointer in a header. Split blocks when allocation is smaller than the block. Coalesce adjacent free blocks on `kfree`.
**Alternatives considered:**
- Buddy allocator — more complex, better for page-sized allocations but overkill for general-purpose kmalloc
- Slab allocator — great for fixed-size objects but we need variable-size allocation first
- Bump allocator — too simple, no freeing capability

### Heap placement at 2 MiB virtual address

**Phase:** 1
**Context:** Need a virtual address range for the heap that doesn't conflict with the kernel image (1 MiB - ~1.1 MiB).
**Decision:** Start the heap at virtual address 0x200000 (2 MiB). This provides clear separation from the kernel image and is naturally page-aligned.
**Alternatives considered:**
- Immediately after `_kernel_end` — too tight, no buffer for kernel growth
- Higher-half addresses (e.g., 0xFFFF800000000000) — requires non-identity mapping, adds complexity not needed yet

### Module naming: `kheap` (kernel heap)

**Phase:** 1
**Context:** Need a module name following the project's naming convention.
**Decision:** Use `kheap.c`/`kheap.h` with `kheap_` prefix for init, and standard `kmalloc`/`kfree` names (universally recognized in kernel development).
**Alternatives considered:**
- `heap.c` — less specific, could conflict with user-space heap later
- `alloc.c` — too generic
