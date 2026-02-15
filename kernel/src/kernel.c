#include "serial.h"

void kernel_main(void) {
    serial_init();
    serial_write("kernel: hello from x86_64 long mode!\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}