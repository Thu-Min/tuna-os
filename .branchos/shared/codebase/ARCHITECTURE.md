---
generated: 2026-03-15T00:00:00Z
commit: 39072ceafaf7eddc9324babf2194289c4e868893
generator: branchos/map-codebase
---

# Architecture

## Directory Tree (Top 2 Levels)

```
tuna_os/
├── Makefile
├── README.md
└── kernel/
    ├── Makefile
    ├── linker.ld
    ├── grub.cfg
    ├── build/
    ├── iso/
    └── src/
```

## Architecture Pattern

Bare-metal monolithic kernel for x86_64, cross-compiled with `x86_64-elf-gcc` and booted via GRUB Multiboot2. The project follows a flat, single-binary kernel model — all subsystems compile and link into one ELF (`kernel.elf`), which GRUB loads directly.

## Entry Points

1. **`kernel/src/boot.S` (`_start`)** — Multiboot2 entry point. Sets up page tables for a 1 GiB identity map, enables PAE and long mode, loads a bootstrap GDT, and far-jumps to 64-bit `long_mode_entry`, which calls `kernel_main`.
2. **`kernel/src/kernel.c` (`kernel_main`)** — C entry point. Initializes serial, GDT, IDT, runs a breakpoint self-test, then enters an idle `hlt` loop.

## Data Flow

```
GRUB (Multiboot2)
  → _start (boot.S): 32-bit → page tables → long mode
    → kernel_main (kernel.c)
      → serial_init(): configure COM1
      → gdt_init(): build GDT table, call gdt_flush (assembly)
      → idt_init(): build IDT, install ISR stubs
      → int3 self-test → isr_common_stub → isr_dispatch → serial output
      → hlt loop
```

## Subsystem Boundaries

- **Boot** (`boot.S`): Hardware transition from 32-bit protected mode to 64-bit long mode. Owns page tables and bootstrap GDT. Hands off to C code.
- **Serial** (`serial.c/h`): COM1 I/O driver. Used by all other subsystems for output. No dependencies beyond port I/O.
- **GDT** (`gdt.c/h`, `gdt_flush.S`): Descriptor table management. Replaces the bootstrap GDT from boot with a proper kernel GDT.
- **IDT/Interrupts** (`idt.c/h`, `interrupts.S`): Exception vector table and dispatch. Assembly stubs save/restore registers and call into C dispatcher.
- **Linker** (`linker.ld`): Controls memory layout — kernel loaded at 1 MiB, sections aligned to 4K pages.
