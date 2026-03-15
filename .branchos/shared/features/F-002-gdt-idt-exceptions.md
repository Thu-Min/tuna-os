---
id: F-002
title: GDT and IDT with exception handling
status: complete
milestone: M1
branch: feature/gdt-idt-exceptions
issue: 2
workstream: null
---

Set up a proper kernel GDT replacing the bootstrap GDT, install an IDT with ISR stubs for all 32 CPU exception vectors, and dispatch exceptions through a C handler with serial logging.

## Acceptance Criteria

### AC-1
Given the kernel has booted into long mode
When gdt_init is called
Then a 3-entry GDT (null, kernel code, kernel data) is loaded and segment registers are reloaded

### AC-2
Given the GDT is loaded
When idt_init is called
Then all 32 CPU exception vectors (0-31) have ISR stubs installed in the IDT

### AC-3
Given the IDT is active
When an int3 breakpoint instruction executes
Then the exception dispatcher logs the vector number and RIP to serial
And execution resumes after the breakpoint

### AC-4
Given the IDT is active
When a fatal exception occurs (e.g., divide by zero)
Then the exception dispatcher logs the fault details to serial
And the CPU halts
