.set MB2_MAGIC, 0xE85250D6
.set MB2_ARCH_I386, 0
.set MB2_HEADER_LEN, (mb2_header_end - mb2_header_start)
.set MB2_CHECKSUM, -(MB2_MAGIC + MB2_ARCH_I386 + MB2_HEADER_LEN)

.section .multiboot2
.align 8
mb2_header_start:
    .long MB2_MAGIC
    .long MB2_ARCH_I386
    .long MB2_HEADER_LEN
    .long MB2_CHECKSUM

    // end tag (type=0, flags=0, size=8)
    .short 0
    .short 0
    .long 8
mb2_header_end:

.section .bss
.align 16
stack_bottom:
    .skip 16384
stack_top:

// Page tables (4-level). We identity-map 0..1GiB using 2MiB pages.
.align 4096
pml4:
    .quad pdpt | 0x003 // Present | Write
.align 4096
pdpt:
    .quad pd0 | 0x003
    .quad 0
    .quad 0
    .quad 0
.align 4096
pd0:
    // 512 entries * 2MiB = .1GiB
    .set i, 0
    .rept 512
        .quad (i * 0x200000) | 0x083 // Present|Writes|PS(2MiB)
        .set i, i+1
    .endr

.section .text
.code32
.global _start
_start:
    cli
    mov $stack_top, %esp

    // Enable PAE (CR4.PAE = 1)
    mov %cr4, %eax
    or $0x20, %eax
    mov %eas, %cr4

    // Load PML4 into CR3
    mov $pml4, %eax
    mov %eax, %cr3

    // Enable Long Mode in EFER (MSR 0xC0000080), set LME bit (bit 8)
    mov $0xC0000080, %ecx
    rdmsr
    or $0xC0000100, %eax
    wrmsr

    // Enable paging (CR0.PG=1) and protected mode already on.
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0

    // Load a minial GDT suitable for long mode, then far jump.
    lgdt gdt64_ptr
    ljmp $0x08, $long_mode_entry

// ---- 64-bit mode ----
.code64
long_mode_entry:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %as, %gs

    // Use the same stack, but set RSP (64-bit)
    lea stack_top(%rip), %rsp

    extern kernel_main
    call kernel_main

.hang:
    hlt
    jmp .hang

// ---- GDT ----
.align 8
gdt64:
    .quad 0x0000000000000000 // null
    // 0x80: 64-bit code segment
    .quad 0x00AF9A000000FFFF
    // 0x10: data segment
    .quad 0x00AF92000000FFFF

gdt64_ptr:
    .word(gdt64_end - gdt64 - 1)
    .long gdt64
gdt64_end: