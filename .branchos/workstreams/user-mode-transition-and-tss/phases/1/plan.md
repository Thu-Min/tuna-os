# Phase 1 Plan

## Objective

Extend the GDT with ring 3 segments and a TSS descriptor, configure the TSS for kernel stack switching, and demonstrate user-mode execution via IRETQ with GPF validation — satisfying all four F-012 acceptance criteria.

## Tasks

### Task 1: Create TSS module (tss.c, tss.h)

Create a new TSS module that defines the 64-bit TSS structure, provides `tss_init()` to allocate and configure it, and `tss_set_rsp0()` to update the kernel stack pointer for privilege transitions.

The TSS structure in 64-bit mode is 104 bytes:
- Reserved (4 bytes)
- RSP0, RSP1, RSP2 (8 bytes each)
- Reserved (8 bytes)
- IST1–IST7 (8 bytes each)
- Reserved (10 bytes)
- IOPB offset (2 bytes)

Only RSP0 is needed for this phase. `tss_init()` stores a pointer to the TSS that the GDT module will use to build the TSS descriptor.

#### Affected Files

- `kernel/src/tss.c`
- `kernel/src/tss.h`

#### Dependencies

None.

#### Risks

- TSS structure must be exactly 104 bytes and correctly packed. Any misalignment causes #TS (Invalid TSS) on `ltr`.

### Task 2: Extend GDT with user segments and TSS descriptor

Expand the GDT from 3 entries (null, kernel code, kernel data) to 7 entries:
- Index 0: Null
- Index 1: Kernel code (0x08) — unchanged
- Index 2: Kernel data (0x10) — unchanged
- Index 3: User data (0x18, RPL=3 → selector 0x1B)
- Index 4: User code (0x20, RPL=3 → selector 0x23)
- Index 5–6: TSS descriptor (16 bytes, selector 0x28)

User code segment: access=0xFA (present, DPL=3, executable), flags=0xA0 (long mode).
User data segment: access=0xF2 (present, DPL=3, writable), flags=0xC0 (32-bit granularity).

TSS descriptor: manually construct the 16-byte system segment descriptor with the base address from the TSS module and limit of 103 (sizeof(tss) - 1).

After LGDT, execute `ltr` with selector 0x28 to load the TSS.

Update `gdt.h` to export segment selector constants and `tss_load()`.

#### Affected Files

- `kernel/src/gdt.c`
- `kernel/src/gdt.h`
- `kernel/src/gdt_flush.S`

#### Dependencies

Task 1 (TSS module provides the TSS base address).

#### Risks

- TSS descriptor is 16 bytes in 64-bit mode (two GDT slots). The upper 8 bytes must be written manually as raw bytes, not as a standard `gdt_descriptor`. Getting the base address encoding wrong causes #GP on `ltr`.
- `gdt_flush.S` must not clobber the TSS selector when reloading segment registers — it only touches CS, DS, ES, SS, FS, GS which are all standard selectors.

### Task 3: Map user-mode pages and create IRETQ transition

Create a `usermode.c`/`usermode.h` module that:
1. Allocates a page for the user stack, maps it with `VMM_FLAG_USER | VMM_FLAG_WRITE`.
2. Ensures the page containing the user-mode test function is mapped with `VMM_FLAG_USER`. Since code lives in the kernel identity-mapped region, add the USER flag to the relevant page.
3. Builds an IRETQ stack frame: push SS (0x1B user data), RSP (user stack top), RFLAGS (IF set), CS (0x23 user code), RIP (user function address), then execute IRETQ.
4. The user-mode test function prints to serial (this will GPF since `outb` is privileged — which is exactly AC-4), or uses a simpler approach: attempt `cli` which triggers #GP immediately.

The IRETQ transition will be done in a small assembly helper (`jump_to_usermode`) that sets up the stack frame and executes IRETQ.

#### Affected Files

- `kernel/src/usermode.c`
- `kernel/src/usermode.h`

#### Dependencies

Task 2 (GDT must have user segments and TSS loaded).

#### Risks

- If the user code page isn't mapped with USER flag, a page fault occurs instead of reaching ring 3. Must verify VMM mapping flags.
- The user stack must also be on a USER-flagged page. If allocated from kheap, the heap pages may not have USER set — need to explicitly map a separate page.
- RFLAGS must have IF (bit 9) set in the IRETQ frame, otherwise interrupts are disabled in ring 3 and the timer stops.

### Task 4: Update IDT exception handler for GPF logging

Update `isr_dispatch` in `idt.c` to handle GPF (vector 13) more gracefully when it comes from ring 3. Check the CS field in the interrupt frame — if it has RPL=3, log "GPF from ring 3" with the faulting RIP, then halt (or kill the task). This validates AC-4.

The existing handler already logs vector 13 and halts, but adding ring 3 detection makes the serial output clearly confirm the privilege transition worked.

#### Affected Files

- `kernel/src/idt.c`
- `kernel/src/idt.h`

#### Dependencies

Task 3 (needs ring 3 code to trigger the GPF).

#### Risks

- The interrupt frame pushed by the CPU on a ring change includes SS and RSP from ring 3. Need to verify the `interrupt_frame` struct accounts for this (it should, since IRETQ always pushes SS/RSP in 64-bit mode regardless of privilege change).

### Task 5: Wire up in kernel_main and update build

Add `tss_init()` and user-mode transition calls to `kernel_main`. Update the Makefile to compile the new source files (`tss.c`, `usermode.c`).

The init sequence becomes: ... → gdt_init() → tss_init() → ... → usermode_test() (after PIT/tasks are ready, or as a standalone test before the scheduler).

For this phase, the user-mode test can run before the scheduler demo, since it will GPF and halt. Alternatively, run it as a task that triggers GPF.

#### Affected Files

- `kernel/src/kernel.c`
- `kernel/Makefile`

#### Dependencies

Tasks 1–4.

#### Risks

- Ordering matters: TSS must be initialized before GDT loads the TSS descriptor (or GDT init must call TSS init internally). Need to decide whether `gdt_init` calls `tss_init` or they're separate calls.

### Task 6: Build and validate in QEMU

Build the kernel and boot in QEMU. Verify serial output shows:
1. GDT loaded with user segments and TSS (AC-1)
2. TSS loaded with RSP0 configured (AC-2)
3. IRETQ transitions to ring 3 successfully (AC-3)
4. Privileged instruction triggers GPF from ring 3, logged to serial (AC-4)

#### Affected Files

- `kernel/Makefile`

#### Dependencies

Tasks 1–5.

#### Risks

- Triple fault on IRETQ if segments or TSS are misconfigured. Debug via QEMU `-d int` flag if needed.

## Affected Files

- `kernel/src/tss.c`
- `kernel/src/tss.h`
- `kernel/src/gdt.c`
- `kernel/src/gdt.h`
- `kernel/src/gdt_flush.S`
- `kernel/src/usermode.c`
- `kernel/src/usermode.h`
- `kernel/src/idt.c`
- `kernel/src/idt.h`
- `kernel/src/kernel.c`
- `kernel/Makefile`
