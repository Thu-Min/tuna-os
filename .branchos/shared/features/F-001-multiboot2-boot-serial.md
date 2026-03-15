---
id: F-001
title: Multiboot2 boot and serial output
status: complete
milestone: M1
branch: feature/multiboot2-boot-serial
issue: 1
workstream: null
---

Boot a bare-metal x86_64 kernel via GRUB Multiboot2, transition from 32-bit protected mode to 64-bit long mode, and output text over COM1 serial.

## Acceptance Criteria

### AC-1
Given the kernel ELF is packaged into a GRUB ISO
When QEMU boots the ISO in BIOS mode
Then the kernel prints "hello from tuna os!" to the serial console

### AC-2
Given the boot assembly sets up identity-mapped page tables
When the CPU transitions from 32-bit protected mode
Then long mode is enabled and kernel_main executes in 64-bit mode

### AC-3
Given the serial driver is initialized on COM1 (0x3F8)
When serial_write is called with a string
Then each character is transmitted over the UART with CR/LF conversion
