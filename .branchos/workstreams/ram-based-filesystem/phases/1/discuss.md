# Phase 1 Discussion

## Goal

Implement a simple in-memory filesystem (ramfs) that supports creating files with content, retrieving files by name, and listing all files in the root directory. This provides the foundation for loading user programs from named files rather than hardcoded `.incbin` blobs.

## Requirements

1. **Initialization (AC-1):** `ramfs_init()` creates an empty root directory structure that can be listed (returns zero files). Uses `kmalloc` for all allocations.

2. **File creation and retrieval (AC-2):** `ramfs_create(name, data, size)` creates a file entry with a copy of the provided data. `ramfs_find(name)` looks up a file by name and returns a handle/pointer to it. The returned file must contain the exact original data and size.

3. **Directory listing (AC-3):** `ramfs_list()` or an iterator API enumerates all files in the root directory, reporting each file's name and size. Logged to serial for verification.

4. **Not-found handling (AC-4):** `ramfs_find(name)` returns NULL for nonexistent files without crashing. No assertions, no panics — just a null return.

## Assumptions

- Flat filesystem: single root directory, no subdirectories, no paths. File names are simple null-terminated strings (max ~63 chars).
- Files are immutable after creation — no write/append/truncate operations needed. Create once, read many.
- File data is copied into ramfs-owned memory via `kmalloc`. The caller's buffer can be freed after `ramfs_create`.
- No maximum file count limit enforced — bounded only by available heap memory.
- File entries stored as a singly-linked list. With expected file counts (<100), linear search is acceptable.
- No file deletion needed for this phase.
- Thread safety is not a concern — ramfs operations are called from kernel context before the scheduler starts (or with interrupts disabled).

## Unknowns

- **File handle design:** Should `ramfs_find` return a pointer to a `struct ramfs_file` (exposing internals) or an opaque handle with accessor functions (`ramfs_get_data`, `ramfs_get_size`)? Struct pointer is simpler and sufficient for a kernel-internal API.
- **Integration with ELF loader:** The ELF loader currently takes a raw `(void*, size)` pair. After ramfs, we can look up files by name and pass the result to `elf_load`. This integration is straightforward and can happen in `kernel_main`.

## Decisions

### Flat linked-list file storage

**Phase:** 1
**Context:** Need a data structure for file entries. Options range from arrays to hash tables to linked lists.
**Decision:** Use a singly-linked list of `struct ramfs_file` nodes, each containing name, data pointer, size, and next pointer. Simple, no fixed capacity, O(n) lookup is fine for <100 files.
**Alternatives considered:**
- Static array — fixed capacity, wasted space for small counts.
- Hash table — overkill complexity for this scale.

---

### Expose struct ramfs_file directly

**Phase:** 1
**Context:** Need a way for callers to access file data after lookup.
**Decision:** `ramfs_find` returns `const struct ramfs_file *` with public `name`, `data`, `size` fields. Simple, no accessor function overhead. Struct defined in `ramfs.h`.
**Alternatives considered:**
- Opaque handle with getter functions — unnecessary abstraction for kernel-internal use.

---
