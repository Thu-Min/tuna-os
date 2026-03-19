#include "gdt.h"
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

static void test_task_a(void) {
    for (;;) {
        serial_write("[A] ");
        for (volatile int i = 0; i < 500000; i++);
    }
}

static void test_task_b(void) {
    for (int n = 0; n < 20; n++) {
        serial_write("[B] ");
        for (volatile int i = 0; i < 500000; i++);
    }
    serial_write("[B done]\n");
    /* returns → task_trampoline calls task_exit() */
}

static void test_task_c(void) {
    for (;;) {
        serial_write("[C] ");
        for (volatile int i = 0; i < 500000; i++);
    }
}

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

    /* User-mode test — drops to ring 3, tests syscalls, then halts */
    serial_write("kernel: testing user mode + syscalls...\n");
    usermode_test();
    /* Should not reach here — GPF handler halts */

    serial_write("kernel: initializing pit...\n");
    pit_init(100);
    serial_write("kernel: pit ready\n");

    serial_write("kernel: initializing tasks...\n");
    task_init();
    task_create(test_task_a);
    task_create(test_task_b);
    task_create(test_task_c);
    serial_write("kernel: tasks ready\n");

    serial_write("kernel: enabling interrupts\n");
    __asm__ volatile ("sti");

    serial_write("kernel: idle loop\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
