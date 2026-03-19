---
number: 16
title: F-016: Minimal interactive shell
labels: [status: unassigned]
url: https://github.com/Thu-Min/tuna-os/issues/16
---

## F-016: Minimal interactive shell

**Branch:** `feature/minimal-shell`
**Depends on:** F-014, F-015

Minimal command-line shell as the first user-mode program — reads input, parses commands, lists files, and runs programs from ramfs.

## Acceptance Criteria

### AC-1
Given shell loaded as user-mode ELF, when started, then prompt displayed on serial and shell waits for input

### AC-2
Given shell waiting, when user types command + Enter, then input parsed into command name and arguments

### AC-3
Given user types "ls", when executed, then ramfs root directory files listed on serial

### AC-4
Given user types name of ELF binary in ramfs, when executed, then kernel loads and runs it as new process and shell waits for completion
