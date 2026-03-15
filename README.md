# Tuna OS

Educational Unix-like kernel project for x86_64, booted with GRUB (Multiboot2) and run on QEMU.

Current milestone: Phase 2 (GDT + IDT + exceptions over serial).

## Project Goals

- Study-first kernel architecture, not production hardening
- C-first implementation with minimal assembly
- Monolithic kernel design
- Serial console on COM1 as first output path
- QEMU + GDB debug workflow

## Current Status (Phase 2)

Implemented:

- Multiboot2 header and boot entry
- Early x86_64 transition path
- Kernel entry (`kernel_main`)
- COM1 serial driver
- Structured kernel GDT setup and reload
- IDT setup for CPU exception vectors (0-31)
- Exception stubs in assembly + C dispatcher
- Breakpoint (`int3`) self-test path
- GRUB ISO image generation
- QEMU BIOS and EFI run/debug targets

## Repository Layout

```text
tuna_os/
├── Makefile                  # top-level wrapper
└── kernel/
    ├── Makefile              # build/run/debug for kernel
    ├── linker.ld             # kernel linker script
    ├── grub.cfg              # GRUB menu entry
    └── src/
        ├── boot.S            # Multiboot2 + early boot + long mode jump
        ├── gdt.c             # kernel GDT table setup
        ├── gdt.h
        ├── gdt_flush.S       # LGDT + segment reload helper
        ├── idt.c             # IDT setup + exception dispatcher
        ├── idt.h
        ├── interrupts.S      # exception entry stubs + common path
        ├── kernel.c          # kernel entry point
        ├── serial.c          # COM1 serial driver
        └── serial.h
```

## Prerequisites

Cross toolchain and emulator tools:

- `x86_64-elf-gcc`
- `x86_64-elf-binutils`
- `grub-mkrescue` (or `x86_64-elf-grub-mkrescue`)
- `xorriso`
- `mtools`
- `qemu-system-x86_64`

macOS (Homebrew) example:

```bash
brew install x86_64-elf-binutils x86_64-elf-gcc x86_64-elf-grub xorriso mtools qemu
```

## Build

From repository root:

```bash
make
```

Or inside kernel directory:

```bash
make -C kernel all
```

## Run (Serial Console)

BIOS path:

```bash
make run
```

EFI path (recommended fallback if BIOS CD boot fails on your host):

```bash
make run-efi
```

Expected output on terminal:

```text
kernel: hello from tuna os!
phase2: initializing gdt...
phase2: gdt ready
phase2: initializing idt...
phase2: idt ready
phase2: triggering int3 self-test
[exception] vector=3 (Breakpoint) error=0x0 rip=...
[exception] breakpoint handled, resuming.
phase2: idle loop
```

## Debug (QEMU + GDB)

Start QEMU paused with GDB stub (BIOS):

```bash
make debug
```

Or paused with EFI firmware:

```bash
make debug-efi
```

In another terminal:

```bash
x86_64-elf-gdb /Volumes/BulkStorage/geek/tuna_os/kernel/build/kernel.elf
```

Then in GDB:

```gdb
target remote :1234
break kernel_main
continue
```

## Phase 2 Notes

- Exception handling currently treats most CPU faults as fatal and halts.
- Breakpoint (`int3`) is handled and returns for bring-up verification.
- `PHASE2_BREAKPOINT_SELF_TEST` in `/Volumes/BulkStorage/geek/tuna_os/kernel/src/kernel.c` controls automatic self-test trigger.

## Next Milestone (Phase 3)

- Programmable timer interrupt (PIT/APIC)
- IRQ remapping and interrupt enable path
- Tick counter and minimal scheduler groundwork
