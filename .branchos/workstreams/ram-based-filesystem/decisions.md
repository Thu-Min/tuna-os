# Decisions

### Flat linked-list file storage

**Phase:** 1
**Context:** Need a data structure for file entries.
**Decision:** Singly-linked list of `struct ramfs_file` nodes with name, data, size, next. O(n) lookup fine for <100 files.
**Alternatives considered:**
- Static array — fixed capacity, wasted space.
- Hash table — overkill for this scale.

---

### Expose struct ramfs_file directly

**Phase:** 1
**Context:** Need a way for callers to access file data after lookup.
**Decision:** `ramfs_find` returns `const struct ramfs_file *` with public fields. Simple kernel-internal API.
**Alternatives considered:**
- Opaque handle with getters — unnecessary abstraction.

---
