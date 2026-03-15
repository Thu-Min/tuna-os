---
id: F-016
title: Minimal interactive shell
status: unassigned
milestone: M4
branch: feature/minimal-shell
issue: 16
workstream: null
---

Implement a minimal command-line shell as the first user-mode program, capable of reading input, parsing commands, and executing built-in operations like listing files and running programs from the ramfs.

## Acceptance Criteria

### AC-1
Given the shell is loaded as a user-mode ELF binary
When it starts
Then a prompt is displayed on the serial console
And the shell waits for keyboard input

### AC-2
Given the shell is waiting for input
When the user types a command and presses Enter
Then the input is parsed into a command name and arguments

### AC-3
Given the user types "ls"
When the command is executed
Then the files in the ramfs root directory are listed on serial output

### AC-4
Given the user types the name of an ELF binary in the ramfs
When the command is executed
Then the kernel loads and runs the binary as a new user-mode process
And the shell waits for it to complete before showing the next prompt
