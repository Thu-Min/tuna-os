---
generated: 2026-03-15T00:00:00Z
commit: 39072ceafaf7eddc9324babf2194289c4e868893
generator: branchos/map-codebase
---

# Modules

## `kernel/src/` — Kernel Source

All kernel logic lives in this flat directory. No subdirectories yet.

### Boot (`boot.S`)
- Purpose: Multiboot2 header, 32-bit to 64-bit transition, page table setup, bootstrap GDT
- Exports: `_start` (global entry point)
- Dependencies: None (first code to execute)

### Serial Driver (`serial.c`, `serial.h`)
- Purpose: COM1 (0x3F8) UART driver — init, character/string output, hex and decimal formatting
- Exports: `serial_init`, `serial_write`, `serial_write_char`, `serial_write_hex_u64`, `serial_write_dec_u64`
- Dependencies: None (uses inline port I/O only)

### GDT (`gdt.c`, `gdt.h`, `gdt_flush.S`)
- Purpose: Build and load a 64-bit kernel GDT with null, code, and data segments
- Exports: `gdt_init`
- Internal: `gdt_flush` (assembly helper called from `gdt_init`)
- Dependencies: None

### IDT / Interrupts (`idt.c`, `idt.h`, `interrupts.S`)
- Purpose: CPU exception handling — IDT setup, ISR stub table (vectors 0–31), C dispatch with serial logging
- Exports: `idt_init`, `isr_dispatch`
- Internal: `isr_stub_table`, `isr_common_stub` (assembly)
- Dependencies: `serial.h` (for exception logging output)

### Kernel Entry (`kernel.c`)
- Purpose: `kernel_main` — orchestrates initialization sequence and enters idle loop
- Dependencies: `serial.h`, `gdt.h`, `idt.h`

## `kernel/` — Build Infrastructure

- `Makefile`: Cross-compilation, ISO generation, QEMU run/debug targets
- `linker.ld`: Kernel loaded at 1 MiB, sections: `.text`, `.rodata`, `.data`, `.bss`
- `grub.cfg`: GRUB serial console config, Multiboot2 boot entry
