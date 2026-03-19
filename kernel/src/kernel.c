#include "elf.h"
#include "gdt.h"
#include "ramfs.h"
#include "idt.h"
#include "kheap.h"
#include "multiboot2.h"
#include "pic.h"
#include "pmm.h"
#include "pit.h"
#include "serial.h"
#include "task.h"
#include "syscall.h"
#include "tss.h"
#include "usermode.h"
#include "vmm.h"

#define BREAKPOINT_SELF_TEST 1

void kernel_main(uint64_t multiboot2_addr) {
    serial_init();
    serial_write("kernel: hello from tuna os!\n");

    serial_write("kernel: parsing multiboot2 memory map...\n");
    multiboot2_parse_mmap(multiboot2_addr);
    multiboot2_print_mmap();

    serial_write("kernel: initializing pmm...\n");
    uint32_t mmap_count;
    const struct mmap_region *mmap_regions = multiboot2_get_mmap(&mmap_count);
    pmm_init(mmap_regions, mmap_count);
    serial_write("kernel: pmm ready\n");

    serial_write("kernel: initializing vmm...\n");
    vmm_init();
    serial_write("kernel: vmm ready\n");

    serial_write("kernel: initializing kheap...\n");
    kheap_init();

    /* kmalloc/kfree self-test */
    serial_write("kernel: kmalloc self-test...\n");
    uint64_t *a = kmalloc(64);
    uint64_t *b = kmalloc(128);
    if (a && b) {
        *a = 0xDEADBEEF;
        *b = 0xCAFEBABE;
        serial_write("  alloc a=");
        serial_write_hex_u64((uint64_t)(uintptr_t)a);
        serial_write(" alloc b=");
        serial_write_hex_u64((uint64_t)(uintptr_t)b);
        serial_write("\n");
        kfree(a);
        uint64_t *c = kmalloc(32);
        serial_write("  freed a, alloc c=");
        serial_write_hex_u64((uint64_t)(uintptr_t)c);
        serial_write("\n");
        kfree(b);
        kfree(c);
        serial_write("  self-test passed\n");
    } else {
        serial_write("  self-test FAILED: kmalloc returned NULL\n");
    }

    serial_write("kernel: initializing gdt...\n");
    gdt_init();
    serial_write("kernel: gdt ready\n");

    serial_write("kernel: initializing tss...\n");
    /* Use a temporary kernel stack address for RSP0 — the boot stack. */
    extern char _kernel_end[];
    uint64_t kernel_stack_top = ((uint64_t)(uintptr_t)_kernel_end + 0x4000) & ~0xFULL;
    tss_init(kernel_stack_top);
    gdt_load_tss((uint64_t)(uintptr_t)tss_get(), sizeof(struct tss) - 1);
    serial_write("kernel: tss ready\n");

    serial_write("kernel: initializing idt...\n");
    idt_init();
    serial_write("kernel: idt ready\n");

#if BREAKPOINT_SELF_TEST
    serial_write("kernel: triggering int3 self-test\n");
    __asm__ volatile ("int3");
#endif

    serial_write("kernel: initializing pic...\n");
    pic_init();
    serial_write("kernel: pic ready\n");

    serial_write("kernel: initializing syscalls...\n");
    syscall_init();

    /* Initialize ramfs and populate with embedded binaries */
    serial_write("kernel: initializing ramfs...\n");
    ramfs_init();

    extern const char _binary_hello_elf_start[];
    extern const char _binary_hello_elf_end[];
    uint64_t hello_size = (uint64_t)(_binary_hello_elf_end - _binary_hello_elf_start);
    ramfs_create("hello.elf", _binary_hello_elf_start, hello_size);

    extern const char _binary_shell_elf_start[];
    extern const char _binary_shell_elf_end[];
    uint64_t shell_size = (uint64_t)(_binary_shell_elf_end - _binary_shell_elf_start);
    ramfs_create("shell.elf", _binary_shell_elf_start, shell_size);

    ramfs_list();

    /* Tell syscall module where the shell ELF is (for re-loading after exec) */
    const struct ramfs_file *shell_file = ramfs_find("shell.elf");
    if (shell_file)
        syscall_set_shell_elf(shell_file->data, shell_file->size);

    /* Boot into shell */
    serial_write("kernel: launching shell...\n");
    usermode_exec_elf(shell_file->data, shell_file->size);
    /* Should not reach here */

    for (;;)
        __asm__ volatile ("hlt");
}
