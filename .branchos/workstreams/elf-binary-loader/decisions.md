# Decisions

### Embed test binary via .incbin

**Phase:** 1
**Context:** Need to include a test ELF binary in the kernel image without a filesystem.
**Decision:** Use `.incbin` assembly directive in a dedicated `.S` file. Exports binary data and size as symbols. Standard approach for bare-metal blob embedding.
**Alternatives considered:**
- `xxd -i` — generates large C arrays, clutters build.
- GRUB module loading — adds Multiboot2 module parsing, overkill.

---

### User ELF base address at 0x400000

**Phase:** 1
**Context:** Need user ELF address range that doesn't overlap kernel (1 MiB), heap (2 MiB), or user stack.
**Decision:** Base at 0x400000 (4 MiB), following Linux x86_64 convention. User stack at 0x800000 (8 MiB).
**Alternatives considered:**
- Higher addresses — wastes space for no benefit.
- Lower addresses — overlaps kernel identity map.

---

### Minimal test program using inline asm syscalls

**Phase:** 1
**Context:** Test ELF needs syscalls but cannot use libc.
**Decision:** Minimal C with `_start`, inline asm for `int $0x80`, cross-compiled with `-ffreestanding -nostdlib` and custom linker script.
**Alternatives considered:**
- Pure assembly — harder to maintain.
- Syscall stub library — premature complexity.

---
