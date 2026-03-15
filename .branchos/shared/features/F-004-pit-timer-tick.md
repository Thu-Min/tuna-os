---
id: F-004
title: PIT timer and tick counter
status: unassigned
milestone: M1
branch: feature/pit-timer-tick
issue: 4
workstream: null
---

Configure the Programmable Interval Timer (PIT) channel 0 to fire periodic IRQ0 interrupts, maintain a global tick counter, and provide a basic time reference for the kernel.

## Acceptance Criteria

### AC-1
Given the PIC has IRQ0 unmasked
When the PIT is initialized with a target frequency (e.g., 100 Hz)
Then IRQ0 fires at approximately the configured rate

### AC-2
Given the PIT is firing
When each timer interrupt is handled
Then a global tick counter is incremented
And EOI is sent to the PIC

### AC-3
Given the tick counter is running
When the kernel calls a tick-reading function
Then the current tick count is returned as a monotonically increasing value

### AC-4
Given the PIT and serial are working
When the kernel boots
Then periodic tick values are logged to serial to confirm timer operation
And the kernel enters an idle loop between interrupts
