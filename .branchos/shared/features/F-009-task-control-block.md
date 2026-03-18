---
id: F-009
title: Task control block and kernel threads
status: in-progress
milestone: M3
branch: feature/task-control-block
issue: 9
workstream: task-control-block-and-kernel-threads
---

Define the task control block (TCB) structure for representing kernel threads, including saved register state, kernel stack, and task metadata. Create and destroy kernel threads.

## Acceptance Criteria

### AC-1
Given the heap allocator is available
When a kernel thread is created with a function pointer
Then a TCB is allocated with a unique task ID, kernel stack, and saved register state
And the initial instruction pointer is set to the provided function

### AC-2
Given a kernel thread has been created
When the thread's saved state is inspected
Then RSP points to a valid kernel stack
And RIP points to the thread entry function

### AC-3
Given multiple kernel threads have been created
When they are enumerated
Then each thread has a distinct task ID and its own kernel stack

### AC-4
Given a kernel thread exists
When it is destroyed
Then its TCB and kernel stack memory are freed
