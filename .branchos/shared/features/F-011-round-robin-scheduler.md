---
id: F-011
title: Round-robin scheduler
status: unassigned
milestone: M3
branch: feature/round-robin-scheduler
issue: 11
workstream: null
---

Implement a preemptive round-robin scheduler that uses the PIT timer interrupt to switch between ready kernel threads at each tick.

## Acceptance Criteria

### AC-1
Given multiple kernel threads are in the ready queue
When the PIT timer interrupt fires
Then the scheduler selects the next thread in round-robin order
And a context switch is performed

### AC-2
Given three kernel threads are running
When each thread prints its ID to serial
Then output shows all three threads making progress over time

### AC-3
Given a thread has finished execution
When the scheduler checks the ready queue
Then the finished thread is removed
And remaining threads continue to be scheduled

### AC-4
Given only one thread remains in the ready queue
When the timer fires
Then the scheduler continues running that thread without crashing
