# Decisions

### PIC gets its own module

**Phase:** 1
**Context:** The PIC has its own I/O ports (0x20/0x21, 0xA0/0xA1) and distinct initialization logic separate from the IDT.
**Decision:** Create `pic.c` and `pic.h` as a new module, following the existing convention of one `.c`/`.h` pair per subsystem.
**Alternatives considered:**
- Add PIC code to `idt.c` — rejected because it mixes IDT table management with hardware device control.

---

### Extract port I/O to a shared header

**Phase:** 1
**Context:** `outb`/`inb` are defined as `static inline` in `serial.c`, but the PIC driver also needs them. Duplicating the definitions would violate DRY and was flagged in CONCERNS.md.
**Decision:** Create `io.h` with `static inline` `outb`/`inb` definitions. Update `serial.c` to use it. `pic.c` will also include it.
**Alternatives considered:**
- Duplicate `outb`/`inb` in `pic.c` — rejected, this is the exact concern flagged in the codebase map.
- Put them in a `.c` file — rejected, they should remain `static inline` for performance in a kernel.

---

### Start with direct IRQ dispatch, not a handler table

**Phase:** 1
**Context:** F-003 only needs to log IRQs and send EOI. A function pointer table adds complexity without immediate benefit.
**Decision:** Handle IRQ vectors directly in `isr_dispatch` with an `if (vector >= 32 && vector < 48)` branch. Refactor to a registration table in F-004 when the PIT needs to register a handler.
**Alternatives considered:**
- Build a full `irq_register_handler(uint8_t irq, handler_fn)` table now — rejected as premature for this phase.

---
