# Phase 1 Plan

## Objective

Deliver a bitmap-based physical page frame allocator (`pmm.c`/`pmm.h`) that initializes from the Multiboot2 memory map, protects kernel and low-memory regions, and exposes `pmm_alloc_frame()`/`pmm_free_frame()` APIs. Verified by serial output showing frame counts.

## Tasks

### Task 1: Add kernel boundary linker symbols

Export `_kernel_start` and `_kernel_end` symbols from `linker.ld` so the PMM can determine which physical pages overlap the kernel image and mark them as used.

#### Affected Files

- `kernel/linker.ld`

#### Dependencies

None — this is a standalone linker script change.

#### Risks

None — adding symbols does not change memory layout or section ordering.

### Task 2: Implement PMM module

Create `pmm.h` and `pmm.c` implementing the bitmap-based physical page frame allocator:

- **`pmm.h`**: Public API — `pmm_init(const struct mmap_region *regions, uint32_t count)`, `pmm_alloc_frame()`, `pmm_free_frame(uint64_t addr)`, `pmm_get_free_count()`, `pmm_get_total_count()`. Constants: `PMM_PAGE_SIZE` (4096), `PMM_MAX_FRAMES` (32768 = 128 MiB).
- **`pmm.c`**: Static bitmap array (`uint8_t bitmap[PMM_MAX_FRAMES / 8]`). Init routine: set all bits to 1 (used), then iterate usable regions clearing bits for free pages, then re-mark kernel region and pages below 1 MiB as used. `alloc_frame` scans for first zero bit, sets it, returns physical address. `free_frame` clears the corresponding bit.
- Serial diagnostics after init: total frames, used frames, free frames.

#### Affected Files

- `kernel/src/pmm.h` (new)
- `kernel/src/pmm.c` (new)

#### Dependencies

- Task 1 (linker symbols for kernel bounds)
- `multiboot2.h` (for `struct mmap_region` and memory type constants)
- `serial.h` (for diagnostic output)

#### Risks

- Off-by-one in page-to-bit mapping — mitigated by careful `addr / PAGE_SIZE` arithmetic and serial verification.
- Bitmap not large enough if QEMU is configured with >128 MiB — frames beyond `PMM_MAX_FRAMES` are simply ignored (safe, just not allocatable).

### Task 3: Integrate PMM into kernel and build

Add `pmm.c` to the Makefile's `SRCS_C` list, include `pmm.h` in `kernel.c`, and call `pmm_init()` after `multiboot2_parse_mmap()` / `multiboot2_print_mmap()`.

#### Affected Files

- `kernel/Makefile`
- `kernel/src/kernel.c`

#### Dependencies

- Task 2 (PMM module must exist)

#### Risks

None — straightforward integration following the existing init pattern.

### Task 4: Build and verify in QEMU

Build with `make -C kernel clean all iso` and run with `make -C kernel run-bios`. Verify serial output shows:
1. Memory map regions (from F-005)
2. PMM init diagnostics: total, used, and free frame counts
3. No crashes or faults

#### Affected Files

(No file changes — verification only)

#### Dependencies

- Tasks 1–3 complete

#### Risks

- Build may fail if `-Werror` catches unused variable warnings — fix inline.

## Affected Files

- `kernel/linker.ld`
- `kernel/src/pmm.h`
- `kernel/src/pmm.c`
- `kernel/Makefile`
- `kernel/src/kernel.c`
