# Phase 1 Discussion

## Goal

Configure the PIT channel 0 for periodic IRQ0 interrupts, register a timer IRQ handler, maintain a monotonic tick counter, and log periodic tick values to serial — delivering all four acceptance criteria for F-004.

## Requirements

1. **PIT initialization** — Program PIT channel 0 in rate generator mode (mode 2) with a divisor that yields ~100 Hz (divisor = 1193182 / 100 ≈ 11932). Expose `pit_init(uint32_t frequency)` so the frequency is configurable.

2. **IRQ handler registration** — The current `isr_dispatch` in `idt.c` logs IRQs generically. Add a function-pointer table so subsystems can register handlers per IRQ line (e.g., `irq_register_handler(uint8_t irq, handler_fn)`). The timer handler will be the first user.

3. **Timer IRQ handler** — Register a handler for IRQ 0 (vector 32) that increments a `static volatile uint64_t tick_count`. EOI is already sent by the dispatcher after the handler returns.

4. **Tick query API** — Expose `pit_get_ticks(void)` returning the current tick count as a monotonically increasing `uint64_t`.

5. **Serial logging** — On boot, log the configured frequency. Periodically log tick values to serial (e.g., every N ticks) to confirm timer operation per AC-4. This should be in the handler itself or in `kernel_main`'s idle loop.

6. **Unmask IRQ 0** — Call `irq_unmask(0)` after PIT configuration so interrupts begin firing.

7. **Init sequence update** — Add `pit_init(100)` to `kernel_main` after `pic_init()` and before `sti`.

## Assumptions

- **Single phase** — F-004 is small enough to deliver in one phase. The PIC infrastructure from F-003 provides everything needed (PIC init, EOI, mask/unmask, ISR stubs for vectors 32-47).
- **No IRQ handler table yet** — The existing `isr_dispatch` has no callback registration. We'll add a simple function-pointer array (`irq_handlers[16]`) to support this and future IRQ users.
- **EOI in dispatcher** — The current dispatcher already sends EOI after handling IRQs. We'll keep this pattern (EOI sent by dispatcher, not by individual handlers) for consistency.
- **100 Hz default** — Standard tick rate for an educational OS. Fast enough for scheduling later, slow enough to not flood serial output.
- **No sleep/delay API yet** — That's a future concern. This phase only provides raw tick counting.

## Unknowns

1. **Where to put the IRQ handler table** — Should the `irq_register_handler` function live in `idt.c` (alongside the dispatcher) or in a new `irq.c`? Since `pic.c` already handles PIC-specific concerns, adding handler registration to `idt.c` keeps dispatch logic co-located. Alternatively, a new `irq.c` would separate IRQ dispatch from IDT setup.

2. **Serial logging frequency** — Logging every tick at 100 Hz would flood output. Need to decide on a reasonable interval (e.g., every 100 ticks = once per second).

## Decisions

### IRQ handler registration lives in idt.c

**Phase:** 1
**Context:** We need somewhere to put the `irq_register_handler()` function and the handler table.
**Decision:** Keep it in `idt.c` alongside `isr_dispatch`, since the dispatcher is what calls registered handlers. This avoids creating a new file for a small addition.
**Alternatives considered:**
- New `irq.c` file — cleaner separation but premature for 1 function and a 16-entry array.
- Put it in `pic.c` — conflates PIC hardware control with dispatch logic.

### PIT module as new pit.c/pit.h pair

**Phase:** 1
**Context:** PIT initialization and tick management need a home.
**Decision:** Create `pit.c` and `pit.h` following existing conventions (module-prefixed functions, static globals, `#pragma once` header).
**Alternatives considered:**
- Adding to an existing file — no suitable candidate; PIT is a distinct hardware subsystem.

### Log ticks once per second

**Phase:** 1
**Context:** Need to confirm timer operation via serial without flooding output.
**Decision:** Log tick count every 100 ticks (once per second at 100 Hz) inside the timer handler.
**Alternatives considered:**
- Log from idle loop — would work but adds coupling between kernel_main and tick internals.
- Log every tick — too noisy at 100 Hz.
