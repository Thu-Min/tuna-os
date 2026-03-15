---
generated: 2026-03-15T00:00:00Z
commit: 39072ceafaf7eddc9324babf2194289c4e868893
generator: branchos/map-codebase
---

# Code Conventions

## Naming

- **Files**: lowercase with underscores (`gdt_flush.S`, `serial.c`)
- **Functions**: `snake_case`, prefixed by module name (`serial_write`, `gdt_init`, `idt_set_gate`)
- **Types**: `struct snake_case` with `__attribute__((packed))` for hardware-facing structs
- **Constants/Macros**: `UPPER_SNAKE_CASE` (`IDT_ENTRIES`, `KERNEL_CODE_SELECTOR`)
- **Assembly labels**: prefixed with `.L` for local labels (`.Lreload_cs`), bare names for globals

## File Organization

- Headers use `#pragma once` include guards
- Each subsystem is a `.c`/`.h` pair, with an optional `.S` for assembly helpers
- Flat source layout under `kernel/src/` — no subdirectories
- Assembly files use AT&T syntax with `.section`, `.code32`/`.code64` directives

## State Management

- Module-level static globals for hardware tables (`static struct gdt_descriptor gdt[3]`, `static struct idt_entry idt[IDT_ENTRIES]`)
- No dynamic allocation — all structures are statically sized
- Hardware registers accessed via inline `outb`/`inb` wrappers defined locally in the module that uses them

## Error Handling

- Fatal exceptions halt the CPU with `cli; hlt` in a loop
- Non-fatal exceptions (breakpoint) log and return
- Serial output is the sole diagnostic channel

## Import/Export

- C headers expose only public API (`void gdt_init(void)`)
- Assembly symbols declared `extern` in C files that need them (`extern void gdt_flush(...)`)
- Assembly exports use `.global` directive
- `kernel.c` includes all subsystem headers and calls init functions in sequence

## Build

- C standard: C11 (`-std=c11`)
- Strict warnings: `-Wall -Wextra -Werror`
- Freestanding: `-ffreestanding -fno-pic -fno-pie -mno-red-zone`
