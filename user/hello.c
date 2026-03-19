/*
 * Minimal user-mode test program for the ELF loader.
 * Freestanding — no libc, no runtime. Uses int 0x80 for syscalls.
 */

#include <stdint.h>

#define SYS_READ  0
#define SYS_WRITE 1
#define SYS_EXIT  4

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

static void sys_exit(int64_t code) {
    __asm__ volatile (
        "int $0x80"
        :
        : "a"((uint64_t)SYS_EXIT),
          "D"((uint64_t)code)
        : "rcx", "r11", "memory"
    );
}

void _start(void) {
    sys_write("Hello from ELF!\n", 16);
    sys_exit(0);
}
