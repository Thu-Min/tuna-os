# Phase 1 Plan

## Objective

Deliver a working PIT timer with IRQ0 handler, monotonic tick counter, and serial tick logging — satisfying all four acceptance criteria for F-004.

## Tasks

### Task 1: Add IRQ handler registration to the interrupt dispatcher

Add a function-pointer table (`irq_handlers[16]`) and registration function to `idt.c`, and update `isr_dispatch` to call registered handlers instead of generic serial logging for IRQs.

**Changes:**
- Declare `typedef void (*irq_handler_t)(struct interrupt_frame *)` and `irq_register_handler()` in `idt.h`.
- In `idt.c`, add a `static irq_handler_t irq_handlers[16]` array (initialized to NULL).
- Add `irq_register_handler(uint8_t irq, irq_handler_t handler)` that stores the handler.
- Modify `isr_dispatch`: if `irq_handlers[irq]` is non-NULL, call it; otherwise fall through to the existing generic serial log. EOI remains after the handler call.

#### Affected Files

- `kernel/src/idt.c`
- `kernel/src/idt.h`

#### Dependencies

None — modifies existing code only.

#### Risks

- Changing `isr_dispatch` could break existing IRQ behavior if done incorrectly. Mitigated by keeping the generic log as the fallback for unregistered IRQs.

### Task 2: Create PIT driver module (pit.c / pit.h)

Create the PIT timer driver with initialization, IRQ0 handler, and tick query API.

**Changes in `pit.h`:**
- `void pit_init(uint32_t frequency)` — configure PIT and register IRQ handler.
- `uint64_t pit_get_ticks(void)` — return current tick count.

**Changes in `pit.c`:**
- `#include "pit.h"`, `#include "idt.h"`, `#include "pic.h"`, `#include "serial.h"`.
- Constants: `PIT_CHANNEL0_DATA 0x40`, `PIT_COMMAND 0x43`, `PIT_BASE_FREQ 1193182`.
- `static volatile uint64_t tick_count = 0` — the global tick counter.
- `static void pit_irq_handler(struct interrupt_frame *frame)`:
  - Increment `tick_count`.
  - Every 100 ticks, log tick count to serial (e.g., `"[pit] tick=<N>\n"`).
  - `frame` parameter unused but required by handler signature.
- `pit_init(uint32_t frequency)`:
  - Compute divisor = `PIT_BASE_FREQ / frequency`.
  - Send command byte `0x34` (channel 0, lobyte/hibyte, mode 2) to port `0x43`.
  - Send divisor low byte then high byte to port `0x40`.
  - Register handler: `irq_register_handler(0, pit_irq_handler)`.
  - Unmask IRQ 0: `irq_unmask(0)`.
  - Log: `"[pit] initialized at <frequency> Hz\n"`.
- `pit_get_ticks()`: return `tick_count`.
- Port I/O: define local `static inline void outb(uint16_t port, uint8_t val)` following existing convention (each module defines its own I/O helpers).

#### Affected Files

- `kernel/src/pit.c` (new)
- `kernel/src/pit.h` (new)

#### Dependencies

- Task 1 (needs `irq_register_handler`).

#### Risks

- Wrong PIT command byte or divisor byte order would produce no interrupts or incorrect frequency. Standard values are well-documented; use `0x34` for mode 2 rate generator.

### Task 3: Integrate PIT into kernel init and build

Wire PIT initialization into `kernel_main` and add new source files to the Makefile.

**Changes in `kernel.c`:**
- `#include "pit.h"`.
- Add `pit_init(100)` after `pic_init()` and before `sti`.
- Log messages: `"kernel: initializing pit...\n"` and `"kernel: pit ready\n"`.

**Changes in `kernel/Makefile`:**
- Add `src/pit.c` to `SRCS_C`.

#### Affected Files

- `kernel/src/kernel.c`
- `kernel/Makefile`

#### Dependencies

- Task 2 (pit.c/pit.h must exist).

#### Risks

- None significant. Standard init sequence addition.

## Affected Files

- `kernel/src/idt.c`
- `kernel/src/idt.h`
- `kernel/src/pit.c` (new)
- `kernel/src/pit.h` (new)
- `kernel/src/kernel.c`
- `kernel/Makefile`
