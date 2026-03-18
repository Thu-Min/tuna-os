---
generated: 2026-03-17T03:45:00Z
commit: 011dba525df1117917cb24fce38283888cc4f393
generator: branchos/map-codebase
---

# Concerns

## VMM Initial Mapping Scope

The VMM identity-maps a fixed 4 MiB range at init. This was pragmatically chosen to cover the kernel image (~1 MiB), heap start (2 MiB), and nearby PMM-allocated frames. As more subsystems allocate frames at higher physical addresses, the VMM may need to map those addresses before writing to them. The boot page tables (2 MiB huge pages covering 1 GiB) remain in physical memory but are no longer referenced after CR3 switches.

## PMM Frame Allocation Beyond Mapped Range

`pmm_alloc_frame()` can return physical addresses well above the 4 MiB VMM-mapped range. Any code that writes to these frames (e.g., `alloc_table()` in VMM, `kheap_expand()`) relies on those addresses being identity-mapped. Currently this works because the VMM maps heap expansion pages before writing to them, and VMM's own `alloc_table` gets frames in the low range. As the system grows, frame allocation order could shift.

## Heap Allocator Fragmentation

The free-list first-fit allocator coalesces adjacent free blocks, but does not return pages to the PMM when freed. Long-running allocation/deallocation patterns could lead to fragmentation where total free memory is sufficient but no single contiguous block is large enough. The linear block walk for both allocation and coalescing is O(n) in the number of blocks.

## No Memory Protection Between Subsystems

All kernel code runs in a single flat address space with read-write access everywhere. There is no guard page between the stack and heap, no read-only protection on `.rodata` or `.text` pages, and no separation between page table memory and general-purpose memory. A stray write could corrupt page tables or interrupt descriptor tables silently.

## Static Limits

Several subsystems use fixed-size static arrays:
- `MMAP_MAX_REGIONS` (32) for memory map entries
- `PMM_MAX_FRAMES` (32768) caps addressable physical memory at 128 MiB
- `IDT_ENTRIES` (256) and `irq_handlers[16]` are appropriately sized for x86
- `KHEAP_MAX_SIZE` (16 MiB) caps heap growth

These are reasonable for current use but will need adjustment for larger memory configurations.

## No `memset`/`memcpy` Utility

Zero-filling of page table frames and heap blocks is done with inline `for` loops. As more subsystems need memory operations, a shared `memset`/`memcpy` implementation would reduce code duplication.

## GDT Lacks TSS Entry

The current GDT has only null, code, and data segments. A Task State Segment (TSS) entry will be required for M3 (user mode transition, F-012) to handle privilege level changes and interrupt stack switching.

## Flat Source Layout

All source files live directly in `kernel/src/` with no subdirectories. Currently manageable at 23 files, but as subsystems grow (scheduling, filesystem, syscalls), the flat layout will make navigation harder.

## Serial-Only Diagnostics

Serial output is the sole diagnostic and logging mechanism. There is no VGA text mode, framebuffer console, or structured logging. All subsystems depend on `serial.h` for output.

## Hardcoded Stack Size

The kernel stack is 16 KiB, defined as a BSS allocation in `boot.S`. There is no guard page or stack overflow detection.

## No IST (Interrupt Stack Table) Usage

All IDT entries have `ist = 0`, meaning exceptions reuse the current stack. A double fault during exception handling would cause undefined behavior rather than a clean crash.

## PIT Tick Logging in IRQ Context

The PIT handler logs to serial every `configured_frequency` ticks (once per second). Serial I/O involves busy-waiting on the UART transmit buffer inside an interrupt handler. At current volumes this is fine, but it would become a problem with higher tick rates or more serial output.
