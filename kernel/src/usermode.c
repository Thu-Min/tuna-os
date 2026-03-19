#include "usermode.h"
#include "elf.h"
#include "gdt.h"
#include "pmm.h"
#include "serial.h"
#include "vmm.h"

#define USER_STACK_VADDR 0x800000  /* 8 MiB */
#define USER_STACK_PAGES 2         /* 8 KiB stack */

void usermode_exec_elf(const void *elf_image, uint64_t elf_size) {
    serial_write("[usermode] loading ELF binary\n");

    uint64_t entry = elf_load(elf_image, elf_size);
    if (!entry) {
        serial_write("[usermode] ELF load failed\n");
        return;
    }

    /* Allocate user stack pages */
    for (uint32_t i = 0; i < USER_STACK_PAGES; i++) {
        uint64_t frame = pmm_alloc_frame();
        if (!frame) {
            serial_write("[usermode] failed to allocate stack frame\n");
            return;
        }
        uint64_t vaddr = USER_STACK_VADDR + (uint64_t)i * VMM_PAGE_SIZE;
        vmm_map_page(vaddr, frame,
                     VMM_FLAG_PRESENT | VMM_FLAG_WRITE | VMM_FLAG_USER);
    }

    uint64_t user_stack_top = USER_STACK_VADDR + (uint64_t)USER_STACK_PAGES * VMM_PAGE_SIZE;

    serial_write("[usermode] entry=");
    serial_write_hex_u64(entry);
    serial_write(" stack=");
    serial_write_hex_u64(user_stack_top);
    serial_write("\n");

    serial_write("[usermode] jumping to ring 3 via iretq\n");

    uint64_t user_ss     = GDT_USER_DATA_RPL3;
    uint64_t user_rsp    = user_stack_top;
    uint64_t user_rflags = 0x202;  /* IF set */
    uint64_t user_cs     = GDT_USER_CODE_RPL3;
    uint64_t user_rip    = entry;

    __asm__ volatile (
        "pushq %0\n\t"
        "pushq %1\n\t"
        "pushq %2\n\t"
        "pushq %3\n\t"
        "pushq %4\n\t"
        "iretq\n\t"
        :
        : "r"(user_ss), "r"(user_rsp), "r"(user_rflags),
          "r"(user_cs), "r"(user_rip)
        : "memory"
    );

    for (;;)
        __asm__ volatile ("hlt");
}
