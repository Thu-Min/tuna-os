# Phase 1 Plan

## Objective

Build a minimal interactive shell as a user-mode ELF that displays a prompt, reads input, parses commands, lists ramfs files via `ls`, and runs ELF binaries by name with return-to-shell — satisfying all four F-016 acceptance criteria.

## Tasks

### Task 1: Add sys_read syscall (serial polling)

Add `sys_read` (syscall 0) to `syscall.c`. Polls serial port `inb(0x3F8 + 5)` bit 0 until data is available, then reads `inb(0x3F8)` and returns the byte in RAX. This is a blocking call.

Requires adding `io.h` include to `syscall.c` for `inb`.

#### Affected Files

- `kernel/src/syscall.c`

#### Dependencies

None.

#### Risks

- Busy-wait loop blocks the kernel. Acceptable for single-task shell; PIT interrupts still fire if IF is set (trap gate keeps IF enabled).

### Task 2: Add sys_listfiles syscall

Add `sys_listfiles` (syscall 2) to `syscall.c`. Calls `ramfs_list()` internally to print file listing to serial. Returns the number of files enumerated.

Alternatively, could write file entries to a user buffer, but printing directly to serial is simpler and matches the AC requirement.

#### Affected Files

- `kernel/src/syscall.c`

#### Dependencies

None (uses existing `ramfs_list()`).

#### Risks

- None.

### Task 3: Add sys_exec syscall with frame save/restore

Add `sys_exec` (syscall 3) to `syscall.c`. Takes filename pointer (RDI) and length (RSI) from user mode.

Implementation:
1. Copy filename from user memory to a kernel buffer.
2. Look up file in ramfs via `ramfs_find`.
3. If not found, return -1.
4. Save a copy of the current interrupt frame (shell's state) to a static `saved_frame`.
5. Call `elf_load` with the file's data.
6. Modify the current interrupt frame: set RIP to the ELF entry point, RSP to a new user stack, CS/SS to user selectors. The modified frame will be restored by IRETQ, jumping into the child.
7. Set a flag indicating a child is running.

When the child calls `sys_exit`, the kernel restores `saved_frame` and sets RAX=0 (success return for `sys_exec`).

#### Affected Files

- `kernel/src/syscall.c`

#### Dependencies

Tasks 1–2 (not strictly, but logically grouped).

#### Risks

- Frame save/restore must be exact — any mismatch corrupts the shell's state.
- The child's user stack must be at a different address than the shell's stack (to avoid clobbering). Use 0x900000 for child stack.
- The child's ELF segments must not overlap the shell's. Shell at 0x400000, child also at 0x400000 — **this is a conflict**. Need to handle by accepting that the child overwrites the shell's code pages, and on `sys_exit`, the kernel re-loads the shell ELF before restoring the frame. Or: re-map shell pages after child exits.

### Task 4: Add sys_exit syscall

Add `sys_exit` (syscall 4) to `syscall.c`. When called:
1. Check if a child is running (the saved_frame flag).
2. If yes, re-load the shell ELF (to restore its code/data pages that the child may have overwritten at 0x400000).
3. Copy `saved_frame` back to the current interrupt frame.
4. Set frame->rax = 0 (success return for the shell's `sys_exec` call).
5. Return — IRETQ pops the restored frame, resuming the shell.

If no child is running (shell itself calls `sys_exit`), halt the system.

#### Affected Files

- `kernel/src/syscall.c`

#### Dependencies

Task 3 (frame save mechanism).

#### Risks

- Re-loading the shell ELF on every child exit adds overhead but ensures shell code pages are correct even if the child overwrote them.

### Task 5: Update hello.elf to use sys_exit

Update `user/hello.c` to call `sys_exit(0)` instead of `cli` at the end. This allows returning to the shell after execution.

#### Affected Files

- `user/hello.c`

#### Dependencies

Task 4 (sys_exit must exist).

#### Risks

- None.

### Task 6: Create shell user program

Create `user/shell.c` — a minimal interactive shell:
- Displays `tuna> ` prompt via `sys_write`.
- Reads one character at a time via `sys_read`, echoes it.
- Handles backspace (delete last char, move cursor back).
- On Enter, null-terminates the input buffer.
- Compares command: `ls` → call `sys_listfiles`, `exit` → call `sys_exit`, anything else → call `sys_exec(command)`.
- Loops back to prompt after command completes.

Update `user/Makefile` to build `shell.elf` with the same linker script (base 0x400000).

#### Affected Files

- `user/shell.c`
- `user/Makefile`

#### Dependencies

Tasks 1–5 (all syscalls must work).

#### Risks

- String comparison without libc — need simple `str_eq` helper.
- Must build string on stack for syscall args (same .rodata issue as F-013). Shell can use `sys_write` with stack-built strings, or keep strings short enough that the compiler uses immediate stores.

### Task 7: Embed shell.elf and update kernel boot

Update `kernel/src/user_programs.S` to embed both `hello.elf` and `shell.elf`.

Update `kernel_main` to:
1. Register both binaries in ramfs: `ramfs_create("hello.elf", ...)` and `ramfs_create("shell.elf", ...)`.
2. Load and run `shell.elf` via `usermode_exec_elf` (the shell becomes the init program).
3. Remove the old direct hello.elf test.

#### Affected Files

- `kernel/src/user_programs.S`
- `kernel/src/kernel.c`

#### Dependencies

Task 6.

#### Risks

- Two `.incbin` directives need distinct symbol names.

### Task 8: Build and validate in QEMU

Build and test interactively in QEMU. Verify:
1. Shell prompt `tuna> ` appears (AC-1)
2. Typing `ls` + Enter lists ramfs files (AC-3)
3. Typing `hello.elf` + Enter runs it, prints output, returns to shell (AC-4)
4. Input is echoed and parsed correctly (AC-2)

#### Affected Files

- `kernel/Makefile`

#### Dependencies

Tasks 1–7.

#### Risks

- Interactive testing requires manual QEMU input. Can also automate with `-chardev` pipe.

## Affected Files

- `kernel/src/syscall.c`
- `kernel/src/kernel.c`
- `kernel/src/user_programs.S`
- `user/hello.c`
- `user/shell.c`
- `user/Makefile`
- `kernel/Makefile`
