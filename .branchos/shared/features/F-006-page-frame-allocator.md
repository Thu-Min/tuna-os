---
id: F-006
title: Physical page frame allocator
status: unassigned
milestone: M2
branch: feature/page-frame-allocator
issue: null
workstream: null
---

Implement a physical page frame allocator that tracks free and used 4 KiB pages using a bitmap, initialized from the detected memory map.

## Acceptance Criteria

### AC-1
Given the memory map has been parsed
When the page frame allocator is initialized
Then all usable RAM pages are marked as free
And pages overlapping the kernel image or reserved regions are marked as used

### AC-2
Given the allocator is initialized
When a page is requested via alloc_frame
Then a free physical page address is returned
And the page is marked as used in the bitmap

### AC-3
Given a page was previously allocated
When it is returned via free_frame
Then the page is marked as free in the bitmap
And can be allocated again

### AC-4
Given all usable pages have been allocated
When another page is requested
Then the allocator returns a null/error indicator without crashing
