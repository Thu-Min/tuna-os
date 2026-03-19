---
number: 15
title: F-015: RAM-based filesystem
labels: [status: unassigned]
url: https://github.com/Thu-Min/tuna-os/issues/15
---

## F-015: RAM-based filesystem

**Branch:** `feature/ram-filesystem`
**Depends on:** F-008

Simple in-memory filesystem (ramfs) supporting create, read, and list operations for loading user programs.

## Acceptance Criteria

### AC-1
Given heap is available, when ramfs initialized, then root directory exists and can be listed

### AC-2
Given ramfs initialized, when file created with name and content, then retrievable by name with matching content

### AC-3
Given files exist, when root directory listed, then all file names and sizes enumerated

### AC-4
Given nonexistent filename, when lookup attempted, then not-found indicator returned without crash
