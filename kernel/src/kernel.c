#include "gdt.h"
#include "idt.h"
#include "multiboot2.h"
#include "pic.h"
#include "pit.h"
#include "serial.h"

#define BREAKPOINT_SELF_TEST 1

void kernel_main(uint64_t multiboot2_addr) {
    serial_init();
    serial_write("kernel: hello from tuna os!\n");

    serial_write("kernel: parsing multiboot2 memory map...\n");
    multiboot2_parse_mmap(multiboot2_addr);
    multiboot2_print_mmap();

    serial_write("kernel: initializing gdt...\n");
    gdt_init();
    serial_write("kernel: gdt ready\n");

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

    serial_write("kernel: initializing pit...\n");
    pit_init(100);
    serial_write("kernel: pit ready\n");

    serial_write("kernel: enabling interrupts\n");
    __asm__ volatile ("sti");

    serial_write("kernel: idle loop\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
