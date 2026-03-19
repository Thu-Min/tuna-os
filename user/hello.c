/*
 * Minimal user-mode test program for the ELF loader.
 * Freestanding — no libc, no runtime. Uses int 0x80 for syscalls.
 */

#include <stdint.h>

#define SYS_WRITE 1

static int64_t sys_write(const char *buf, uint64_t len) {
    int64_t ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"((uint64_t)SYS_WRITE),
          "D"((uint64_t)(uintptr_t)buf),
          "S"(len)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static int64_t sys_invalid(void) {
    int64_t ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"((uint64_t)999)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void _start(void) {
    sys_write("Hello from ELF!\n", 16);

    /* Test invalid syscall — should return -1 */
    sys_invalid();

    sys_write("ELF test ok\n", 12);

    /* Halt — cli triggers GPF from ring 3 */
    __asm__ volatile ("cli");
    for (;;)
        __asm__ volatile ("hlt");
}
