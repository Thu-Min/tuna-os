# Decisions

### Map only the kernel region initially, not full 1 GiB

**Phase:** 1
**Context:** Identity-mapping 1 GiB with 4 KiB pages requires ~262K page table frames (~1 GiB), which is impractical. Need to decide the initial mapping scope.
**Decision:** Map from physical address 0 up to `_kernel_end` rounded up to the next 2 MiB boundary, plus additional mappings as needed for page table frames themselves. Use `vmm_map_page()` for anything beyond this range. This keeps page table overhead minimal while covering all kernel code, data, BSS (including the PMM bitmap).
**Alternatives considered:**
- Map full 1 GiB with 2 MiB huge pages (simpler, but doesn't exercise the 4 KiB mapping path and doesn't meet AC-2)
- Map full 1 GiB with 4 KiB pages (~1 GiB overhead — impractical)

---

### Module naming: `vmm` (virtual memory manager)

**Phase:** 1
**Context:** Need a module name for the page table management code, following the project's naming convention.
**Decision:** Use `vmm.c`/`vmm.h` with `vmm_` prefix. Complements `pmm` (physical) with `vmm` (virtual).
**Alternatives considered:**
- `paging.c` — less consistent with pmm naming
- `pt.c` — too terse

---
