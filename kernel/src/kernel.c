#include "serial.h"

void kernel_main(void) {
    serial_init();
    serial_write("kernel: hello from tuna os!\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}