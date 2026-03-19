# Phase 1 Plan

## Objective

Implement an `int 0x80` syscall interface with a numbered dispatch table, a `sys_write` handler, invalid-syscall error handling, and an end-to-end user→kernel→user round-trip demo — satisfying all four F-013 acceptance criteria.

## Tasks

### Task 1: Add ISR stub for vector 0x80 in interrupts.S

Add `ISR_NOERR 128` to `interrupts.S` to create a stub for vector 0x80. This reuses the existing `isr_common_stub` path (push vector, save registers, call `isr_dispatch`, restore, IRETQ).

Also declare the stub as `extern` in a header so it can be registered by `idt_set_gate`.

#### Affected Files

- `kernel/src/interrupts.S`

#### Dependencies

None.

#### Risks

- The stub must use `ISR_NOERR` (no error code pushed by CPU for software interrupts).

### Task 2: Register vector 0x80 as DPL=3 trap gate

Update `idt_init()` to register `isr_stub_128` at vector 0x80 with type_attr `0xEF` (present, DPL=3, trap gate). This allows ring 3 code to invoke `int $0x80`.

The existing gates use `0x8E` (DPL=0 interrupt gate). A new constant `IDT_GATE_TRAP_USER` = `0xEF` is needed.

Export the `isr_stub_128` symbol from `interrupts.S` and declare it extern in `idt.c`.

#### Affected Files

- `kernel/src/idt.c`

#### Dependencies

Task 1 (the stub must exist).

#### Risks

- Trap gates don't clear IF — interrupts remain enabled during syscall handling. Acceptable for now since handlers are short.

### Task 3: Create syscall dispatch module (syscall.c, syscall.h)

Create `syscall.c`/`syscall.h` with:
- `syscall_init()` — registers built-in syscalls in the dispatch table.
- `syscall_dispatch(frame)` — called from `isr_dispatch` when vector == 128. Reads RAX (syscall number) from the interrupt frame, looks up the handler, calls it with args from RDI/RSI/RDX/R10/R8/R9, writes return value back to frame->rax.
- A static dispatch table `syscall_table[MAX_SYSCALLS]` of function pointers.
- `syscall_register(number, handler)` for registering handlers.

If the syscall number is out of range or unregistered, return -1 in RAX (AC-4).

#### Affected Files

- `kernel/src/syscall.c`
- `kernel/src/syscall.h`

#### Dependencies

None (standalone module).

#### Risks

- Must update frame->rax (not a local variable) so the value is restored by `POP_GENERAL_REGS` and returned to user mode.

### Task 4: Implement sys_write (syscall 1)

Implement `sys_write(buf, len)` as syscall number 1. Takes user buffer pointer (RDI) and length (RSI). Validates the pointer is in user-accessible range (basic sanity check: addr < some bound, addr+len doesn't overflow). Writes `len` bytes from `buf` to serial one character at a time. Returns the number of bytes written.

Register `sys_write` in `syscall_init()`.

#### Affected Files

- `kernel/src/syscall.c`

#### Dependencies

Task 3 (dispatch infrastructure).

#### Risks

- No true memory validation — just a range check. Sufficient for this phase.

### Task 5: Hook syscall dispatch into isr_dispatch

Update `isr_dispatch()` in `idt.c` to detect vector 128 and call `syscall_dispatch(frame)` instead of the exception/IRQ paths. This must happen before the exception handler logic to avoid treating syscalls as exceptions.

#### Affected Files

- `kernel/src/idt.c`

#### Dependencies

Tasks 2 and 3.

#### Risks

- None — straightforward vector check and function call.

### Task 6: Update usermode test to invoke sys_write

Replace the `cli` instruction in `usermode.c`'s user function with an `int $0x80` invocation:
- Set RAX = 1 (sys_write)
- Set RDI = pointer to string
- Set RSI = string length
- Execute `int $0x80`

The string must be on a user-accessible page. Since the user function's code page is already mapped with USER flag, a string literal in the same translation unit will be on the same or adjacent page (needs verification — may need to place the string in a mapped location explicitly).

After the syscall returns, the user function should also test an invalid syscall (e.g., RAX = 999, `int $0x80`) to verify error return, then halt gracefully.

#### Affected Files

- `kernel/src/usermode.c`

#### Dependencies

Tasks 4 and 5.

#### Risks

- String literal may not be on a USER-mapped page (it could be in .rodata which is a different page). May need to construct the string on the user stack or map the .rodata page with USER flag.

### Task 7: Wire up in kernel_main and update build

Add `syscall_init()` call to `kernel_main` (after IDT init, before usermode test). Update Makefile to compile `syscall.c`.

#### Affected Files

- `kernel/src/kernel.c`
- `kernel/Makefile`

#### Dependencies

Tasks 1–6.

#### Risks

- Init ordering: `syscall_init` must run before `usermode_test` but after `idt_init`.

### Task 8: Build and validate in QEMU

Build and boot in QEMU. Verify serial output shows:
1. `int 0x80` transfers control to kernel handler (AC-1)
2. Syscall number dispatches to registered handler (AC-2)
3. `sys_write` prints user string to serial and returns to user mode (AC-3)
4. Invalid syscall returns error without crash (AC-4)

#### Affected Files

- `kernel/Makefile`

#### Dependencies

Tasks 1–7.

#### Risks

- Triple fault if DPL or gate type is wrong. Debug with QEMU `-d int`.

## Affected Files

- `kernel/src/interrupts.S`
- `kernel/src/idt.c`
- `kernel/src/syscall.c`
- `kernel/src/syscall.h`
- `kernel/src/usermode.c`
- `kernel/src/kernel.c`
- `kernel/Makefile`
