---
id: F-010
title: Context switching
status: in-progress
milestone: M3
branch: feature/context-switching
issue: 10
workstream: context-switching
---

Implement the low-level context switch mechanism that saves one thread's register state and restores another's, enabling the kernel to multiplex the CPU between kernel threads.

## Acceptance Criteria

### AC-1
Given two kernel threads exist with different entry functions
When a context switch is performed from thread A to thread B
Then thread B's registers are restored and execution continues at thread B's saved RIP

### AC-2
Given thread A was running and a context switch occurred
When a context switch back to thread A happens
Then thread A resumes exactly where it left off with all registers restored

### AC-3
Given the context switch is implemented in assembly
When it saves the current thread's state
Then all callee-saved registers (RBX, RBP, R12-R15) and RSP are preserved

### AC-4
Given two threads each print to serial in a loop
When context switches alternate between them
Then interleaved output from both threads appears on the serial console
