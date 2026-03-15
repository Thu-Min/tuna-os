---
number: 4
title: F-004: PIT timer and tick counter
labels: [status: unassigned]
url: https://github.com/Thu-Min/tuna-os/issues/4
---

## F-004: PIT timer and tick counter

**Branch:** `feature/pit-timer-tick`
**Depends on:** F-003

Configure PIT channel 0 to fire periodic IRQ0 interrupts, maintain a global tick counter, and provide a basic time reference.

## Acceptance Criteria

### AC-1
Given IRQ0 is unmasked, when PIT is initialized at ~100 Hz, then IRQ0 fires at the configured rate

### AC-2
Given PIT is firing, when each timer interrupt is handled, then a global tick counter is incremented and EOI sent

### AC-3
Given the tick counter is running, when read, then it returns a monotonically increasing value

### AC-4
Given PIT and serial are working, when kernel boots, then periodic tick values are logged to serial
