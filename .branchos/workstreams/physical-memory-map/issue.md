---
number: 5
title: "F-005: Physical memory map detection"
labels: [status: unassigned]
url: https://github.com/Thu-Min/tuna-os/issues/5
---

## F-005: Physical memory map detection

**Branch:** `feature/physical-memory-map`
**Depends on:** F-001

Parse the Multiboot2 memory map tag passed by GRUB to detect available physical memory regions.

## Acceptance Criteria

### AC-1
Given GRUB passes Multiboot2 info, when boot code preserves the pointer, then kernel_main can access the tag structure

### AC-2
Given Multiboot2 info is accessible, when the memory map tag is parsed, then each region's base, length, and type are extracted

### AC-3
Given the memory map is parsed, when logged to serial, then usable and reserved regions are clearly listed with addresses and sizes
