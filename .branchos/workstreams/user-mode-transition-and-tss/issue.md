---
number: 12
title: F-012: User mode transition and TSS
labels: [status: unassigned]
url: https://github.com/Thu-Min/tuna-os/issues/12
---

## F-012: User mode transition and TSS

**Branch:** `feature/user-mode-tss`
**Depends on:** F-011

Set up ring 3 GDT segments, configure TSS for kernel stack switching on privilege transitions, and execute code in user mode.

## Acceptance Criteria

### AC-1
Given kernel GDT exists, when user-mode segments added, then ring 3 code/data descriptors and TSS descriptor are present

### AC-2
Given TSS has RSP0 configured, when ring 3 → ring 0 transition occurs, then CPU switches to the kernel stack

### AC-3
Given user segments and TSS configured, when kernel jumps to user function via IRETQ, then it executes in ring 3 (CPL=3)

### AC-4
Given code runs in ring 3, when a privileged instruction executes, then GPF is raised and logged to serial
