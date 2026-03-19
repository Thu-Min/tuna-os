# Phase 1 Execution

## Completed Tasks

### Task 1: Add sys_read syscall
- **Status:** Complete
- **Commits:** 57b8461
- **Notes:** Polls `inb(0x3F8 + 5)` bit 0 until data ready, returns byte. Blocking call.

### Task 2: Add sys_listfiles syscall
- **Status:** Complete
- **Commits:** 57b8461
- **Notes:** Calls `ramfs_list()` internally, returns 0.

### Task 3: Add sys_exec syscall with frame save/restore
- **Status:** Complete
- **Commits:** 57b8461
- **Notes:** Saves caller's interrupt frame to static `saved_frame`, loads child ELF via `elf_load`, allocates child stack at 0x900000, modifies frame to child entry/stack, returns via IRETQ into child.

### Task 4: Add sys_exit syscall
- **Status:** Complete
- **Commits:** 57b8461
- **Notes:** Re-loads shell ELF (child overwrites same 0x400000 pages), restores `saved_frame`, sets RAX=0 (success return for sys_exec). Shell-itself-exit halts.

### Task 5: Update hello.elf to use sys_exit
- **Status:** Complete
- **Commits:** 57b8461
- **Notes:** Replaced `cli` with `sys_exit(0)`. Child now returns gracefully to shell.

### Task 6: Create shell user program
- **Status:** Complete
- **Commits:** 57b8461
- **Notes:** `user/shell.c` with prompt loop, char-by-char input via sys_read, echo, backspace support, command parsing. Builtin `ls` calls sys_listfiles, `exit` calls sys_exit, anything else tries sys_exec. Uses helper wrappers (syscall0, syscall2, write_str, etc.).

### Task 7: Embed shell.elf and update kernel boot
- **Status:** Complete
- **Commits:** 57b8461
- **Notes:** `user_programs.S` embeds both hello.elf and shell.elf. kernel_main registers both in ramfs, calls `syscall_set_shell_elf` for re-loading, boots into shell. Removed old test task functions.

### Task 8: Build and validate in QEMU
- **Status:** Complete
- **Commits:** N/A (validation only)
- **Notes:** All 4 ACs verified via automated QEMU testing:
  - AC-1: `tuna> ` prompt displayed after boot
  - AC-2: Input echoed, parsed correctly (ls, hello.elf, badcmd all handled)
  - AC-3: `ls` lists "shell.elf 14000 bytes" and "hello.elf 10472 bytes"
  - AC-4: `hello.elf` runs ("Hello from ELF!"), returns to shell prompt

  Bug fixed: TLB stale entries caused child ELF to execute with wrong data when pages were remapped at same virtual address. Fixed by adding `invlpg` after `vmm_map_page` in `elf_load`.

## Blockers

None
