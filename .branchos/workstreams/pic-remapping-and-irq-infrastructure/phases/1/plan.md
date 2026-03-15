# Phase 1 Plan

## Objective

Initialize the dual 8259 PIC, remap hardware IRQs to IDT vectors 32–47, extend the ISR stub table and IDT to cover those vectors, provide mask/unmask/EOI primitives, and enable interrupts. The kernel must idle safely with `STI` + `HLT` and all IRQs masked.

## Tasks

### Task 1: Extract port I/O to shared header

Create `kernel/src/io.h` with `static inline` `outb` and `inb` functions. Remove the duplicate definitions from `kernel/src/serial.c` and replace them with `#include "io.h"`.

#### Affected Files

- `kernel/src/io.h` (new)
- `kernel/src/serial.c`

#### Dependencies

None — this is a pure refactor with no behavioral change.

#### Risks

Low. If `io.h` is missing an include guard or has a typo, serial output breaks. Verify `make` compiles cleanly and `make run` still prints the boot messages.

---

### Task 2: Create PIC module

Create `kernel/src/pic.c` and `kernel/src/pic.h` implementing:

- `pic_init()`: Send ICW1–ICW4 to master (0x20/0x21) and slave (0xA0/0xA1). Remap master IRQs to vectors 32–39, slave to 40–47. Mask all IRQs (0xFF to both data ports).
- `pic_eoi(uint8_t irq)`: Send EOI (0x20) to master. If `irq >= 8`, also send EOI to slave first.
- `irq_mask(uint8_t irq)`: Set the bit for the given IRQ in the appropriate PIC's mask register.
- `irq_unmask(uint8_t irq)`: Clear the bit for the given IRQ in the appropriate PIC's mask register.

The header should expose all four functions. The implementation should `#include "io.h"` for port access.

#### Affected Files

- `kernel/src/pic.h` (new)
- `kernel/src/pic.c` (new)

#### Dependencies

Task 1 (io.h must exist).

#### Risks

Incorrect ICW sequence will cause triple-fault or hang. The ICW byte values are well-documented and QEMU is forgiving, but the order matters: ICW1 → ICW2 → ICW3 → ICW4 with no extra reads/writes in between.

---

### Task 3: Extend ISR stubs for IRQ vectors 32–47

In `kernel/src/interrupts.S`:

1. Add 16 new `ISR_NOERR` entries for vectors 32–47 (IRQ 0–15).
2. Extend `isr_stub_table` from 32 entries to 48, adding `.quad isr_stub_32` through `.quad isr_stub_47`.

Update the `extern` declaration in `kernel/src/idt.c`: change `EXCEPTION_COUNT` usage for the stub table size to a new constant (e.g., `ISR_STUB_COUNT 48`) so the IDT init loop installs stubs for vectors 0–47.

#### Affected Files

- `kernel/src/interrupts.S`
- `kernel/src/idt.c`
- `kernel/src/idt.h`

#### Dependencies

None — can be done in parallel with Tasks 1 and 2, but must be done before Task 4.

#### Risks

If the stub table size constant is wrong, stubs for IRQ vectors won't be installed and IRQs will hit empty IDT entries → double fault. Verify by checking that `idt_init` installs 48 gates.

---

### Task 4: Add IRQ dispatch path

Modify `isr_dispatch` in `kernel/src/idt.c` to handle IRQ vectors:

1. Add `#include "pic.h"`.
2. After the exception handling logic, add a branch for vectors 32–47:
   - Log `[irq] irq=N` to serial (where N = vector - 32).
   - Call `pic_eoi(vector - 32)`.
   - Return (do not halt).
3. Keep the existing fatal-exception halt path for vectors 0–31 (except breakpoint).

#### Affected Files

- `kernel/src/idt.c`

#### Dependencies

Tasks 2 and 3 (PIC module and extended stubs must exist).

#### Risks

Forgetting to call `pic_eoi` will cause the PIC to stop delivering further IRQs of the same or lower priority. Forgetting to return will halt the kernel on IRQ delivery.

---

### Task 5: Integrate PIC init and enable interrupts

Update `kernel/src/kernel.c`:

1. Add `#include "pic.h"`.
2. After `idt_init()`, call `pic_init()` and log to serial.
3. After PIC init, enable interrupts with `__asm__ volatile ("sti")` and log to serial.
4. The existing `HLT` loop will now wake on interrupts and re-halt. No other changes needed.

Update `kernel/Makefile`:

1. Add `src/pic.c` to `SRCS_C`.

#### Affected Files

- `kernel/src/kernel.c`
- `kernel/Makefile`

#### Dependencies

Tasks 1–4 (all prior tasks complete).

#### Risks

If any prior task has a bug, `STI` will expose it immediately (triple-fault, hang, or spurious interrupt storm). This task should be done last so all pieces are in place before enabling interrupts.

---

### Task 6: Build and verify

1. Run `make clean && make` to verify clean compilation with `-Wall -Wextra -Werror`.
2. Run `make run` (BIOS) and verify serial output shows:
   - Boot messages through GDT/IDT init.
   - PIC init message.
   - Breakpoint self-test still works.
   - Interrupts enabled message.
   - Kernel enters idle loop without crashing.
3. No IRQ log messages should appear (all IRQs masked).

#### Affected Files

No new file changes — verification only.

#### Dependencies

Task 5.

#### Risks

None — read-only verification step.

## Affected Files

- `kernel/src/io.h` (new)
- `kernel/src/pic.h` (new)
- `kernel/src/pic.c` (new)
- `kernel/src/serial.c`
- `kernel/src/interrupts.S`
- `kernel/src/idt.c`
- `kernel/src/idt.h`
- `kernel/src/kernel.c`
- `kernel/Makefile`
