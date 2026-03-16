# Decisions — physical-memory-map

> Key decisions, trade-offs, and rationale captured during this workstream.

### Store memory map in a static array with fixed upper bound

**Phase:** 1
**Context:** The kernel has no dynamic allocation, and the memory map is needed before we can build an allocator.
**Decision:** Use a static array of memory map entries (e.g., `struct mmap_entry regions[32]`) with a count variable. 32 entries is more than sufficient for typical x86 QEMU setups.
**Alternatives considered:**
- Keeping only the Multiboot2 pointer and re-parsing on demand — fragile if the memory gets reused later
- Linked list — requires allocation we don't have yet

---

### Create a new `multiboot2.c/h` module

**Phase:** 1
**Context:** Need a clean place for Multiboot2 struct definitions and parsing logic.
**Decision:** Follow existing convention with a `multiboot2.c`/`multiboot2.h` pair in `kernel/src/`. This module will own the Multiboot2 tag definitions and memory map parsing. Export functions like `multiboot2_parse_mmap()` and `multiboot2_get_mmap()`.
**Alternatives considered:**
- Inlining everything in `kernel.c` — violates the module-per-subsystem convention
- Separate `mmap.c` for memory map — premature split; the parsing is tightly coupled to Multiboot2 tag format

---
