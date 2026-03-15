---
generated: 2026-03-15T06:10:00Z
commit: 6bc7c277cbf0bfc239f249c8c50b112f8907e9e5
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
- Purpose: Multiboot2 header, 32-bit to 64-bit transition, page table setup, bootstrap GDT
- Exports: `_start` (global entry point)
- Dependencies: None (first code to execute)

### Serial Driver (`serial.c`, `serial.h`)
- Purpose: COM1 (0x3F8) UART driver — init, character/string output, hex and decimal formatting
- Exports: `serial_init`, `serial_write`, `serial_write_char`, `serial_write_hex_u64`, `serial_write_dec_u64`
- Dependencies: `io.h`

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
- Purpose: `kernel_main` — orchestrates initialization sequence and enters idle loop
- Dependencies: `gdt.h`, `idt.h`, `pic.h`, `pit.h`, `serial.h`

## `kernel/` — Build Infrastructure

- `Makefile`: Cross-compilation, ISO generation, QEMU run/debug targets
- `linker.ld`: Kernel loaded at 1 MiB, sections: `.text`, `.rodata`, `.data`, `.bss`
- `grub.cfg`: GRUB serial console config, Multiboot2 boot entry
