---
id: F-003
title: PIC remapping and IRQ infrastructure
status: in-progress
milestone: M1
branch: feature/pic-irq-infrastructure
issue: 3
workstream: pic-remapping-and-irq-infrastructure
---

Initialize the 8259 PIC, remap IRQs 0-15 to IDT vectors 32-47 to avoid collision with CPU exceptions, and provide an IRQ dispatch path with proper EOI signaling.

## Acceptance Criteria

### AC-1
Given the IDT is initialized with exception handlers
When the PIC is initialized
Then IRQ 0-7 are remapped to IDT vectors 32-39
And IRQ 8-15 are remapped to IDT vectors 40-47

### AC-2
Given the PIC is remapped
When an IRQ fires
Then the corresponding ISR stub is invoked
And an End-of-Interrupt (EOI) is sent to the correct PIC chip

### AC-3
Given all IRQs are initially masked
When a specific IRQ is unmasked
Then only that IRQ can trigger an interrupt
And all other IRQs remain masked

### AC-4
Given the PIC and IDT are configured
When interrupts are enabled with STI
Then the kernel does not immediately triple-fault or hang
