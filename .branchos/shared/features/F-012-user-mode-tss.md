---
id: F-012
title: User mode transition and TSS
status: in-progress
milestone: M3
branch: feature/user-mode-tss
issue: 12
workstream: user-mode-transition-and-tss
---

Set up user-mode (ring 3) GDT segments, configure a Task State Segment (TSS) for kernel stack switching on privilege transitions, and execute code in ring 3.

## Acceptance Criteria

### AC-1
Given the kernel GDT has kernel code and data segments
When user-mode segments are added to the GDT
Then ring 3 code and data segment descriptors are present
And the TSS descriptor is installed

### AC-2
Given the TSS is configured with a kernel stack pointer (RSP0)
When a privilege transition from ring 3 to ring 0 occurs
Then the CPU switches to the kernel stack specified in the TSS

### AC-3
Given user-mode segments and TSS are configured
When the kernel jumps to a user-mode function via IRETQ
Then the function executes in ring 3 (CPL=3)

### AC-4
Given code is running in ring 3
When a privileged instruction is executed
Then a general protection fault is raised
And the kernel's exception handler logs it to serial
