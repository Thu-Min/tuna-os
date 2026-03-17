---
id: F-005
title: Physical memory map detection
status: unassigned
milestone: M2
branch: feature/physical-memory-map
issue: 5
workstream: physical-memory-map
---

Parse the Multiboot2 memory map tag passed by GRUB to detect available physical memory regions, distinguishing usable RAM from reserved areas.

## Acceptance Criteria

### AC-1
Given GRUB passes Multiboot2 information to the kernel
When the boot code preserves the Multiboot2 info pointer
Then kernel_main can access the Multiboot2 tag structure

### AC-2
Given the Multiboot2 info is accessible
When the memory map tag is parsed
Then each memory region's base address, length, and type are extracted

### AC-3
Given the memory map has been parsed
When the kernel logs the detected regions to serial
Then usable RAM regions and reserved regions are clearly listed with addresses and sizes
