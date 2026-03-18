# Decisions

### Context switch saves only callee-saved registers

**Phase:** 1
**Context:** Need to decide what register state to save/restore during a context switch.
**Decision:** Save/restore only the 6 callee-saved registers (RBX, RBP, R12-R15) plus RSP. The switch is always called from C code, so caller-saved registers are already handled by the compiler.
**Alternatives considered:**
- Save all general-purpose registers (unnecessary overhead, the ABI handles caller-saved)
- Use `iret`-based switching (more complex, needed for user-mode but not kernel-to-kernel)

---

### Timer-driven preemptive scheduling via PIT hook

**Phase:** 1
**Context:** Need a mechanism to trigger context switches without cooperative yielding.
**Decision:** Call `schedule()` from the PIT IRQ handler every N ticks. This reuses existing PIT infrastructure and provides preemptive multitasking.
**Alternatives considered:**
- Cooperative-only yielding (simpler but doesn't satisfy AC-4's interleaved output requirement without explicit yield calls in loops)
- Separate timer for scheduling (unnecessary complexity, PIT is already running)

---

### Separate switch.S assembly file

**Phase:** 1
**Context:** The `switch_to` function must be in assembly for direct register manipulation.
**Decision:** Create `kernel/src/switch.S` as a new assembly file, matching the project convention of dedicated `.S` files for assembly routines.
**Alternatives considered:**
- Inline assembly in C (fragile, hard to control register allocation for this use case)
- Add to existing `interrupts.S` (unrelated concern, better separation)

---
