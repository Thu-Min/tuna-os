---
id: F-013
title: System call interface
status: unassigned
milestone: M4
branch: feature/syscall-interface
issue: 13
workstream: null
---

Implement a system call mechanism using software interrupts or SYSCALL/SYSRET that allows user-mode code to request kernel services through a numbered dispatch table.

## Acceptance Criteria

### AC-1
Given user-mode code is running in ring 3
When a system call is invoked (via INT or SYSCALL instruction)
Then control transfers to the kernel's syscall handler in ring 0

### AC-2
Given the syscall handler receives a syscall number and arguments
When the number matches a registered handler
Then the corresponding handler function is called with the arguments

### AC-3
Given a sys_write syscall is implemented
When user-mode code calls sys_write with a string
Then the string is printed to the serial console
And control returns to user mode with a result value

### AC-4
Given a user-mode program invokes an invalid syscall number
When the syscall handler looks up the number
Then an error code is returned to user mode without crashing the kernel
