---
number: 13
title: F-013: System call interface
labels: [status: unassigned]
url: https://github.com/Thu-Min/tuna-os/issues/13
---

## F-013: System call interface

**Branch:** `feature/syscall-interface`
**Depends on:** F-012

Implement syscall mechanism (INT or SYSCALL/SYSRET) allowing user-mode code to request kernel services through a numbered dispatch table.

## Acceptance Criteria

### AC-1
Given user-mode code in ring 3, when syscall invoked, then control transfers to kernel handler in ring 0

### AC-2
Given handler receives syscall number and args, when number matches registered handler, then handler is called with args

### AC-3
Given sys_write is implemented, when user code calls it with a string, then string is printed to serial and control returns to user mode

### AC-4
Given invalid syscall number, when handler looks it up, then error code returned to user mode without kernel crash
