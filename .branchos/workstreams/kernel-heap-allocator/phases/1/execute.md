# Phase 1 Execution

## Completed Tasks

### Task 1: Implement kheap module
- **Status:** Complete
- **Commits:** 07af4ee
- **Notes:** Created `kheap.c`/`kheap.h` with free-list first-fit allocator. 16-byte block headers (size + free flag). `kmalloc` walks blocks by address for first-fit, splits oversized blocks. `kfree` marks block free and coalesces adjacent free blocks. `kheap_expand()` maps additional pages on demand via PMM+VMM.

### Task 2: Integrate kheap into kernel and build
- **Status:** Complete
- **Commits:** 07af4ee
- **Notes:** Added `src/kheap.c` to `SRCS_C`. Added `kheap_init()` and self-test in `kernel_main()` after VMM init. Also fixed VMM to map 4 MiB (was only mapping to `_kernel_end` ~0x10D000), which was needed because heap pages and page table frames allocated by PMM were above the mapped range.

### Task 3: Build and verify in QEMU
- **Status:** Complete
- **Notes:** QEMU EFI boot verified. Serial output confirms:
  - Heap initialized at 0x200000 with 64 KiB
  - kmalloc self-test: alloc a=0x200010, alloc b=0x200060, freed a, alloc c=0x200010 (reused freed block)
  - All subsystems functional: GDT, IDT, int3, PIC, PIT (timer ticks at 100 Hz)

## Blockers

None
