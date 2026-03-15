# Roadmap: Tuna OS

> A learn-by-building operating system course that takes you from a blank screen to a working x86_64 kernel, one phase at a time.

**Milestones:** 4 | **Features:** 16

---

## M1: Boot & Interrupts (0/4 features complete)

| # | Feature | Status | Depends On |
|---|---------|--------|------------|
| F-001 | Multiboot2 boot and serial output | unassigned | -- |
| F-002 | GDT and IDT with exception handling | unassigned | F-001 |
| F-003 | PIC remapping and IRQ infrastructure | unassigned | F-002 |
| F-004 | PIT timer and tick counter | unassigned | F-003 |

## M2: Memory Management (0/4 features complete)

| # | Feature | Status | Depends On |
|---|---------|--------|------------|
| F-005 | Physical memory map detection | unassigned | F-001 |
| F-006 | Physical page frame allocator | unassigned | F-005 |
| F-007 | Kernel page table management | unassigned | F-006 |
| F-008 | Kernel heap allocator | unassigned | F-007 |

## M3: Processes & Scheduling (0/4 features complete)

| # | Feature | Status | Depends On |
|---|---------|--------|------------|
| F-009 | Task control block and kernel threads | unassigned | F-008 |
| F-010 | Context switching | unassigned | F-009 |
| F-011 | Round-robin scheduler | unassigned | F-004, F-010 |
| F-012 | User mode transition and TSS | unassigned | F-011 |

## M4: Userspace & Shell (0/4 features complete)

| # | Feature | Status | Depends On |
|---|---------|--------|------------|
| F-013 | System call interface | unassigned | F-012 |
| F-014 | ELF binary loader | unassigned | F-007, F-013 |
| F-015 | RAM-based filesystem | unassigned | F-008 |
| F-016 | Minimal interactive shell | unassigned | F-014, F-015 |
