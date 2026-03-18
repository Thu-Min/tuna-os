---
generated: 2026-03-17T03:45:00Z
commit: 011dba525df1117917cb24fce38283888cc4f393
generator: branchos/map-codebase
---

# Modules

## `kernel/src/` — Kernel Source

All kernel logic lives in this flat directory. No subdirectories.

### I/O (`io.h`)
- Purpose: Shared inline port I/O primitives (`outb`, `inb`)
- Exports: `outb`, `inb`
- Dependencies: None (header-only, uses inline assembly)
- Used by: `serial.c`, `pic.c`, `pit.c`

### Boot (`boot.S`)
- Purpose: Multiboot2 header, 32-bit to 64-bit transition, bootstrap page tables (2 MiB huge pages for 1 GiB identity map), bootstrap GDT
- Exports: `_start` (global entry point)
- Dependencies: None (first code to execute)

### Serial Driver (`serial.c`, `serial.h`)
- Purpose: COM1 (0x3F8) UART driver — init, character/string output, hex and decimal formatting
- Exports: `serial_init`, `serial_write`, `serial_write_char`, `serial_write_hex_u64`, `serial_write_dec_u64`
- Dependencies: `io.h`

### Multiboot2 (`multiboot2.c`, `multiboot2.h`)
- Purpose: Parse Multiboot2 memory map tag from GRUB, store detected memory regions
- Exports: `multiboot2_parse_mmap`, `multiboot2_get_mmap`, `multiboot2_print_mmap`
- Types: `struct mmap_region` (base, length, type), `struct multiboot2_mmap_entry`
- Dependencies: `serial.h`

### PMM (`pmm.c`, `pmm.h`)
- Purpose: Bitmap-based physical page frame allocator for 4 KiB frames
- Exports: `pmm_init`, `pmm_alloc_frame`, `pmm_free_frame`, `pmm_get_free_count`, `pmm_get_total_count`
- Constants: `PMM_PAGE_SIZE` (4096), `PMM_MAX_FRAMES` (32768 = 128 MiB)
- Internal: `bitmap[4096]` (static), kernel bounds via `_kernel_start`/`_kernel_end` linker symbols
- Dependencies: `multiboot2.h`, `serial.h`

### VMM (`vmm.c`, `vmm.h`)
- Purpose: 4-level page table management — build, map, unmap 4 KiB pages, CR3 switching
- Exports: `vmm_init`, `vmm_map_page`, `vmm_unmap_page`
- Constants: `VMM_PAGE_SIZE` (4096), `VMM_FLAG_PRESENT`, `VMM_FLAG_WRITE`, `VMM_FLAG_USER`
- Internal: `pml4` (static pointer to root page table), `alloc_table` (allocates and zeroes page table frames)
- Dependencies: `pmm.h`, `serial.h`

### Kernel Heap (`kheap.c`, `kheap.h`)
- Purpose: Dynamic memory allocator — free-list first-fit `kmalloc`/`kfree` with block splitting and coalescing
- Exports: `kheap_init`, `kmalloc`, `kfree`
- Constants: `KHEAP_START` (0x200000), `KHEAP_INITIAL_SIZE` (64 KiB), `KHEAP_MAX_SIZE` (16 MiB)
- Internal: `block_header` struct (size, free flag), linear block walk for allocation and coalescing
- Dependencies: `pmm.h`, `vmm.h`, `serial.h`

### GDT (`gdt.c`, `gdt.h`, `gdt_flush.S`)
- Purpose: Build and load a 64-bit kernel GDT with null, code, and data segments
- Exports: `gdt_init`
- Internal: `gdt_flush` (assembly helper called from `gdt_init`)
- Dependencies: None

### IDT / Interrupts (`idt.c`, `idt.h`, `interrupts.S`)
- Purpose: CPU exception handling and IRQ dispatch — IDT setup, ISR stub table (vectors 0–47), C dispatch with serial logging, and per-IRQ handler registration
- Exports: `idt_init`, `isr_dispatch`, `irq_register_handler`
- Types: `struct interrupt_frame`, `irq_handler_t` (function pointer typedef)
- Internal: `isr_stub_table`, `isr_common_stub` (assembly), `irq_handlers[16]` (static handler table)
- Dependencies: `serial.h`, `pic.h`

### PIC (`pic.c`, `pic.h`)
- Purpose: Dual 8259A PIC driver — initialization with IRQ remapping to vectors 32-47, per-IRQ mask/unmask, EOI signaling
- Exports: `pic_init`, `pic_eoi`, `irq_mask`, `irq_unmask`
- Dependencies: `io.h`

### PIT (`pit.c`, `pit.h`)
- Purpose: Programmable Interval Timer driver — configures channel 0 for periodic interrupts, maintains monotonic tick counter, periodic serial logging
- Exports: `pit_init`, `pit_get_ticks`
- Internal: `pit_irq_handler` (registered via `irq_register_handler`), `tick_count` (volatile)
- Dependencies: `idt.h`, `io.h`, `pic.h`, `serial.h`

### Kernel Entry (`kernel.c`)
- Purpose: `kernel_main` — orchestrates initialization sequence (serial → mmap → PMM → VMM → kheap → GDT → IDT → PIC → PIT) and enters idle loop
- Dependencies: `gdt.h`, `idt.h`, `kheap.h`, `multiboot2.h`, `pic.h`, `pmm.h`, `pit.h`, `serial.h`, `vmm.h`

## `kernel/` — Build Infrastructure

- `Makefile`: Cross-compilation, ISO generation, QEMU run/debug targets (BIOS and EFI)
- `linker.ld`: Kernel loaded at 1 MiB, sections: `.text`, `.rodata`, `.data`, `.bss`. Exports `_kernel_start` and `_kernel_end` symbols.
- `grub.cfg`: GRUB serial console config, Multiboot2 boot entry
