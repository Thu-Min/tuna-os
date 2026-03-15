.section .text
.code64

.global gdt_flush
gdt_flush:
    lgdt (%rdi)

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs

    pushq $0x08
    leaq .Lreload_cs(%rip), %rax
    pushq %rax
    lretq

.Lreload_cs:
    ret
