#include "elf.h"
#include "pmm.h"
#include "serial.h"
#include "vmm.h"

static int elf_validate(const Elf64_Ehdr *ehdr, uint64_t size) {
    if (size < sizeof(Elf64_Ehdr)) {
        serial_write("[elf] image too small for ELF header\n");
        return 0;
    }

    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr->e_ident[1]       != ELFMAG1 ||
        ehdr->e_ident[2]       != ELFMAG2 ||
        ehdr->e_ident[EI_MAG3] != ELFMAG3) {
        serial_write("[elf] invalid magic\n");
        return 0;
    }

    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        serial_write("[elf] not ELF64\n");
        return 0;
    }

    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        serial_write("[elf] not little-endian\n");
        return 0;
    }

    if (ehdr->e_type != ET_EXEC) {
        serial_write("[elf] not ET_EXEC\n");
        return 0;
    }

    if (ehdr->e_machine != EM_X86_64) {
        serial_write("[elf] not x86_64\n");
        return 0;
    }

    return 1;
}

static void load_segment(const uint8_t *image, const Elf64_Phdr *phdr) {
    uint64_t vaddr_start = phdr->p_vaddr & ~(uint64_t)0xFFF;
    uint64_t vaddr_end   = (phdr->p_vaddr + phdr->p_memsz + 0xFFF) & ~(uint64_t)0xFFF;

    /* Determine page flags */
    uint32_t flags = VMM_FLAG_PRESENT | VMM_FLAG_USER;
    if (phdr->p_flags & PF_W)
        flags |= VMM_FLAG_WRITE;

    serial_write("[elf]   vaddr=");
    serial_write_hex_u64(phdr->p_vaddr);
    serial_write(" filesz=");
    serial_write_dec_u64(phdr->p_filesz);
    serial_write(" memsz=");
    serial_write_dec_u64(phdr->p_memsz);
    serial_write(" flags=");
    if (phdr->p_flags & PF_R) serial_write("R");
    if (phdr->p_flags & PF_W) serial_write("W");
    if (phdr->p_flags & PF_X) serial_write("X");
    serial_write("\n");

    /* Map pages and copy data */
    for (uint64_t page = vaddr_start; page < vaddr_end; page += VMM_PAGE_SIZE) {
        uint64_t frame = pmm_alloc_frame();
        if (!frame) {
            serial_write("[elf] out of frames\n");
            return;
        }

        /* Zero the frame first (for BSS and partial pages) */
        uint8_t *frame_ptr = (uint8_t *)(uintptr_t)frame;
        for (uint64_t i = 0; i < VMM_PAGE_SIZE; i++)
            frame_ptr[i] = 0;

        /* Copy file data that falls within this page */
        uint64_t seg_data_start = phdr->p_vaddr;
        uint64_t seg_data_end   = phdr->p_vaddr + phdr->p_filesz;

        uint64_t page_start = page;
        uint64_t page_end   = page + VMM_PAGE_SIZE;

        /* Overlap between [page_start, page_end) and [seg_data_start, seg_data_end) */
        uint64_t copy_start = (page_start > seg_data_start) ? page_start : seg_data_start;
        uint64_t copy_end   = (page_end < seg_data_end) ? page_end : seg_data_end;

        if (copy_start < copy_end) {
            uint64_t dst_offset = copy_start - page;
            uint64_t src_offset = phdr->p_offset + (copy_start - phdr->p_vaddr);
            uint64_t copy_len   = copy_end - copy_start;

            const uint8_t *src = image + src_offset;
            uint8_t *dst = frame_ptr + dst_offset;
            for (uint64_t i = 0; i < copy_len; i++)
                dst[i] = src[i];
        }

        vmm_map_page(page, frame, flags);
        __asm__ volatile ("invlpg (%0)" : : "r"(page) : "memory");
    }
}

uint64_t elf_load(const void *image, uint64_t size) {
    const Elf64_Ehdr *ehdr = (const Elf64_Ehdr *)image;

    if (!elf_validate(ehdr, size))
        return 0;

    serial_write("[elf] valid ELF64 x86_64 binary\n");
    serial_write("[elf] entry=");
    serial_write_hex_u64(ehdr->e_entry);
    serial_write(" phnum=");
    serial_write_dec_u64(ehdr->e_phnum);
    serial_write("\n");

    /* Validate program header table fits in image */
    uint64_t ph_end = ehdr->e_phoff + (uint64_t)ehdr->e_phnum * ehdr->e_phentsize;
    if (ph_end > size) {
        serial_write("[elf] program headers exceed image size\n");
        return 0;
    }

    const uint8_t *img = (const uint8_t *)image;
    uint32_t loaded = 0;

    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        const Elf64_Phdr *phdr = (const Elf64_Phdr *)(img + ehdr->e_phoff +
                                                       (uint64_t)i * ehdr->e_phentsize);

        if (phdr->p_type != PT_LOAD)
            continue;

        serial_write("[elf] PT_LOAD segment ");
        serial_write_dec_u64(i);
        serial_write(":\n");

        load_segment(img, phdr);
        loaded++;
    }

    serial_write("[elf] loaded ");
    serial_write_dec_u64(loaded);
    serial_write(" segments\n");

    return ehdr->e_entry;
}
