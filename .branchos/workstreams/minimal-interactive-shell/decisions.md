# Decisions

### sys_read via serial polling

**Phase:** 1
**Context:** Shell needs to read keyboard input from serial.
**Decision:** `sys_read` (syscall 0) polls serial port in a loop until data ready, returns byte. Simple blocking read.
**Alternatives considered:**
- IRQ-driven with ring buffer — overkill for single-task shell.

---

### sys_exec with frame save/restore

**Phase:** 1
**Context:** Need to run child ELF and return to shell afterward.
**Decision:** `sys_exec` saves shell's interrupt frame, modifies frame to child's entry/stack, returns via IRETQ. On `sys_exit`, restore shell frame.
**Alternatives considered:**
- Separate kernel task — requires task management, blocking wait, per-process address spaces.
- No return (halt after child) — doesn't satisfy AC-4.

---

### sys_exit syscall for child termination

**Phase:** 1
**Context:** Child programs need graceful termination that returns to caller.
**Decision:** `sys_exit` (syscall 4) restores caller's saved frame and returns to it.
**Alternatives considered:**
- CLI/GPF termination — ungraceful, doesn't return to shell.
- Return from `_start` — requires trampoline setup.

---
