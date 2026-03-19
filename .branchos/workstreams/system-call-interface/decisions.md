# Decisions

### INT 0x80 over SYSCALL/SYSRET

**Phase:** 1
**Context:** Two mechanisms for user→kernel transition: software interrupt (`int 0x80`) and the dedicated `syscall` instruction. SYSCALL is faster but requires MSR configuration.
**Decision:** Use `int 0x80` for initial syscall interface. Simpler with existing IDT infrastructure. SYSCALL/SYSRET deferred.
**Alternatives considered:**
- SYSCALL/SYSRET — faster but requires STAR/LSTAR/SFMASK MSR setup, out of scope.

---

### Dedicated ISR stub for vector 0x80

**Phase:** 1
**Context:** Existing ISR stub table covers vectors 0–47. Extending to 129 entries wastes space.
**Decision:** Add a single `ISR_NOERR 128` stub in `interrupts.S` and register it directly via `idt_set_gate`. Existing dispatch path handles arbitrary vectors.
**Alternatives considered:**
- Extend stub table to 129 entries — wastes ~640 bytes.
- Separate syscall entry point — more work, less code reuse.

---

### Linux x86_64 syscall register convention

**Phase:** 1
**Context:** Need a convention for syscall number and argument passing.
**Decision:** RAX = syscall number, RDI/RSI/RDX/R10/R8/R9 = args 1–6, return in RAX.
**Alternatives considered:**
- Custom convention — no benefit, less familiar.
- Stack-based arguments — slower, harder to validate.

---
