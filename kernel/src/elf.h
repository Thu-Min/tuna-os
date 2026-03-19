#pragma once

#include <stdint.h>

/* ELF64 header */
typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) Elf64_Ehdr;

/* ELF64 program header */
typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} __attribute__((packed)) Elf64_Phdr;

/* e_ident indices */
#define EI_MAG0    0
#define EI_MAG3    3
#define EI_CLASS   4
#define EI_DATA    5

/* e_ident values */
#define ELFMAG0    0x7f
#define ELFMAG1    'E'
#define ELFMAG2    'L'
#define ELFMAG3    'F'
#define ELFCLASS64 2
#define ELFDATA2LSB 1

/* e_type */
#define ET_EXEC    2

/* e_machine */
#define EM_X86_64  62

/* p_type */
#define PT_LOAD    1

/* p_flags */
#define PF_X       0x1
#define PF_W       0x2
#define PF_R       0x4

/*
 * Load an ELF64 binary from memory into user address space.
 * Returns the entry point address on success, 0 on error.
 */
uint64_t elf_load(const void *image, uint64_t size);
