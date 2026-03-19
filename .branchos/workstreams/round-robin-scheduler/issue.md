---
number: 11
title: F-011: Round-robin scheduler
labels: [status: unassigned]
url: https://github.com/Thu-Min/tuna-os/issues/11
---

## F-011: Round-robin scheduler

**Branch:** `feature/round-robin-scheduler`
**Depends on:** F-004, F-010

Preemptive round-robin scheduler using PIT timer interrupt to switch between ready kernel threads at each tick.

## Acceptance Criteria

### AC-1
Given multiple threads in ready queue, when PIT fires, then next thread is selected round-robin and context switched

### AC-2
Given three threads print IDs to serial, then output shows all three making progress over time

### AC-3
Given a thread finishes, when scheduler checks ready queue, then finished thread is removed and others continue

### AC-4
Given only one thread remains, when timer fires, then it continues running without crashing
