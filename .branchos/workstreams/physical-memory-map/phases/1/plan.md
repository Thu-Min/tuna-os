# Phase 1 Plan

## Objective

Preserve the Multiboot2 info pointer from GRUB, parse the memory map tag, store detected physical memory regions in a static array, and log them to serial. Satisfies all three acceptance criteria for F-005.

## Tasks

### Task 1: Preserve Multiboot2 info pointer in boot.S

Pass the Multiboot2 info pointer (in `ebx` at entry) to `kernel_main` as its first argument.

**Analysis:** `ebx` is not clobbered during the 32→64-bit transition — only `eax`, `ecx`, and control registers are modified. In 64-bit mode, `ebx`/`rbx` is callee-saved and survives to the `call kernel_main` site. We need to zero-extend `ebx` into `rdi` (System V AMD64 first argument register) just before the call.

**Changes:**
- Before `call kernel_main`, add: `mov %ebx, %edi` (writing to `edi` zero-extends to `rdi` in 64-bit mode)

#### Affected Files

- `kernel/src/boot.S`

#### Dependencies

None — this is the first task.

#### Risks

- If a future boot.S change clobbers `ebx`, the pointer will be lost. The register usage is well-documented by the added comment.

---

### Task 2: Create multiboot2.h with struct definitions

Define packed C structs for Multiboot2 structures:

- `struct multiboot2_fixed_header` — `total_size` (u32), `reserved` (u32)
- `struct multiboot2_tag` — `type` (u32), `size` (u32) (generic tag header)
- `struct multiboot2_mmap_entry` — `base_addr` (u64), `length` (u64), `type` (u32), `reserved` (u32)
- `struct multiboot2_tag_mmap` — inherits tag header fields, adds `entry_size` (u32), `entry_version` (u32), followed by entries

Constants:
- `MULTIBOOT2_TAG_TYPE_END 0`
- `MULTIBOOT2_TAG_TYPE_MMAP 6`
- `MULTIBOOT2_MMAP_TYPE_AVAILABLE 1` through `MULTIBOOT2_MMAP_TYPE_BAD_RAM 5`

Parsed result types:
- `struct mmap_region` — `base` (u64), `length` (u64), `type` (u32)
- `MMAP_MAX_REGIONS 32`

Public API:
- `void multiboot2_parse_mmap(uint64_t multiboot2_addr)` — parse and store regions
- `const struct mmap_region *multiboot2_get_mmap(uint32_t *count)` — retrieve parsed regions
- `void multiboot2_print_mmap(void)` — log regions to serial

#### Affected Files

- `kernel/src/multiboot2.h` (new)

#### Dependencies

None.

#### Risks

None — header-only definitions.

---

### Task 3: Implement multiboot2.c with parsing and logging

Implement the three public functions:

**`multiboot2_parse_mmap(uint64_t multiboot2_addr)`:**
1. Cast `multiboot2_addr` to `struct multiboot2_fixed_header *`
2. Start iteration at first tag: `addr + 8` (skip fixed header)
3. Loop: read `struct multiboot2_tag *`, check `type`
   - If type == 6 (MMAP): cast to `struct multiboot2_tag_mmap *`, iterate entries by `entry_size` stride, copy each into static `regions[]` array up to `MMAP_MAX_REGIONS`
   - If type == 0 (END): break
   - Advance to next tag: `current + ALIGN_UP(tag->size, 8)`
4. Store region count

**`multiboot2_get_mmap(uint32_t *count)`:**
- Set `*count` to stored region count, return pointer to static `regions[]`

**`multiboot2_print_mmap(void)`:**
- For each region, print: `"mmap: base=0x<hex> length=0x<hex> type=<string>"`
- Type strings: "available", "reserved", "ACPI reclaimable", "ACPI NVS", "bad memory", "unknown"

#### Affected Files

- `kernel/src/multiboot2.c` (new)

#### Dependencies

- Task 2 (multiboot2.h for struct definitions)

#### Risks

- Off-by-one in tag alignment. Multiboot2 spec requires 8-byte alignment for tags — the alignment macro must round up `(size + 7) & ~7`.

---

### Task 4: Update kernel_main to accept and use Multiboot2 info

Modify `kernel_main` to:
1. Change signature from `void kernel_main(void)` to `void kernel_main(uint64_t multiboot2_addr)`
2. After `serial_init()`, call `multiboot2_parse_mmap(multiboot2_addr)`
3. Call `multiboot2_print_mmap()` to log detected regions
4. Add `#include "multiboot2.h"`

#### Affected Files

- `kernel/src/kernel.c`

#### Dependencies

- Task 1 (boot.S passes pointer)
- Task 3 (multiboot2.c implementation)

#### Risks

None — additive change; all existing init calls remain.

---

### Task 5: Add multiboot2.c to Makefile

Add `src/multiboot2.c` to `SRCS_C` in the kernel Makefile so it gets compiled and linked.

#### Affected Files

- `kernel/Makefile`

#### Dependencies

- Task 3 (source file must exist)

#### Risks

None.

---

### Task 6: Build and boot verification

Run `make clean && make iso && make run-bios` from `kernel/` to verify:
1. Kernel compiles without warnings/errors (`-Werror` enforced)
2. Kernel boots in QEMU and prints memory map regions to serial
3. All existing subsystems (GDT, IDT, PIC, PIT) still initialize correctly
4. Memory regions show expected types (available, reserved) with valid addresses

#### Affected Files

(no file changes — verification only)

#### Dependencies

- All previous tasks

#### Risks

- QEMU BIOS vs EFI may report different memory layouts. The run-bios target uses legacy BIOS which is more predictable for Multiboot2.

## Affected Files

- `kernel/src/boot.S`
- `kernel/src/multiboot2.h` (new)
- `kernel/src/multiboot2.c` (new)
- `kernel/src/kernel.c`
- `kernel/Makefile`
