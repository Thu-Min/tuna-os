---
generated: 2026-03-17T03:45:00Z
commit: 011dba525df1117917cb24fce38283888cc4f393
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

1. **`kernel/src/boot.S` (`_start`)** — Multiboot2 entry point. Sets up page tables for a 1 GiB identity map using 2 MiB huge pages, enables PAE and long mode, loads a bootstrap GDT, and far-jumps to 64-bit `long_mode_entry`, which passes the Multiboot2 info pointer to `kernel_main`.
2. **`kernel/src/kernel.c` (`kernel_main`)** — C entry point. Initializes serial, memory map, PMM, VMM, kheap, GDT, IDT, PIC, and PIT, then enters an idle `hlt` loop with interrupts enabled.

## Data Flow

```
GRUB (Multiboot2)
  → _start (boot.S): 32-bit → page tables → long mode
    → kernel_main (kernel.c)
      → serial_init(): configure COM1
      → multiboot2_parse_mmap(): parse memory map from Multiboot2 info
      → pmm_init(): bitmap allocator from memory map, mark kernel+low mem used
      → vmm_init(): build 4-level page tables, identity-map 4 MiB, switch CR3
      → kheap_init(): map heap at 2 MiB, init free-list allocator
      → gdt_init(): build GDT table, call gdt_flush (assembly)
      → idt_init(): build IDT, install ISR stubs (vectors 0-47)
      → int3 self-test → isr_common_stub → isr_dispatch → serial output
      → pic_init(): remap PIC, mask all IRQs
      → pit_init(100): program PIT at 100 Hz, register IRQ0 handler, unmask IRQ0
      → sti (enable interrupts)
      → hlt loop (timer ticks fire IRQ0 → pit_irq_handler → tick counter)
```

## Subsystem Boundaries

- **Boot** (`boot.S`): Hardware transition from 32-bit protected mode to 64-bit long mode. Owns bootstrap page tables (2 MiB huge pages) and bootstrap GDT. Hands off to C code.
- **I/O** (`io.h`): Shared inline `outb`/`inb` port I/O wrappers used by all hardware-facing modules.
- **Serial** (`serial.c/h`): COM1 UART driver. Used by all other subsystems for diagnostic output. Depends only on `io.h`.
- **Multiboot2** (`multiboot2.c/h`): Parses Multiboot2 memory map tag from GRUB. Exports parsed region array for PMM consumption.
- **PMM** (`pmm.c/h`): Physical page frame allocator. Bitmap-based, tracks 4 KiB frames up to 128 MiB. Initialized from memory map, protects kernel and low-memory regions.
- **VMM** (`vmm.c/h`): Virtual memory manager. Builds 4-level page tables (PML4→PDPT→PD→PT) with 4 KiB pages. Provides `vmm_map_page`/`vmm_unmap_page` for on-demand mapping. Replaces boot page tables and switches CR3.
- **Kernel Heap** (`kheap.c/h`): Dynamic memory allocator (`kmalloc`/`kfree`). Free-list first-fit with block splitting and coalescing. Heap at 0x200000, expands on demand via PMM+VMM.
- **GDT** (`gdt.c/h`, `gdt_flush.S`): Descriptor table management. Replaces the bootstrap GDT from boot with a proper kernel GDT.
- **IDT/Interrupts** (`idt.c/h`, `interrupts.S`): Exception and IRQ vector table, ISR stub dispatch, and IRQ handler registration. Assembly stubs save/restore registers and call into C dispatcher.
- **PIC** (`pic.c/h`): 8259A PIC driver. Remaps IRQs to vectors 32-47, manages per-IRQ masking, and sends EOI.
- **PIT** (`pit.c/h`): Programmable Interval Timer driver. Configures channel 0 for periodic interrupts, maintains a monotonic tick counter.
- **Linker** (`linker.ld`): Controls memory layout — kernel loaded at 1 MiB, sections aligned to 4K pages. Exports `_kernel_start` and `_kernel_end` symbols.
