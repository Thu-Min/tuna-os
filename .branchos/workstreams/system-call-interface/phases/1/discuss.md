# Phase 1 Discussion

## Goal

Implement a syscall mechanism allowing user-mode code (ring 3) to request kernel services through a numbered dispatch table. The initial implementation uses `int 0x80` with a DPL=3 trap gate. A `sys_write` syscall demonstrates end-to-end user→kernel→user round-trip with serial output.

## Requirements

1. **Syscall entry via INT 0x80 (AC-1):** Install a trap gate at vector 0x80 with DPL=3 so ring 3 code can invoke `int $0x80`. The ISR saves registers, extracts the syscall number from RAX and arguments from RDI/RSI/RDX/R10/R8/R9 (Linux convention), dispatches to a handler, places the return value in RAX, and returns via IRETQ.

2. **Numbered dispatch table (AC-2):** A static array of function pointers indexed by syscall number. The dispatcher looks up the handler by number, calls it with the arguments, and returns the result. If the number is out of range or the slot is NULL, return an error code (e.g., -1).

3. **sys_write implementation (AC-3):** Syscall 1 (`sys_write`) takes a buffer pointer and length from user-mode, validates the pointer (basic bounds check), and writes the data to serial via `serial_write`. Returns number of bytes written. Control returns to user mode after the syscall.

4. **Invalid syscall handling (AC-4):** If the syscall number doesn't match a registered handler, the dispatcher returns -1 (or -ENOSYS) to user mode in RAX without crashing the kernel.

## Assumptions

- The `int 0x80` approach is chosen over SYSCALL/SYSRET for simplicity. SYSCALL/SYSRET requires STAR/LSTAR/SFMASK MSR configuration and is a performance optimization for later.
- The existing ISR stub infrastructure (`interrupts.S`) can be extended to handle vector 0x80. However, vector 0x80 needs special treatment: it must be a trap gate (not interrupt gate) with DPL=3 so ring 3 can invoke it. The existing `ISR_NOERR` macro works for the stub, but `idt_set_gate` needs a new type_attr value.
- The syscall convention follows Linux x86_64: RAX = syscall number, RDI = arg1, RSI = arg2, RDX = arg3, R10 = arg4, R8 = arg5, R9 = arg6. Return value in RAX.
- The user-mode test function from F-012 (`usermode.c`) will be updated to invoke `sys_write` via `int $0x80` instead of executing `cli`. This transforms the GPF test into a syscall round-trip test.
- Pointer validation for `sys_write` will be minimal: check that the buffer address is within user-accessible range (e.g., < some upper bound). Full virtual memory validation is deferred.
- The ISR stub count needs to increase from 48 to at least 129 (0x80 = 128), or vector 0x80 can be handled with a dedicated stub outside the table.

## Unknowns

- **ISR stub for vector 0x80:** The current `isr_stub_table` only covers vectors 0–47. Adding stubs for all vectors up to 0x80 would waste space. A dedicated `isr_stub_128` outside the table, registered directly in `idt_set_gate`, is cleaner. Need to confirm this doesn't break the existing dispatch logic.
- **Return to user mode after syscall:** The `isr_common_stub` already does IRETQ, which should correctly restore ring 3 state (CS, SS, RSP, RFLAGS from the interrupt frame). Need to verify that modifying RAX in the interrupt frame (for the return value) before POP_GENERAL_REGS works correctly — the frame's RAX field must be updated, and the pop sequence restores it.
- **Trap gate vs interrupt gate:** Trap gates don't clear IF on entry (interrupts stay enabled). This means a timer IRQ could fire during syscall handling. For now this is acceptable since syscall handlers are short, but it's worth noting.

## Decisions

### INT 0x80 over SYSCALL/SYSRET

**Phase:** 1
**Context:** Two mechanisms for user→kernel transition: software interrupt (`int 0x80`) and the dedicated `syscall` instruction. SYSCALL is faster (no IDT lookup, no stack switch overhead) but requires STAR/LSTAR/SFMASK MSR setup.
**Decision:** Use `int 0x80` for the initial syscall interface. Simpler to implement with existing IDT infrastructure. SYSCALL/SYSRET optimization deferred.
**Alternatives considered:**
- SYSCALL/SYSRET — faster but requires MSR configuration and careful register handling, out of scope.

---

### Dedicated ISR stub for vector 0x80

**Phase:** 1
**Context:** The existing ISR stub table covers vectors 0–47. Extending to 129 entries wastes space for unused vectors 48–127.
**Decision:** Add a single `ISR_NOERR 128` stub in `interrupts.S` and register it directly via `idt_set_gate`. The `isr_common_stub` and `isr_dispatch` already handle arbitrary vector numbers, so no changes to the dispatch path are needed beyond adding syscall-specific logic for vector 128.
**Alternatives considered:**
- Extend stub table to 129 entries — wastes ~640 bytes for unused stubs.
- Separate syscall entry point that doesn't use `isr_common_stub` — more work, less code reuse.

---

### Linux x86_64 syscall register convention

**Phase:** 1
**Context:** Need a convention for passing syscall number and arguments between user mode and kernel.
**Decision:** Follow the Linux x86_64 convention: RAX = syscall number, RDI/RSI/RDX/R10/R8/R9 = args 1–6, return value in RAX. This is familiar to developers and avoids inventing a custom ABI.
**Alternatives considered:**
- Custom register convention — no benefit, less familiar.
- Stack-based arguments — slower, harder to validate.

---
