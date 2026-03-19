# Phase 1 Plan

## Objective

Implement a simple RAM-based filesystem (ramfs) with create, find, and list operations. Integrate it into kernel_main to store the embedded ELF binary as a named file and load it from ramfs — satisfying all four F-015 acceptance criteria.

## Tasks

### Task 1: Create ramfs module (ramfs.c, ramfs.h)

Implement the ramfs module with:

- `struct ramfs_file` — public struct with `name` (char[64]), `data` (void*), `size` (uint64_t), `next` pointer.
- `ramfs_init()` — initializes the file list head to NULL. Logs to serial.
- `ramfs_create(name, data, size)` — allocates a `ramfs_file` via `kmalloc`, copies `name` (truncated to 63 chars + null), allocates and copies `data`, appends to the linked list. Returns pointer to the new file or NULL on allocation failure.
- `ramfs_find(name)` — walks the linked list comparing names. Returns `const struct ramfs_file *` or NULL if not found (AC-4).
- `ramfs_list()` — walks the linked list and logs each file's name and size to serial (AC-3).

#### Affected Files

- `kernel/src/ramfs.c`
- `kernel/src/ramfs.h`

#### Dependencies

None (uses `kmalloc`/`kfree` from kheap and `serial_write` for logging).

#### Risks

- String comparison must be exact (no path normalization). Simple `strcmp`-equivalent loop.
- Name truncation at 63 chars must null-terminate correctly.

### Task 2: Integrate ramfs into kernel_main

Update `kernel_main` to:
1. Call `ramfs_init()` after kheap init.
2. Register the embedded hello.elf binary as a ramfs file: `ramfs_create("hello.elf", _binary_hello_elf_start, elf_size)`.
3. Call `ramfs_list()` to enumerate files (AC-1, AC-3).
4. Look up the file: `ramfs_find("hello.elf")` and pass its data/size to `usermode_exec_elf`.
5. Test not-found: `ramfs_find("nonexistent")` and verify NULL return (AC-4).

Update Makefile to compile `ramfs.c`.

#### Affected Files

- `kernel/src/kernel.c`
- `kernel/Makefile`

#### Dependencies

Task 1.

#### Risks

- None significant — straightforward wiring.

### Task 3: Build and validate in QEMU

Build and boot in QEMU. Verify serial output shows:
1. ramfs initialized, root directory listed with zero files, then with "hello.elf" after creation (AC-1, AC-3)
2. File created and retrieved with matching content (AC-2 — verified by ELF loader successfully parsing and running it)
3. All file names and sizes listed (AC-3)
4. Not-found lookup returns NULL without crash (AC-4)

#### Affected Files

- `kernel/Makefile`

#### Dependencies

Tasks 1–2.

#### Risks

- None — existing QEMU validation pattern.

## Affected Files

- `kernel/src/ramfs.c`
- `kernel/src/ramfs.h`
- `kernel/src/kernel.c`
- `kernel/Makefile`
