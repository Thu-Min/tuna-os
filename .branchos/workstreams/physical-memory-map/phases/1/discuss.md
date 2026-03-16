# Phase 1 Discussion

## Goal

Parse the Multiboot2 memory map passed by GRUB to detect and log all physical memory regions. This is the foundational step for M2 (Memory Management) — the page frame allocator (F-006) will consume this memory map to know which frames are available.

## Requirements

1. **Preserve Multiboot2 info pointer** — `boot.S` must save the `ebx` register (which GRUB sets to the physical address of the Multiboot2 info structure) across the 32→64-bit transition and pass it as an argument to `kernel_main`.

2. **Define Multiboot2 tag structures** — Create C structs for the Multiboot2 fixed header, generic tag header, and memory map tag (type 6) including its per-entry format. All structs must be `__attribute__((packed))`.

3. **Tag iteration** — Walk the Multiboot2 tag list starting from the info pointer, advancing by each tag's `size` (8-byte aligned), until reaching the terminating tag (type 0, size 8).

4. **Memory map parsing** — When the memory map tag (type 6) is found, iterate its entries and extract base address, length, and type for each region. Multiboot2 memory map entry types: 1 = available RAM, 2 = reserved, 3 = ACPI reclaimable, 4 = ACPI NVS, 5 = bad memory.

5. **Serial logging** — Print each memory region to serial in a readable format showing base address, length (in hex), and human-readable type string. Clearly distinguish usable RAM from reserved regions.

6. **No functional change to existing subsystems** — GDT, IDT, PIC, PIT initialization must continue working. The memory map parsing is additive.

## Assumptions

- **GRUB provides a valid Multiboot2 info structure** — We trust that GRUB populates the memory map tag correctly. No validation of the Multiboot2 magic number beyond what boot.S already checks.
- **Identity mapping covers the Multiboot2 info** — The existing 1 GiB identity map in boot.S covers the Multiboot2 info structure (GRUB places it in low memory).
- **Single phase is sufficient** — This feature has a narrow scope (parse and log), so one phase covers all three acceptance criteria.
- **Static storage only** — Consistent with the current codebase convention, memory map data will be stored in static arrays (no dynamic allocation). A reasonable upper bound for memory regions (e.g., 32 entries) is sufficient.

## Unknowns

1. **How many memory regions will QEMU/GRUB report?** — Typically 5-8 regions for a standard QEMU VM, but we should handle up to ~32 to be safe.
2. **Does the existing boot.S clobber `ebx` during the 32→64 transition?** — Need to verify which registers are used during page table setup and mode switch. If `ebx` is clobbered, it must be saved (e.g., to `edi` or a memory location) before the transition.

## Decisions

### Store memory map in a static array with fixed upper bound

**Phase:** 1
**Context:** The kernel has no dynamic allocation, and the memory map is needed before we can build an allocator.
**Decision:** Use a static array of memory map entries (e.g., `struct mmap_entry regions[32]`) with a count variable. 32 entries is more than sufficient for typical x86 QEMU setups.
**Alternatives considered:**
- Keeping only the Multiboot2 pointer and re-parsing on demand — fragile if the memory gets reused later
- Linked list — requires allocation we don't have yet

### Create a new `multiboot2.c/h` module

**Phase:** 1
**Context:** Need a clean place for Multiboot2 struct definitions and parsing logic.
**Decision:** Follow existing convention with a `multiboot2.c`/`multiboot2.h` pair in `kernel/src/`. This module will own the Multiboot2 tag definitions and memory map parsing. Export functions like `multiboot2_parse_mmap()` and `multiboot2_get_mmap()`.
**Alternatives considered:**
- Inlining everything in `kernel.c` — violates the module-per-subsystem convention
- Separate `mmap.c` for memory map — premature split; the parsing is tightly coupled to Multiboot2 tag format
