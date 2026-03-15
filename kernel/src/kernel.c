#include "gdt.h"
#include "idt.h"
#include "serial.h"

#define PHASE2_BREAKPOINT_SELF_TEST 1

void kernel_main(void) {
    serial_init();
    serial_write("kernel: hello from tuna os!\n");
    serial_write("phase2: initializing gdt...\n");
    gdt_init();
    serial_write("phase2: gdt ready\n");

    serial_write("phase2: initializing idt...\n");
    idt_init();
    serial_write("phase2: idt ready\n");

#if PHASE2_BREAKPOINT_SELF_TEST
    serial_write("phase2: triggering int3 self-test\n");
    __asm__ volatile ("int3");
#endif
    serial_write("phase2: idle loop\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
