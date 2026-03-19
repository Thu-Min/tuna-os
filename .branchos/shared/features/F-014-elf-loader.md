---
id: F-014
title: ELF binary loader
status: in-progress
milestone: M4
branch: feature/elf-loader
issue: 14
workstream: elf-binary-loader
---

Parse and load ELF64 binaries into user-mode virtual address space, set up a user stack, and jump to the ELF entry point in ring 3.

## Acceptance Criteria

### AC-1
Given a valid ELF64 binary is available in memory
When the ELF header is parsed
Then the loader identifies PT_LOAD segments with their virtual addresses, sizes, and permissions

### AC-2
Given the ELF segments have been parsed
When the loader maps them into a user-mode address space
Then each segment is loaded at its specified virtual address with correct page permissions (read, write, execute)

### AC-3
Given the ELF is loaded and a user stack is allocated
When execution jumps to the ELF entry point via IRETQ
Then the user program runs in ring 3 and can make system calls

### AC-4
Given an invalid or malformed ELF binary is presented
When the loader attempts to parse it
Then an error is reported to serial without crashing the kernel
