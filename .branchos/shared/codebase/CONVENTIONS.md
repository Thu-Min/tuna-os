---
generated: 2026-03-15T06:10:00Z
commit: 6bc7c277cbf0bfc239f249c8c50b112f8907e9e5
generator: branchos/map-codebase
---

# Code Conventions

## Naming

- **Files**: lowercase with underscores (`gdt_flush.S`, `serial.c`)
- **Functions**: `snake_case`, prefixed by module name (`serial_write`, `gdt_init`, `idt_set_gate`, `pit_get_ticks`)
- **Types**: `struct snake_case` with `__attribute__((packed))` for hardware-facing structs; `typedef` for function pointer types (`irq_handler_t`)
- **Constants/Macros**: `UPPER_SNAKE_CASE` (`IDT_ENTRIES`, `PIT_BASE_FREQ`, `PIC1_CMD`)
- **Assembly labels**: prefixed with `.L` for local labels (`.Lreload_cs`), bare names for globals

## File Organization

- Headers use `#pragma once` include guards
- Each subsystem is a `.c`/`.h` pair, with an optional `.S` for assembly helpers
- Flat source layout under `kernel/src/` — no subdirectories
- Assembly files use AT&T syntax with `.section`, `.code32`/`.code64` directives
- Shared utilities like port I/O live in header-only files (`io.h`)

## State Management

- Module-level static globals for hardware tables (`static struct gdt_descriptor gdt[3]`, `static struct idt_entry idt[IDT_ENTRIES]`)
- `volatile` qualifier for state modified by interrupt handlers (`static volatile uint64_t tick_count`)
- No dynamic allocation — all structures are statically sized
- Hardware registers accessed via shared `outb`/`inb` from `io.h`

## Error Handling

- Fatal exceptions halt the CPU with `cli; hlt` in a loop
- Non-fatal exceptions (breakpoint) log and return
- Serial output is the sole diagnostic channel

## Import/Export

- C headers expose only public API (`void gdt_init(void)`)
- Assembly symbols declared `extern` in C files that need them (`extern void gdt_flush(...)`)
- Assembly exports use `.global` directive
- `kernel.c` includes all subsystem headers and calls init functions in sequence
- IRQ handlers registered via function pointer (`irq_register_handler(irq, handler_fn)`)

## Build

- C standard: C11 (`-std=c11`)
- Strict warnings: `-Wall -Wextra -Werror`
- Freestanding: `-ffreestanding -fno-pic -fno-pie -mno-red-zone`
