# Phase 1 Discussion

## Goal

Implement an ELF64 binary loader that parses ELF headers and program headers, maps PT_LOAD segments into user-mode virtual address space with correct permissions, allocates a user stack, and jumps to the entry point in ring 3. Demonstrate end-to-end by loading and running an embedded test ELF binary that makes syscalls.

## Requirements

1. **ELF64 header parsing (AC-1):** Parse the ELF64 header to validate magic number, class (64-bit), data encoding (little-endian), type (ET_EXEC), machine (EM_X86_64), and extract the entry point (`e_entry`) and program header table offset/count. Iterate program headers and identify PT_LOAD segments with their virtual address (`p_vaddr`), file size (`p_filesz`), memory size (`p_memsz`), offset (`p_offset`), and flags (`p_flags`).

2. **Segment loading (AC-2):** For each PT_LOAD segment, allocate physical frames via PMM, map them into user address space via VMM with correct permissions (PF_R → PRESENT, PF_W → WRITE, PF_X → no NX, all with USER flag). Copy segment data from the in-memory ELF image to the mapped pages. Handle `p_memsz > p_filesz` (BSS) by zero-filling the remainder.

3. **User stack and entry (AC-3):** Allocate a user stack (e.g., 8 KiB at a fixed high address like 0x7FFFF000). Build an IRETQ frame with the ELF entry point as RIP, user stack as RSP, and user code/data selectors. Jump to ring 3. The loaded program must be able to call `sys_write` via `int $0x80`.

4. **Error handling (AC-4):** If the ELF header is invalid (bad magic, wrong class, wrong machine), report the error to serial and return without crashing the kernel.

## Assumptions

- The ELF binary will be embedded in the kernel image as a byte array (e.g., via `xxd -i` or `.incbin` in assembly). This avoids needing a filesystem. The test binary is a minimal statically-linked ELF64 that calls `sys_write` and exits.
- The test ELF binary will be cross-compiled separately with a custom linker script that places code at a user-space address (e.g., 0x400000). It will use inline assembly to invoke `int $0x80` directly (no libc).
- User virtual addresses for ELF segments will be in the range 0x400000–0x7FFFFF (well above kernel at 1 MiB and heap at 2 MiB). The VMM will map these pages on demand with USER flag.
- The kernel currently identity-maps the first 4 MiB. User ELF addresses above 4 MiB require new page table entries, which `vmm_map_page` handles transparently.
- The TSS RSP0 is already configured from F-012, so ring 3 → ring 0 transitions (syscalls, exceptions) will switch to the kernel stack correctly.
- NX (No-Execute) bit is not currently used (not enabled in page tables or EFER MSR), so all pages are executable. Permission mapping only needs PRESENT, WRITE, and USER flags.

## Unknowns

- **Embedding the test binary:** Need to decide between `.incbin` (assembler directive) and `xxd -i` (generates C array). `.incbin` is cleaner but requires a separate assembly file. `xxd -i` generates a large C file but integrates into the build more simply.
- **Test binary toolchain:** The test ELF must be a freestanding static ELF64 with no libc. It needs a custom linker script (base at 0x400000) and entry point that uses inline asm for syscalls. Cross-compilation with the existing `x86_64-elf-gcc` toolchain should work.
- **Page-aligned segment loading:** ELF segments may not start on page boundaries. The loader must handle page-aligned mapping: map from `p_vaddr & ~0xFFF` and copy data at the correct offset within the first page.
- **Multiple PT_LOAD segments:** A typical ELF has separate text (RX) and data (RW) segments. The loader must handle multiple segments. If they share a page (uncommon in properly linked binaries), the more permissive flags should apply.

## Decisions

### Embed test binary via .incbin

**Phase:** 1
**Context:** Need to include a test ELF binary in the kernel image without a filesystem.
**Decision:** Use a `.incbin` assembly directive in a dedicated `.S` file that exports the binary data and its size as symbols. This is the standard approach for embedding blobs in bare-metal kernels — clean, no generated files, and the linker handles alignment.
**Alternatives considered:**
- `xxd -i` — generates large C arrays, clutters the build, slower compile for large binaries.
- GRUB module loading — adds Multiboot2 module parsing complexity, overkill for a test binary.

---

### User ELF base address at 0x400000

**Phase:** 1
**Context:** Need to choose a virtual address range for user ELF binaries that doesn't overlap with kernel (1 MiB), heap (2 MiB), or user stack.
**Decision:** User ELF binaries link with base address 0x400000 (4 MiB). This follows the traditional Linux x86_64 convention and sits above the kernel identity map. User stack at 0x800000 (8 MiB), growing down.
**Alternatives considered:**
- Higher addresses (e.g., 0x10000000) — works but wastes address space mapping range for no benefit at this stage.
- Lower addresses (< 4 MiB) — would overlap with kernel identity map.

---

### Minimal test program using inline asm syscalls

**Phase:** 1
**Context:** The test ELF needs to make syscalls to prove AC-3 but cannot use libc.
**Decision:** Write a minimal C program with `_start` entry point that uses inline assembly for `int $0x80` syscalls. Cross-compile with `-ffreestanding -nostdlib` and a custom linker script. No libc, no runtime.
**Alternatives considered:**
- Pure assembly test program — harder to maintain, less readable.
- Linking against a minimal syscall stub library — premature, adds build complexity.

---
