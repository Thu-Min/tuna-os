---
generated: 2026-03-15T00:00:00Z
commit: 39072ceafaf7eddc9324babf2194289c4e868893
generator: branchos/map-codebase
---

# Stack

This is a bare-metal OS kernel with no package manager dependencies. All tooling is system-level.

## Build Toolchain

- **x86_64-elf-gcc**: Cross-compiler targeting x86_64 ELF — compiles freestanding C and assembly
- **x86_64-elf-binutils** (includes `x86_64-elf-ld`): Cross-linker and binary utilities
- **GNU Make**: Build orchestration via Makefiles

## ISO Generation

- **grub-mkrescue** (`x86_64-elf-grub`): Creates bootable GRUB ISO from kernel ELF
- **xorriso**: ISO 9660 filesystem tool, required by grub-mkrescue
- **mtools**: FAT filesystem utilities, required by grub-mkrescue for EFI boot images

## Runtime / Emulation

- **QEMU** (`qemu-system-x86_64`): x86_64 system emulator — provides BIOS and EFI boot paths, serial console, GDB stub for debugging
- **EDK2 firmware** (bundled with QEMU): UEFI firmware images for EFI boot path testing

## Debug

- **x86_64-elf-gdb**: Cross-architecture GDB for kernel debugging via QEMU's GDB stub

## Language Standards

- **C11**: Kernel C code compiled with `-std=c11`
- **GNU as** (AT&T syntax): Assembly for boot, GDT flush, and ISR stubs
