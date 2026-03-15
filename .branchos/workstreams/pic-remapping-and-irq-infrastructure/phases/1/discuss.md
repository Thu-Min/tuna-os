# Phase 1 Discussion

## Goal

Initialize the dual 8259 PIC (master + slave), remap hardware IRQs away from CPU exception vectors, extend the IDT and ISR stub table to cover IRQ vectors 32–47, and provide mask/unmask and EOI primitives. After this phase, the kernel can safely enable interrupts (`STI`) without triple-faulting, and individual IRQs can be selectively unmasked for use by future drivers (PIT, keyboard, etc.).

## Requirements

1. **PIC initialization (ICW1–ICW4):** Send the full initialization sequence to both the master PIC (ports 0x20/0x21) and slave PIC (ports 0xA0/0xA1). Remap master IRQs 0–7 to IDT vectors 32–39, slave IRQs 8–15 to vectors 40–47.

2. **Mask all IRQs by default:** After init, write 0xFF to both PIC data ports so no IRQs fire until explicitly unmasked. This prevents spurious interrupts before drivers are ready.

3. **IRQ mask/unmask API:** Provide `irq_mask(uint8_t irq)` and `irq_unmask(uint8_t irq)` functions that set/clear individual bits in the PIC mask registers. Must handle routing to the correct PIC (master for 0–7, slave for 8–15).

4. **EOI signaling:** Provide `pic_eoi(uint8_t irq)` that sends the end-of-interrupt command (0x20) to the correct PIC(s). For slave IRQs (8–15), EOI must be sent to both slave and master.

5. **Extend ISR stubs to vectors 32–47:** Add 16 new `ISR_NOERR` entries in `interrupts.S` for IRQ vectors and extend `isr_stub_table` to 48 entries. Update `idt_init` to install these stubs.

6. **IRQ dispatch path:** Modify `isr_dispatch` in `idt.c` to detect IRQ vectors (32–47), call `pic_eoi`, and log the IRQ number to serial. Non-exception vectors should not be treated as fatal.

7. **Enable interrupts:** Add `STI` after PIC init is complete. The kernel must survive in its idle `HLT` loop with interrupts enabled and all IRQs masked.

## Assumptions

- **No APIC needed yet.** The 8259 PIC is sufficient for QEMU's default configuration. APIC/IOAPIC can be addressed in a future feature if needed.
- **Spurious IRQ 7/15 handling is deferred.** These are rare on QEMU and can be handled later. For now, the default IRQ path (log + EOI) is acceptable.
- **No IRQ sharing or chaining.** Each IRQ maps to exactly one handler. A handler registration table can come later with F-004 (PIT) or beyond.
- **The existing `isr_common_stub` assembly path works for IRQs.** IRQ stubs use the same register save/restore and dispatch mechanism as exception stubs — no separate IRQ assembly path needed.

## Unknowns

1. **Should `isr_dispatch` use a function pointer table for IRQ handlers?** A simple `switch` or `if` on the vector number is fine for now, but F-004 (PIT) will need to register a handler. Decision: start with direct dispatch in `isr_dispatch`, refactor to a handler table when F-004 needs it.

2. **Should PIC code live in its own file or extend `idt.c`?** The PIC is a distinct hardware device with its own I/O ports, suggesting a separate `pic.c`/`pic.h` module.

3. **Port I/O helpers (`outb`/`inb`) are currently `static inline` in `serial.c`.** The PIC driver needs them too. Should they be extracted to a shared header?

## Decisions

### PIC gets its own module

**Phase:** 1
**Context:** The PIC has its own I/O ports (0x20/0x21, 0xA0/0xA1) and distinct initialization logic separate from the IDT.
**Decision:** Create `pic.c` and `pic.h` as a new module, following the existing convention of one `.c`/`.h` pair per subsystem.
**Alternatives considered:**
- Add PIC code to `idt.c` — rejected because it mixes IDT table management with hardware device control.

### Extract port I/O to a shared header

**Phase:** 1
**Context:** `outb`/`inb` are defined as `static inline` in `serial.c`, but the PIC driver also needs them. Duplicating the definitions would violate DRY and was flagged in CONCERNS.md.
**Decision:** Create `io.h` with `static inline` `outb`/`inb` definitions. Update `serial.c` to use it. `pic.c` will also include it.
**Alternatives considered:**
- Duplicate `outb`/`inb` in `pic.c` — rejected, this is the exact concern flagged in the codebase map.
- Put them in a `.c` file — rejected, they should remain `static inline` for performance in a kernel.

### Start with direct IRQ dispatch, not a handler table

**Phase:** 1
**Context:** F-003 only needs to log IRQs and send EOI. A function pointer table adds complexity without immediate benefit.
**Decision:** Handle IRQ vectors directly in `isr_dispatch` with an `if (vector >= 32 && vector < 48)` branch. Refactor to a registration table in F-004 when the PIT needs to register a handler.
**Alternatives considered:**
- Build a full `irq_register_handler(uint8_t irq, handler_fn)` table now — rejected as premature for this phase.
