---
number: 10
title: F-010: Context switching
labels: [status: unassigned]
url: https://github.com/Thu-Min/tuna-os/issues/10
---

## F-010: Context switching

**Branch:** `feature/context-switching`
**Depends on:** F-009

Implement low-level context switch that saves/restores register state to multiplex the CPU between kernel threads.

## Acceptance Criteria

### AC-1
Given two threads exist, when context switch from A to B, then B's registers are restored and execution continues at B's RIP

### AC-2
Given A was running and switched away, when switched back, then A resumes exactly where it left off

### AC-3
Given context switch is in assembly, when saving state, then all callee-saved registers and RSP are preserved

### AC-4
Given two threads print to serial in loops, when context switches alternate, then interleaved output appears
