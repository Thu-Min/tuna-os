# Phase 1 Plan

## Objective

Deliver a kernel heap allocator (`kheap.c`/`kheap.h`) providing `kmalloc()`/`kfree()` with a free-list first-fit strategy, backed by PMM+VMM for on-demand page mapping. Heap starts at 0x200000 with 64 KiB initial allocation. Verified by allocating, freeing, and re-allocating memory in QEMU with serial diagnostics.

## Tasks

### Task 1: Implement kheap module

Create `kheap.h` and `kheap.c` implementing the free-list heap allocator:

**`kheap.h`:**
- Constants: `KHEAP_START` (0x200000), `KHEAP_INITIAL_SIZE` (64 KiB), `KHEAP_MAX_SIZE` (16 MiB), `KHEAP_MIN_BLOCK` (32 bytes).
- API: `kheap_init()`, `kmalloc(uint64_t size)`, `kfree(void *ptr)`.

**`kheap.c`:**
- Block header struct: `size` (uint64_t, includes header), `free` flag (uint32_t), `next` pointer (for free list).
- `kheap_init()`:
  1. Allocate `KHEAP_INITIAL_SIZE / PAGE_SIZE` physical frames via `pmm_alloc_frame()`.
  2. Map each frame into the heap region via `vmm_map_page(KHEAP_START + offset, phys, PRESENT|WRITE)`.
  3. Initialize the free list with one large free block spanning the initial region.
  4. Log heap start address and initial size to serial.
- `kmalloc(size)`:
  1. Align size up to 8 bytes, add header size.
  2. Walk free list for first block >= requested size (first-fit).
  3. If block is large enough to split, split it and keep the remainder on the free list.
  4. If no block fits, call `kheap_expand()` to map more pages and add a new free block.
  5. Mark block as used, return pointer past header.
- `kfree(ptr)`:
  1. Retrieve header from pointer (ptr - header_size).
  2. Mark block as free.
  3. Coalesce with adjacent free blocks (walk the block list by address order).
- `kheap_expand()`:
  1. Allocate more pages via PMM+VMM at the heap's current end.
  2. Add the new region as a free block.
  3. Coalesce with the previous last block if it's free.
  4. Log expansion to serial.

#### Affected Files

- `kernel/src/kheap.h` (new)
- `kernel/src/kheap.c` (new)

#### Dependencies

- PMM (`pmm_alloc_frame()`) for backing physical frames.
- VMM (`vmm_map_page()`) for mapping heap pages.
- Serial for diagnostics.

#### Risks

- **Coalescing bugs** — adjacent block detection requires walking blocks by address, not just the free list. Use a linear walk from heap start to find neighbors.
- **Header alignment** — header must be a multiple of 8 bytes to maintain alignment guarantees. Use a 24-byte header (size: 8, free: 4, padding: 4, next: 8) or pack to 16 bytes with bitfield for free flag.

### Task 2: Integrate kheap into kernel and build

Add `kheap.c` to the Makefile's `SRCS_C` list, include `kheap.h` in `kernel.c`, and call `kheap_init()` after `vmm_init()`. Add a simple self-test: allocate a block, write to it, free it, allocate again to verify reuse.

#### Affected Files

- `kernel/Makefile`
- `kernel/src/kernel.c`

#### Dependencies

- Task 1 (kheap module must exist).

#### Risks

None — straightforward integration following the existing init pattern.

### Task 3: Build and verify in QEMU

Build and run. Verify serial output shows:
1. Heap initialized at 0x200000 with 64 KiB
2. kmalloc/kfree self-test passes
3. No crashes or faults
4. All other subsystems still functional

#### Affected Files

(No file changes — verification only)

#### Dependencies

- Tasks 1–2 complete.

#### Risks

- Page fault if heap pages aren't correctly mapped — debug via serial output before/after map calls.

## Affected Files

- `kernel/src/kheap.h`
- `kernel/src/kheap.c`
- `kernel/Makefile`
- `kernel/src/kernel.c`
