# Phase 1 Discussion

## Goal

Set up the infrastructure for user mode (ring 3) execution: extend the GDT with user-mode code/data segments and a TSS descriptor, configure the TSS with RSP0 for kernel stack switching on privilege transitions, and demonstrate jumping to a user-mode function via IRETQ with GPF validation.

## Requirements

1. **GDT extension (AC-1):** Add ring 3 code segment (index 3, selector 0x18|3=0x1B), ring 3 data segment (index 4, selector 0x20|3=0x23), and a TSS descriptor (index 5–6, takes two GDT slots in 64-bit mode since TSS descriptors are 16 bytes). The GDT must grow from 3 entries to 7 (null + kernel code + kernel data + user data + user code + TSS low + TSS high).

2. **TSS setup (AC-2):** Allocate and initialize a `struct tss` with `rsp0` pointing to the kernel stack. Load the TSS via `ltr` instruction after LGDT. On ring 3 → ring 0 transitions (interrupts, exceptions, syscalls), the CPU reads RSP0 from the TSS to switch to the kernel stack.

3. **IRETQ to user mode (AC-3):** Build an IRETQ frame on the kernel stack with user code/data selectors and a user-mode function address, then execute IRETQ to drop to ring 3. The user-mode code page must be mapped with the `USER` flag in the page tables so CPL=3 can access it.

4. **GPF validation (AC-4):** The user-mode function attempts a privileged instruction (e.g., `cli`, `hlt`, or `outb`) which triggers a General Protection Fault (#GP, vector 13). The existing IDT/exception handler logs this to serial, confirming ring 3 enforcement.

## Assumptions

- The existing GDT infrastructure (`gdt.c`, `gdt_flush.S`) can be extended in-place. The `gdt` array grows from 3 to 7 entries, and `gdt_flush` continues to work (it just reloads segment registers with kernel selectors).
- The user-mode test function will live in kernel address space but on a page mapped with `VMM_FLAG_USER`. This is sufficient for demonstrating ring 3 — actual user-space address separation comes in a later feature.
- IDT gates must use DPL=0 (ring 0) for interrupt/trap gates, which is already the case (`IDT_GATE_INTERRUPT = 0x8E`). This means ring 3 code cannot use `int` to invoke arbitrary ISRs — only hardware interrupts and CPU exceptions will trap into the kernel. This is correct and desired.
- The TSS needs only `rsp0` configured. IST (Interrupt Stack Table) entries can remain zero for now — double-fault IST is a future hardening concern.
- GDT segment ordering follows the System V ABI / Linux convention: kernel code (0x08), kernel data (0x10), user data (0x18|3), user code (0x20|3). Note: user data comes before user code because `sysret` expects this layout. Even though we're using IRETQ (not sysret) for now, adopting this ordering avoids a future GDT restructure.

## Unknowns

- **User-mode page mapping:** The user-mode function's code page and stack page both need `VMM_FLAG_USER` set. Need to verify whether the current VMM identity map (first 4 MiB) can simply have the user flag added to specific pages, or whether a separate user mapping is needed. Since the kernel occupies 1 MiB+, and we can place user code at a known address within the identity-mapped range, adding `VMM_FLAG_USER` to those specific pages should work.
- **User stack allocation:** Ring 3 needs its own stack (can't share the kernel stack). Need to allocate a page for the user stack, map it with `VMM_FLAG_USER | VMM_FLAG_WRITE`, and set RSP in the IRETQ frame to point there.
- **GPF handler behavior:** Currently, vector 13 (GPF) halts the system. For the test, we need it to log the fault and either halt gracefully or return. Since the faulting instruction can't be "fixed," halting after logging is acceptable for this phase.

## Decisions

### GDT segment ordering

**Phase:** 1
**Context:** x86_64 GDT segment layout affects both IRETQ and future SYSRET/SYSCALL usage. SYSRET requires that user segments follow a specific pattern relative to STAR MSR.
**Decision:** Use ordering: null(0), kernel code(0x08), kernel data(0x10), user data(0x18), user code(0x20), TSS(0x28). User data before user code follows the SYSRET convention. User selectors will be OR'd with RPL=3 (0x1B and 0x23).
**Alternatives considered:**
- User code before user data — simpler conceptually but requires GDT restructure when adding SYSCALL/SYSRET later.
- Separate user and kernel GDTs — unnecessary complexity for a single-address-space kernel.

---

### TSS descriptor as two GDT slots

**Phase:** 1
**Context:** In 64-bit mode, a TSS descriptor is 16 bytes (128 bits), consuming two GDT entry slots. The upper 8 bytes hold the high 32 bits of the TSS base address.
**Decision:** GDT slots 5 and 6 will hold the TSS descriptor. The TSS selector is 0x28 (5 * 8). The second slot (index 6) is the upper half and is not independently addressable.
**Alternatives considered:**
- None — this is mandated by the x86_64 architecture.

---

### IRETQ for ring 3 transition (not SYSRET)

**Phase:** 1
**Context:** Two mechanisms exist for returning to user mode: IRETQ (general) and SYSRET (fast path, requires STAR/LSTAR MSR setup).
**Decision:** Use IRETQ for the initial user-mode transition. It's simpler, doesn't require MSR configuration, and works with any segment layout. SYSRET can be added later when implementing the syscall interface.
**Alternatives considered:**
- SYSRET — faster but requires SYSCALL/SYSRET MSR setup (STAR, LSTAR, SFMASK), which is out of scope for this feature.

---
