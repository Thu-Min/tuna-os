# Phase 1 Plan

## Objective

Implement an ELF64 binary loader that parses headers, maps PT_LOAD segments into user address space, allocates a user stack, and jumps to the entry point in ring 3. Demonstrate with an embedded test ELF that calls `sys_write` — satisfying all four F-014 acceptance criteria.

## Tasks

### Task 1: Create test user program and build infrastructure

Create a minimal freestanding ELF64 test program under `user/` that:
- Has a `_start` entry point (no libc)
- Calls `sys_write` via `int $0x80` to print "Hello from ELF!\n"
- Calls an invalid syscall to test error handling
- Calls `sys_write` again to print "ELF test ok\n"
- Halts via `cli` (triggers GPF → kernel halts)

Create `user/hello.c` with inline asm syscalls, `user/link.ld` (base at 0x400000), and a `user/Makefile` that produces `user/build/hello.elf`.

#### Affected Files

- `user/hello.c`
- `user/link.ld`
- `user/Makefile`

#### Dependencies

None.

#### Risks

- Must use `-ffreestanding -nostdlib -static -fno-pic -fno-pie` and the custom linker script. Any C runtime references (e.g., `__stack_chk_fail`) will cause link errors.

### Task 2: Embed test ELF in kernel via .incbin

Create `kernel/src/user_programs.S` with `.incbin "../../user/build/hello.elf"` that exports `_binary_hello_elf_start`, `_binary_hello_elf_end`, and `_binary_hello_elf_size` symbols. These are referenced by the ELF loader at runtime.

Update `kernel/Makefile` to:
1. Build the user program first (invoke `make -C ../user`).
2. Compile `user_programs.S` (which depends on the user binary).
3. Link the new object into the kernel.

#### Affected Files

- `kernel/src/user_programs.S`
- `kernel/Makefile`

#### Dependencies

Task 1 (user binary must exist before `.incbin`).

#### Risks

- Relative path in `.incbin` must match the build directory layout. The assembler resolves paths relative to the source file location.

### Task 3: Create ELF64 parser module (elf.c, elf.h)

Create `elf.h` with ELF64 header structures:
- `Elf64_Ehdr` — ELF header (magic, class, type, machine, entry, phoff, phnum, phentsize)
- `Elf64_Phdr` — Program header (type, flags, offset, vaddr, filesz, memsz)
- Constants: `ELFMAG`, `ELFCLASS64`, `ET_EXEC`, `EM_X86_64`, `PT_LOAD`, `PF_R/W/X`

Create `elf.c` with `elf_load(const void *image, uint64_t size)`:
1. Validate ELF magic (`\x7fELF`), class (64-bit), type (ET_EXEC), machine (x86_64).
2. Extract entry point from `e_entry`.
3. Iterate program headers, find PT_LOAD segments.
4. For each PT_LOAD: allocate frames, map pages at `p_vaddr` with USER + permission flags, copy `p_filesz` bytes from `image + p_offset`, zero-fill `p_memsz - p_filesz` (BSS).
5. Return entry point on success, 0 on error.

Handle page-aligned mapping: map from `p_vaddr & ~0xFFF`, compute offset within first page, copy data at correct position.

#### Affected Files

- `kernel/src/elf.c`
- `kernel/src/elf.h`

#### Dependencies

None (standalone module, uses PMM and VMM).

#### Risks

- Page boundary alignment: if `p_vaddr` isn't page-aligned, the first page needs partial copy at an offset.
- Segments spanning multiple pages require a loop allocating one frame per page.
- BSS zero-fill must cover from `p_filesz` to `p_memsz`, possibly spanning additional pages beyond what `p_filesz` requires.

### Task 4: Update usermode module to load and run ELF

Replace the hardcoded `user_function` in `usermode.c` with an `elf_exec()` function that:
1. Calls `elf_load()` with the embedded ELF binary to get the entry point.
2. Allocates a user stack (reuse existing 0x800000 mapping).
3. Builds IRETQ frame with the ELF entry point and jumps to ring 3.

The existing `usermode_test()` function becomes `elf_exec()` (or wraps it). The old naked assembly `user_function` is removed.

#### Affected Files

- `kernel/src/usermode.c`
- `kernel/src/usermode.h`

#### Dependencies

Tasks 2 and 3 (embedded binary and ELF loader).

#### Risks

- The ELF entry point address must be on a USER-mapped page (handled by elf_load).
- The user stack page must also be USER-mapped (already handled).

### Task 5: Wire up in kernel_main and update build

Update `kernel_main` to call `elf_exec()` (or the updated `usermode_test()`). Update `kernel/Makefile` SRCS_C and SRCS_S to include `elf.c` and `user_programs.S`. Add the `user/` build dependency.

#### Affected Files

- `kernel/src/kernel.c`
- `kernel/Makefile`

#### Dependencies

Tasks 1–4.

#### Risks

- Build ordering: user program must be built before kernel compilation (for `.incbin`).

### Task 6: Build and validate in QEMU

Build user program and kernel, boot in QEMU. Verify serial output shows:
1. ELF header parsed, PT_LOAD segments identified (AC-1)
2. Segments mapped at correct virtual addresses (AC-2)
3. "Hello from ELF!" printed via syscall from ring 3 (AC-3)
4. Invalid ELF test shows error without crash (AC-4)

For AC-4, add a test in `kernel_main` that calls `elf_load()` with a garbage buffer before the real load.

#### Affected Files

- `kernel/Makefile`

#### Dependencies

Tasks 1–5.

#### Risks

- Triple fault if ELF segments are loaded at wrong addresses or with wrong permissions.

## Affected Files

- `user/hello.c`
- `user/link.ld`
- `user/Makefile`
- `kernel/src/user_programs.S`
- `kernel/src/elf.c`
- `kernel/src/elf.h`
- `kernel/src/usermode.c`
- `kernel/src/usermode.h`
- `kernel/src/kernel.c`
- `kernel/Makefile`
