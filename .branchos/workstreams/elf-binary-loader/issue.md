---
number: 14
title: F-014: ELF binary loader
labels: [status: unassigned]
url: https://github.com/Thu-Min/tuna-os/issues/14
---

## F-014: ELF binary loader

**Branch:** `feature/elf-loader`
**Depends on:** F-007, F-013

Parse and load ELF64 binaries into user-mode virtual address space, set up user stack, and jump to entry point in ring 3.

## Acceptance Criteria

### AC-1
Given valid ELF64 binary in memory, when header parsed, then PT_LOAD segments identified with addresses, sizes, and permissions

### AC-2
Given segments parsed, when mapped into user address space, then each loaded at specified VA with correct page permissions

### AC-3
Given ELF loaded and user stack allocated, when execution jumps to entry point, then user program runs in ring 3 and can make syscalls

### AC-4
Given invalid ELF binary, when loader attempts parse, then error reported to serial without kernel crash
