---
id: F-015
title: RAM-based filesystem
status: unassigned
milestone: M4
branch: feature/ram-filesystem
issue: 15
workstream: null
---

Implement a simple in-memory filesystem (ramfs) that supports creating, reading, and listing files, providing the minimal file abstraction needed for loading user programs.

## Acceptance Criteria

### AC-1
Given the heap allocator is available
When the ramfs is initialized
Then a root directory exists and can be listed

### AC-2
Given the ramfs is initialized
When a file is created with a name and content
Then the file can be retrieved by name
And its content matches what was stored

### AC-3
Given files exist in the ramfs
When the root directory is listed
Then all file names and sizes are enumerated

### AC-4
Given a file name that does not exist
When a lookup is attempted
Then a not-found indicator is returned without crashing
