# Phase 1 Discussion

## Goal

Implement a bitmap-based physical page frame allocator (`pmm.c/pmm.h`) that initializes from the Multiboot2 memory map (F-005), tracks 4 KiB page frames, and exposes `alloc_frame`/`free_frame` APIs for use by future virtual memory management.

## Requirements

1. **Bitmap data structure** — A static bitmap where each bit represents one 4 KiB physical page frame. Bit 0 = free, bit 1 = used.
2. **Initialization from memory map** — `pmm_init()` iterates the regions from `multiboot2_get_mmap()`, marks all pages in usable RAM regions as free, and marks everything else (reserved, ACPI, bad RAM) as used.
3. **Kernel region protection** — Pages overlapping the kernel image (from `_kernel_start` to `_kernel_end` linker symbols) must be marked as used during init, even if they fall within a usable RAM region.
4. **Low memory protection** — Pages below 1 MiB should be marked as used (real-mode IVT, BDA, VGA, BIOS ROM, etc.) regardless of memory map type.
5. **`alloc_frame()`** — Scans the bitmap for the first free bit, marks it used, and returns the physical address. Returns 0 (NULL) if no free frames remain.
6. **`free_frame(addr)`** — Clears the bit for the given physical address, making it available for future allocation.
7. **Serial diagnostics** — After init, log total frames, used frames, and free frames to serial for verification.
8. **Integration** — Call `pmm_init()` from `kernel_main()` after `multiboot2_parse_mmap()` and before any future subsystem that needs physical pages.

## Assumptions

- **Maximum physical memory**: The bitmap will be statically sized. Assuming a reasonable upper bound (e.g., 128 MiB = 32,768 pages = 4 KiB bitmap) is sufficient for the current QEMU configuration. A `PMM_MAX_FRAMES` constant will control this.
- **No concurrency**: No locking needed — the kernel is single-threaded with no preemptive multitasking yet.
- **Identity-mapped memory**: The kernel runs in an identity-mapped 1 GiB region (set up in `boot.S`), so physical addresses can be used directly as pointers.
- **Static bitmap placement**: The bitmap lives as a static array in `.bss` — no dynamic allocation chicken-and-egg problem.
- **4 KiB page size**: Standard x86 page size, matching the linker script's 4K section alignment.

## Unknowns

- **Bitmap size vs real RAM**: Should the bitmap cover only detected RAM or up to `PMM_MAX_FRAMES`? Covering a fixed maximum is simpler; bits for non-existent memory just stay "used".
- **Linker symbols**: The linker script currently does not export `_kernel_start` / `_kernel_end`. These need to be added to `linker.ld`.

## Decisions

### Static bitmap with fixed maximum

**Phase:** 1
**Context:** Need to decide how to size the bitmap without dynamic allocation.
**Decision:** Use a statically-allocated bitmap in `.bss` sized to `PMM_MAX_FRAMES` (e.g., 32,768 for 128 MiB). Mark all bits as "used" initially, then clear bits for usable RAM regions during init. This "default used" approach is safer — unknown memory is never accidentally allocated.
**Alternatives considered:**
- Dynamically compute bitmap size from memory map — adds complexity, no real benefit at this stage
- Linked free list — more memory-efficient for sparse memory but harder to implement and debug

### Module naming: `pmm` (physical memory manager)

**Phase:** 1
**Context:** Need a module name following the project's `module_function()` convention.
**Decision:** Use `pmm.c`/`pmm.h` with functions prefixed `pmm_` (e.g., `pmm_init`, `pmm_alloc_frame`, `pmm_free_frame`).
**Alternatives considered:**
- `frame.c` / `frame_alloc()` — less descriptive of the subsystem's scope
- `palloc.c` — less conventional
