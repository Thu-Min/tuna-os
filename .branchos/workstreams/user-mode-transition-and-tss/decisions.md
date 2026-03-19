# Decisions

### GDT segment ordering

**Phase:** 1
**Context:** x86_64 GDT segment layout affects both IRETQ and future SYSRET/SYSCALL usage. SYSRET requires user segments follow a specific pattern relative to STAR MSR.
**Decision:** Use ordering: null(0), kernel code(0x08), kernel data(0x10), user data(0x18), user code(0x20), TSS(0x28). User data before user code follows the SYSRET convention. User selectors OR'd with RPL=3 (0x1B and 0x23).
**Alternatives considered:**
- User code before user data — requires GDT restructure when adding SYSCALL/SYSRET later.
- Separate user and kernel GDTs — unnecessary complexity.

---

### TSS descriptor as two GDT slots

**Phase:** 1
**Context:** In 64-bit mode, a TSS descriptor is 16 bytes (128 bits), consuming two GDT entry slots. The upper 8 bytes hold the high 32 bits of the TSS base address.
**Decision:** GDT slots 5 and 6 hold the TSS descriptor. TSS selector is 0x28 (5 * 8). Second slot (index 6) is the upper half.
**Alternatives considered:**
- None — mandated by x86_64 architecture.

---

### IRETQ for ring 3 transition (not SYSRET)

**Phase:** 1
**Context:** Two mechanisms exist for returning to user mode: IRETQ (general) and SYSRET (fast path, requires MSR setup).
**Decision:** Use IRETQ for initial user-mode transition. Simpler, no MSR configuration needed. SYSRET added later with syscall interface.
**Alternatives considered:**
- SYSRET — faster but requires STAR/LSTAR/SFMASK MSR setup, out of scope.

---
