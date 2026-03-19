# Phase 1 Execution

## Completed Tasks

### Task 1: Add ISR stub for vector 0x80
- **Status:** Complete
- **Commits:** 2287362
- **Notes:** Added `ISR_NOERR 128` to `interrupts.S`. Reuses existing `isr_common_stub` path.

### Task 2: Register vector 0x80 as DPL=3 trap gate
- **Status:** Complete
- **Commits:** 2287362
- **Notes:** Added `IDT_GATE_TRAP_USER = 0xEF` constant. Vector 128 registered with `isr_stub_128` in `idt_init()`.

### Task 3: Create syscall dispatch module
- **Status:** Complete
- **Commits:** 2287362
- **Notes:** `syscall.c/h` with 16-entry dispatch table, `syscall_init()`, `syscall_dispatch()`. Reads RAX from interrupt frame, dispatches to handler, writes return value back to frame->rax for POP_GENERAL_REGS restoration.

### Task 4: Implement sys_write (syscall 1)
- **Status:** Complete
- **Commits:** 2287362
- **Notes:** Takes buf (RDI) and len (RSI), validates non-null and no overflow, writes to serial char by char. Returns byte count.

### Task 5: Hook syscall dispatch into isr_dispatch
- **Status:** Complete
- **Commits:** 2287362
- **Notes:** Vector 128 check added before IRQ/exception dispatch. Calls `syscall_dispatch(frame)` and returns.

### Task 6: Update usermode test to invoke sys_write
- **Status:** Complete
- **Commits:** 2287362
- **Notes:** Rewrote `user_function` as `__attribute__((naked))` pure assembly to avoid compiler-generated .rodata references (first attempt with C char arrays caused page fault because .rodata isn't USER-mapped). Builds strings on stack with immediate byte stores. Tests: (1) sys_write prints "Hello from ring 3!", (2) invalid syscall 999 returns error, (3) sys_write prints "syscall test ok", (4) cli triggers GPF halt.

### Task 7: Wire up in kernel_main and update build
- **Status:** Complete
- **Commits:** 2287362
- **Notes:** Added `syscall_init()` after PIC init, before usermode test. Makefile updated with `syscall.c`.

### Task 8: Build and validate in QEMU
- **Status:** Complete
- **Commits:** N/A (validation only)
- **Notes:** QEMU serial output confirms all 4 ACs:
  - AC-1: `int $0x80` from ring 3 transfers to kernel handler (no #GP)
  - AC-2: Syscall number 1 dispatches to sys_write handler
  - AC-3: "Hello from ring 3!" printed to serial, control returns to user mode
  - AC-4: "[syscall] invalid number=999" logged, user mode continues (no crash)

## Blockers

None
