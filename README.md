# Tuna OS

Educational Unix-like kernel project for x86_64, booted with GRUB (Multiboot2) and run on QEMU.

## Project Goals

- Study-first kernel architecture, not production hardening
- C-first implementation with minimal assembly
- Monolithic kernel design
- Serial console on COM1 as first output path
- QEMU + GDB debug workflow

## Current Status

Tuna OS boots into an interactive shell running in user mode. The kernel implements:

- **Boot**: Multiboot2 header, early x86_64 transition, long mode entry
- **Serial I/O**: COM1 serial driver for all kernel output
- **GDT/TSS**: Kernel and user-mode segment descriptors, Task State Segment for ring transitions
- **IDT**: CPU exception vectors (0-31) with assembly stubs and C dispatcher
- **PIC**: 8259 PIC remapping, IRQ infrastructure, keyboard IRQ handler
- **PIT**: Programmable Interval Timer, tick counter
- **Memory**: Multiboot2 memory map parsing, physical page frame allocator (bitmap), kernel page table management (VMM), kernel heap allocator (`kmalloc`/`kfree`)
- **Tasking**: Task control blocks, context switching, round-robin scheduler
- **User mode**: TSS-based ring 3 transition, `int 0x80` system call interface (`sys_write`, `sys_read`, `sys_exit`, `sys_exec`)
- **ELF loader**: Loads ELF64 binaries into user-space page tables
- **RAM filesystem**: In-memory filesystem with embedded user binaries
- **Shell**: Interactive shell with `help`, `echo`, `hello`, and `exit` commands

## Repository Layout

```text
tuna_os/
├── Makefile                  # top-level wrapper
├── kernel/
│   ├── Makefile              # build/run/debug for kernel
│   ├── linker.ld             # kernel linker script
│   ├── grub.cfg              # GRUB menu entry
│   └── src/
│       ├── boot.S            # Multiboot2 + early boot + long mode jump
│       ├── kernel.c          # kernel entry point
│       ├── serial.c/h        # COM1 serial driver
│       ├── gdt.c/h           # GDT setup
│       ├── gdt_flush.S       # LGDT + segment reload helper
│       ├── tss.c/h           # Task State Segment
│       ├── idt.c/h           # IDT setup + exception dispatcher
│       ├── interrupts.S      # exception/IRQ entry stubs
│       ├── pic.c/h           # 8259 PIC driver
│       ├── pit.c/h           # PIT timer driver
│       ├── io.h              # port I/O helpers
│       ├── multiboot2.c/h    # Multiboot2 memory map parser
│       ├── pmm.c/h           # physical memory manager
│       ├── vmm.c/h           # virtual memory manager
│       ├── kheap.c/h         # kernel heap allocator
│       ├── task.c/h          # task control blocks + scheduler
│       ├── switch.S          # context switch trampoline
│       ├── syscall.c/h       # system call dispatcher
│       ├── usermode.c/h      # user-mode transition
│       ├── elf.c/h           # ELF64 loader
│       ├── ramfs.c/h         # RAM filesystem
│       └── user_programs.S   # embedded user binary blobs
└── user/
    ├── Makefile              # user-space program build
    ├── link.ld               # user-space linker script
    ├── hello.c               # hello world user program
    └── shell.c               # interactive shell
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

## Run

Default (EFI boot):

```bash
make run
```

BIOS boot (if your GRUB has `pc` modules):

```bash
make run-bios
```

## Debug (QEMU + GDB)

Start QEMU paused with GDB stub:

```bash
make debug        # EFI
make debug-bios   # BIOS
```

In another terminal:

```bash
x86_64-elf-gdb kernel/build/kernel.elf
```

Then in GDB:

```gdb
target remote :1234
break kernel_main
continue
```
