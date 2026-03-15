---
id: F-007
title: Kernel page table management
status: unassigned
milestone: M2
branch: feature/kernel-page-tables
issue: null
workstream: null
---

Replace the boot-time identity map with a proper kernel page table infrastructure that can map and unmap virtual pages to physical frames on demand.

## Acceptance Criteria

### AC-1
Given the page frame allocator is working
When the kernel page table manager is initialized
Then a new PML4 is created with the kernel identity-mapped
And CR3 is loaded with the new page table root

### AC-2
Given the page table manager is active
When a virtual address is mapped to a physical frame
Then the corresponding PTE is created with the requested flags (present, writable, etc.)
And intermediate page table levels are allocated as needed

### AC-3
Given a virtual page is mapped
When it is unmapped
Then the PTE is cleared
And the TLB entry is invalidated

### AC-4
Given the new page tables are loaded
When the kernel continues execution
Then all existing kernel code and data remain accessible without faulting
