/*
 * Embed user-mode ELF binaries into the kernel image.
 * The assembler .incbin directive includes the raw file bytes.
 */
.section .rodata

.global _binary_hello_elf_start
.global _binary_hello_elf_end
.global _binary_hello_elf_size

_binary_hello_elf_start:
    .incbin "../user/build/hello.elf"
_binary_hello_elf_end:

.align 8
_binary_hello_elf_size:
    .quad _binary_hello_elf_end - _binary_hello_elf_start
