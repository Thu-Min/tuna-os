# Phase 1 Discussion

## Goal

Build a minimal interactive shell as a user-mode ELF program that displays a prompt, reads serial input character by character, parses commands, and supports `ls` (list ramfs files) and running ELF binaries from ramfs by name. This is the capstone feature — the first truly interactive user program.

## Requirements

1. **Shell prompt and input (AC-1, AC-2):** The shell displays a prompt (e.g., `tuna> `) on serial, reads characters one at a time via a `sys_read` syscall, echoes them back, accumulates a line buffer, and on Enter, parses the command name and arguments.

2. **ls command (AC-3):** When the user types `ls`, the shell calls a `sys_listfiles` syscall that returns file names from ramfs. The shell prints each file name and size.

3. **Run ELF by name (AC-4):** When the user types a filename that exists in ramfs (e.g., `hello.elf`), the kernel loads and runs it. After the program completes, control returns to the shell. This requires a `sys_exec` syscall that triggers ELF loading in the kernel and waits for the child to finish.

4. **New syscalls needed:**
   - `sys_read` (syscall 0) — read one byte from serial. Blocks until data available. Returns the byte in RAX.
   - `sys_listfiles` (syscall 2) — writes ramfs file listing to serial. Returns number of files.
   - `sys_exec` (syscall 3) — takes filename pointer and length. Kernel loads the ELF from ramfs and runs it. Returns 0 on success, -1 if file not found or invalid ELF.

## Assumptions

- Serial input uses polling (`inb(0x3F8 + 5)` bit 0 = data ready, `inb(0x3F8)` to read). The `sys_read` syscall blocks in a loop until a byte is available. This is acceptable for a single-threaded shell.
- The shell is a separate ELF binary in `user/shell.c`, compiled and embedded alongside `hello.elf` via `.incbin`. Both binaries are registered in ramfs at boot.
- For AC-4 ("shell waits for completion"), the simplest approach is: `sys_exec` runs the child program in the current task's context (replacing the shell temporarily), and when the child exits (via `cli` → GPF or a new `sys_exit` syscall), control returns to the shell. However, this requires returning from user mode to the shell after the child finishes — which is complex.
- **Simpler approach for AC-4:** `sys_exec` loads and runs the child ELF. When the child terminates (e.g., via `sys_exit`), the kernel restores the shell's state and returns to it. This can be done by having `sys_exec` save the shell's interrupt frame, run the child, and on child exit (new `sys_exit` syscall), restore the shell's frame and return.
- The shell runs in an infinite loop: prompt → read → parse → execute → repeat.
- Line editing is minimal: backspace deletes last char, Enter submits. No history, no tab completion.
- Maximum input line length: 128 chars.

## Unknowns

- **Returning to shell after child exec:** This is the hardest part. Options:
  - (A) `sys_exec` saves the shell's user-mode registers, runs the child to completion, then restores shell state and returns. Requires careful stack management.
  - (B) Create the child as a separate kernel task, have the shell's `sys_exec` block until the child task finishes. More complex but cleaner long-term.
  - (C) The child writes output then calls `sys_exit(0)`, which returns to the `sys_exec` handler, which returns to the shell. This is essentially option A.

  **Decision: Use option A/C** — `sys_exec` is a blocking syscall. The kernel loads the child ELF, jumps to it via IRETQ, and when the child calls `sys_exit`, the kernel returns to the `sys_exec` handler which returns to the shell. The key insight: the shell calls `int $0x80` for `sys_exec`, which saves the shell's full register state in the interrupt frame. `sys_exec` then replaces the frame's RIP/RSP with the child's entry/stack and returns via IRETQ into the child. When the child calls `sys_exit` (another `int $0x80`), the kernel restores the *original* shell frame saved during `sys_exec` and returns.

- **Keyboard IRQ vs polling:** Polling is simpler but busy-waits. Keyboard IRQ (IRQ1) would require a buffer and sleep/wake mechanism. Polling is sufficient for this phase.

## Decisions

### sys_read via serial polling

**Phase:** 1
**Context:** Shell needs to read keyboard input from serial.
**Decision:** `sys_read` (syscall 0) polls `inb(0x3F8 + 5)` bit 0 in a loop until data is ready, then returns the byte. Simple blocking read. No IRQ, no buffering.
**Alternatives considered:**
- IRQ-driven with ring buffer — better for multitasking but requires sleep/wake, overkill for single-task shell.

---

### sys_exec with frame save/restore

**Phase:** 1
**Context:** Need to run a child ELF and return to shell afterward.
**Decision:** `sys_exec` saves the current interrupt frame (shell's state), loads the child ELF, modifies the frame to point to the child's entry/stack, and returns via IRETQ into the child. When the child calls `sys_exit`, the kernel restores the saved shell frame and returns to the shell's `sys_exec` call site with a return value.
**Alternatives considered:**
- Separate kernel task for child — cleaner but requires task management, blocking wait, per-process address spaces.
- No return to shell (halt after child) — doesn't satisfy AC-4.

---

### sys_exit syscall for child termination

**Phase:** 1
**Context:** Child programs need a way to terminate and return to the caller.
**Decision:** Add `sys_exit` (syscall 4) that signals child program completion. The kernel's `sys_exit` handler restores the caller's (shell's) saved frame and returns to it.
**Alternatives considered:**
- CLI/GPF termination — works but is ungraceful, doesn't return to shell.
- Return from `_start` — requires stack setup to return to a trampoline, more complex.

---
