---
id: F-008
title: Kernel heap allocator
status: unassigned
milestone: M2
branch: feature/kernel-heap
issue: 8
workstream: null
---

Implement a kernel heap allocator (kmalloc/kfree) that provides arbitrary-sized dynamic memory allocation backed by the page frame allocator and virtual memory system.

## Acceptance Criteria

### AC-1
Given the virtual memory system is initialized
When the heap is initialized with a virtual address range
Then the heap region is mapped to physical frames
And the allocator metadata is set up

### AC-2
Given the heap is initialized
When kmalloc is called with a size
Then a pointer to a usable memory block of at least that size is returned
And the block is properly aligned (minimum 8-byte alignment)

### AC-3
Given memory was allocated with kmalloc
When kfree is called on the pointer
Then the memory is returned to the free pool
And can be reused by subsequent kmalloc calls

### AC-4
Given the heap is under pressure
When kmalloc needs more backing memory
Then additional physical pages are allocated and mapped into the heap region
