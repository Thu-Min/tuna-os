# Tuna OS

Educational Unix-like kernel project for x86_64, booted with GRUB (Multiboot2) and run on QEMU.

Phase 1 (boot + serial output).

## Project Goals

- Study-first kernel architecture, not production hardening
- C-first implementation with minimal assembly
- Monolithic kernel design
- Serial console on COM1 as first output path
- QEMU + GDB debug workflow

## Current Status (Phase 1)

Implemented:

- Multiboot2 header and boot entry
- Early x86_64 transition path
- Kernel entry (`kernel_main`)
- COM1 serial driver
- GRUB ISO image generation
- QEMU run/debug targets

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

```bash
make run
```

Expected output on terminal:

```text
kernel: hello from tuna os!
```

## Debug (QEMU + GDB)

Start QEMU paused with GDB stub:

```bash
make debug
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
