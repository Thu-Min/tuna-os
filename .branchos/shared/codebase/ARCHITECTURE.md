---
generated: 2026-03-15T06:10:00Z
commit: 6bc7c277cbf0bfc239f249c8c50b112f8907e9e5
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

Bare-metal monolithic kernel for x86_64, cross-compiled with `x86_64-elf-gcc` and booted via GRUB Multiboot2. All subsystems compile and link into a single ELF (`kernel.elf`). The kernel follows a flat, single-binary model with no dynamic loading.

## Entry Points

1. **`kernel/src/boot.S` (`_start`)** — Multiboot2 entry point. Sets up page tables for a 1 GiB identity map, enables PAE and long mode, loads a bootstrap GDT, and far-jumps to 64-bit `long_mode_entry`, which calls `kernel_main`.
2. **`kernel/src/kernel.c` (`kernel_main`)** — C entry point. Initializes serial, GDT, IDT, PIC, and PIT, then enters an idle `hlt` loop with interrupts enabled.

## Data Flow

```
GRUB (Multiboot2)
  → _start (boot.S): 32-bit → page tables → long mode
    → kernel_main (kernel.c)
      → serial_init(): configure COM1
      → gdt_init(): build GDT table, call gdt_flush (assembly)
      → idt_init(): build IDT, install ISR stubs (vectors 0-47)
      → int3 self-test → isr_common_stub → isr_dispatch → serial output
      → pic_init(): remap PIC, mask all IRQs
      → pit_init(100): program PIT at 100 Hz, register IRQ0 handler, unmask IRQ0
      → sti (enable interrupts)
      → hlt loop (timer ticks fire IRQ0 → pit_irq_handler → tick counter)
```

## Subsystem Boundaries

- **Boot** (`boot.S`): Hardware transition from 32-bit protected mode to 64-bit long mode. Owns page tables and bootstrap GDT. Hands off to C code.
- **I/O** (`io.h`): Shared inline `outb`/`inb` port I/O wrappers used by all hardware-facing modules.
- **Serial** (`serial.c/h`): COM1 UART driver. Used by all other subsystems for diagnostic output. Depends only on `io.h`.
- **GDT** (`gdt.c/h`, `gdt_flush.S`): Descriptor table management. Replaces the bootstrap GDT from boot with a proper kernel GDT.
- **IDT/Interrupts** (`idt.c/h`, `interrupts.S`): Exception and IRQ vector table, ISR stub dispatch, and IRQ handler registration. Assembly stubs save/restore registers and call into C dispatcher. Provides `irq_register_handler()` for per-IRQ callbacks.
- **PIC** (`pic.c/h`): 8259A PIC driver. Remaps IRQs to vectors 32-47, manages per-IRQ masking, and sends EOI.
- **PIT** (`pit.c/h`): Programmable Interval Timer driver. Configures channel 0 for periodic interrupts, maintains a monotonic tick counter, and logs ticks to serial.
- **Linker** (`linker.ld`): Controls memory layout — kernel loaded at 1 MiB, sections aligned to 4K pages.
