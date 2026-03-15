---
generated: 2026-03-15T06:10:00Z
commit: 6bc7c277cbf0bfc239f249c8c50b112f8907e9e5
generator: branchos/map-codebase
---

# Concerns

## Flat Source Layout

All source files live directly in `kernel/src/` with no subdirectories. Currently manageable at 15 files, but as subsystems grow (memory management, scheduling, filesystem, syscalls), the flat layout will make navigation harder.

## No Dynamic Memory Allocation

All data structures are statically sized. The IRQ handler table is fixed at 16 entries, the IDT at 256 entries, and the GDT at 3 entries. Future subsystems (task management, filesystem) will likely need heap allocation before they can be implemented.

## Serial-Only Diagnostics

Serial output is the sole diagnostic and logging mechanism. There is no VGA text mode, framebuffer console, or structured logging. All subsystems depend on `serial.h` for output, creating a fan-in dependency.

## Bootstrap GDT in boot.S

The bootstrap GDT defined in `boot.S` uses a 32-bit `.long` for the pointer address in `gdt64_ptr`, which works because the kernel is loaded at 1 MiB (within 32-bit address space). The bootstrap GDT is also conceptually duplicated by the proper GDT in `gdt.c` with similar but not identical encodings.

## Hardcoded Stack Size

The kernel stack is 16 KiB, defined as a BSS allocation in `boot.S`. There is no guard page or stack overflow detection. Nested interrupt handlers or deep call chains could silently corrupt memory below the stack.

## No IST (Interrupt Stack Table) Usage

All IDT entries have `ist = 0`, meaning exceptions reuse the current stack. A double fault or stack overflow during exception handling would cause undefined behavior rather than a clean crash.

## PIT Tick Logging in IRQ Context

The PIT handler logs to serial every `configured_frequency` ticks (once per second). Serial I/O involves busy-waiting on the UART transmit buffer. Doing this inside an interrupt handler blocks other interrupts for the duration of serial output. At current volumes this is fine, but it would become a problem with higher tick rates or more serial output.

## Identity Map Covers 1 GiB

The page tables in `boot.S` identity-map the first 1 GiB using 2 MiB pages. This is sufficient for early boot but will need to be replaced with a proper virtual memory manager before higher-half kernel or user-space support.
