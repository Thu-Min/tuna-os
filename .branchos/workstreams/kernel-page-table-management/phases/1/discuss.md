# Phase 1 Discussion

## Goal

Replace the boot-time 2 MiB huge-page identity map with a proper 4-level page table infrastructure (`vmm.c`/`vmm.h`) that can map and unmap individual 4 KiB virtual pages to physical frames on demand, using the PMM for page table allocation.

## Requirements

1. **Build a new PML4 from C** — `vmm_init()` allocates a fresh PML4 via `pmm_alloc_frame()`, then populates it to identity-map the kernel region (at minimum the first 1 GiB, matching the boot map) using 4 KiB pages through all 4 levels (PML4 → PDPT → PD → PT).
2. **Map function** — `vmm_map_page(uint64_t virt, uint64_t phys, uint32_t flags)` creates a PTE mapping a single 4 KiB virtual page to a physical frame. Allocates intermediate page table levels (PDPT, PD, PT) on demand via `pmm_alloc_frame()` if they don't already exist.
3. **Unmap function** — `vmm_unmap_page(uint64_t virt)` clears the PTE for a virtual page and invalidates the TLB entry via `invlpg`.
4. **CR3 switch** — After building the new page tables, load CR3 with the new PML4 physical address. The kernel must continue executing without faulting.
5. **TLB invalidation** — Provide `vmm_invlpg(uint64_t virt)` to invalidate individual TLB entries after unmap operations.
6. **Serial diagnostics** — Log page table init progress and final state (e.g., pages mapped, CR3 loaded) to serial.

## Assumptions

- **Identity mapping preserved** — The kernel runs identity-mapped. Virtual addresses equal physical addresses for all kernel code/data. The VMM maintains this invariant for the kernel region.
- **4 KiB pages only** — No 2 MiB or 1 GiB huge pages in the new tables. This simplifies the implementation and matches the PMM's 4 KiB frame granularity.
- **No recursive mapping** — We won't use recursive page table mapping. Since the kernel is identity-mapped, page table frames are directly accessible at their physical addresses.
- **No user-space** — All mappings are supervisor-only (no user-accessible flag) for now. User mode comes in M3 (F-012).
- **Sufficient frames** — Identity-mapping 1 GiB with 4 KiB pages requires: 1 PML4 + 1 PDPT + 512 PDs + 512×512=262,144 PTs. That's ~262,658 frames (~1 GiB) just for page tables — far too many. We should only map what's actually needed (kernel region + a reasonable range), not the full 1 GiB.
- **Map kernel region + low memory** — Map from 0 to kernel_end (rounded up), plus enough extra for page table frames themselves. The boot's 1 GiB identity map remains active until CR3 is switched, so we can build the new tables in identity-mapped memory.

## Unknowns

- **How much to identity-map?** — Mapping the full 1 GiB at 4 KiB granularity is too expensive. Options: (a) map only what the kernel uses plus some headroom, (b) use 2 MiB huge pages for the bulk and 4 KiB for fine-grained regions. Decision: start with mapping the kernel region + enough for the PMM bitmap + page table frames, and provide `vmm_map_page()` for on-demand mapping beyond that.
- **Page table frame bookkeeping** — Frames allocated for page tables are "overhead" not tracked by the PMM as free/used application memory. The PMM already marks them as used, which is correct.

## Decisions

### Map only the kernel region initially, not full 1 GiB

**Phase:** 1
**Context:** Identity-mapping 1 GiB with 4 KiB pages requires ~262K page table frames (~1 GiB), which is impractical. Need to decide the initial mapping scope.
**Decision:** Map from physical address 0 up to `_kernel_end` rounded up to the next 2 MiB boundary, plus additional mappings as needed for page table frames themselves. Use `vmm_map_page()` for anything beyond this range. This keeps page table overhead minimal while covering all kernel code, data, BSS (including the PMM bitmap).
**Alternatives considered:**
- Map full 1 GiB with 2 MiB huge pages (simpler, but doesn't exercise the 4 KiB mapping path and doesn't meet AC-2)
- Map full 1 GiB with 4 KiB pages (~1 GiB overhead — impractical)

### Module naming: `vmm` (virtual memory manager)

**Phase:** 1
**Context:** Need a module name for the page table management code, following the project's naming convention.
**Decision:** Use `vmm.c`/`vmm.h` with `vmm_` prefix. Complements `pmm` (physical) with `vmm` (virtual).
**Alternatives considered:**
- `paging.c` — less consistent with pmm naming
- `pt.c` — too terse
