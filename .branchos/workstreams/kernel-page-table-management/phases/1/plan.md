# Phase 1 Plan

## Objective

Deliver a virtual memory manager (`vmm.c`/`vmm.h`) that builds a new 4-level page table from C, identity-maps the kernel region with 4 KiB pages, switches CR3, and exposes `vmm_map_page()`/`vmm_unmap_page()` APIs for on-demand mapping. Verified by successful CR3 switch with continued kernel execution in QEMU.

## Tasks

### Task 1: Implement VMM module

Create `vmm.h` and `vmm.c` implementing the 4-level page table manager:

**`vmm.h`:**
- Page table entry flag constants: `VMM_FLAG_PRESENT` (1), `VMM_FLAG_WRITE` (2), `VMM_FLAG_USER` (4).
- `VMM_PAGE_SIZE` (4096).
- API: `vmm_init()`, `vmm_map_page(uint64_t virt, uint64_t phys, uint32_t flags)`, `vmm_unmap_page(uint64_t virt)`.

**`vmm.c`:**
- Internal helpers to index into each page table level: `PML4_INDEX(addr)`, `PDPT_INDEX(addr)`, `PD_INDEX(addr)`, `PT_INDEX(addr)` — each extracts the relevant 9-bit field from a 64-bit virtual address.
- `vmm_map_page()`: walks PML4→PDPT→PD→PT, allocating intermediate tables via `pmm_alloc_frame()` and zeroing them with a `memset`-like loop when they don't exist. Sets the final PTE with `phys | flags`.
- `vmm_unmap_page()`: walks the tables to find the PTE, clears it, and executes `invlpg` to invalidate the TLB.
- `vmm_init()`:
  1. Allocates a new PML4 frame via `pmm_alloc_frame()`, zeroes it.
  2. Identity-maps from 0 to `_kernel_end` (rounded up to next page) using `vmm_map_page(addr, addr, PRESENT|WRITE)`.
  3. Writes the new PML4 physical address to CR3 via inline assembly.
  4. Logs progress to serial.

#### Affected Files

- `kernel/src/vmm.h` (new)
- `kernel/src/vmm.c` (new)

#### Dependencies

- PMM (`pmm_alloc_frame()`) for allocating page table frames.
- Linker symbols `_kernel_start`, `_kernel_end` for kernel region bounds.

#### Risks

- **Incorrect page table construction causes triple fault on CR3 switch** — mitigated by ensuring identity-map covers all kernel code/data/BSS/stack before switching. The boot's 1 GiB map stays active until CR3 is loaded, so the new tables must cover everything the kernel touches.
- **Stack must be mapped** — the stack is in `.bss` (within kernel bounds), so identity-mapping to `_kernel_end` covers it.
- **Missing zero-init of new page table frames** — frames from PMM contain stale data. Must zero each allocated table frame before use.

### Task 2: Integrate VMM into kernel and build

Add `vmm.c` to the Makefile's `SRCS_C` list, include `vmm.h` in `kernel.c`, and call `vmm_init()` after `pmm_init()` and before the GDT/IDT setup (since those structures are identity-mapped and must remain accessible).

#### Affected Files

- `kernel/Makefile`
- `kernel/src/kernel.c`

#### Dependencies

- Task 1 (VMM module must exist).

#### Risks

- Init ordering matters — VMM must come after PMM but before anything that might use higher-half addresses (not applicable yet, but establishes the pattern).

### Task 3: Build and verify in QEMU

Build with `make -C kernel clean all iso` and run with `make -C kernel run-bios` or `make -C kernel run`. Verify:
1. PMM init succeeds (serial output)
2. VMM init succeeds — serial shows pages mapped, CR3 loaded
3. Kernel continues past CR3 switch without triple fault
4. GDT, IDT, PIC, PIT all still work (timer ticks fire)

#### Affected Files

(No file changes — verification only)

#### Dependencies

- Tasks 1–2 complete.

#### Risks

- Triple fault on CR3 switch if any critical region is unmapped — debug by checking serial output up to the point of failure.

## Affected Files

- `kernel/src/vmm.h`
- `kernel/src/vmm.c`
- `kernel/Makefile`
- `kernel/src/kernel.c`
