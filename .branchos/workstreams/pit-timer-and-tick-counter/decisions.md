# Decisions

### IRQ handler registration lives in idt.c

**Phase:** 1
**Context:** We need somewhere to put the `irq_register_handler()` function and the handler table.
**Decision:** Keep it in `idt.c` alongside `isr_dispatch`, since the dispatcher is what calls registered handlers. This avoids creating a new file for a small addition.
**Alternatives considered:**
- New `irq.c` file — cleaner separation but premature for 1 function and a 16-entry array.
- Put it in `pic.c` — conflates PIC hardware control with dispatch logic.

---

### PIT module as new pit.c/pit.h pair

**Phase:** 1
**Context:** PIT initialization and tick management need a home.
**Decision:** Create `pit.c` and `pit.h` following existing conventions (module-prefixed functions, static globals, `#pragma once` header).
**Alternatives considered:**
- Adding to an existing file — no suitable candidate; PIT is a distinct hardware subsystem.

---

### Log ticks once per second

**Phase:** 1
**Context:** Need to confirm timer operation via serial without flooding output.
**Decision:** Log tick count every 100 ticks (once per second at 100 Hz) inside the timer handler.
**Alternatives considered:**
- Log from idle loop — would work but adds coupling between kernel_main and tick internals.
- Log every tick — too noisy at 100 Hz.

---
