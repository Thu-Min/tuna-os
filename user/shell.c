/*
 * Minimal interactive shell for tuna_os.
 * Freestanding — no libc, no runtime. Uses int 0x80 for syscalls.
 */

#include <stdint.h>

#define SYS_READ      0
#define SYS_WRITE     1
#define SYS_LISTFILES 2
#define SYS_EXEC      3
#define SYS_EXIT      4

static int64_t syscall0(uint64_t nr) {
    int64_t ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(nr) : "rcx", "r11", "memory");
    return ret;
}

static int64_t syscall2(uint64_t nr, uint64_t a1, uint64_t a2) {
    int64_t ret;
    __asm__ volatile ("int $0x80" : "=a"(ret)
                      : "a"(nr), "D"(a1), "S"(a2)
                      : "rcx", "r11", "memory");
    return ret;
}

static void write_str(const char *s) {
    uint64_t len = 0;
    while (s[len]) len++;
    syscall2(SYS_WRITE, (uint64_t)(uintptr_t)s, len);
}

static void write_char(char c) {
    syscall2(SYS_WRITE, (uint64_t)(uintptr_t)&c, 1);
}

static char read_char(void) {
    return (char)syscall0(SYS_READ);
}

static int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static uint64_t str_len(const char *s) {
    uint64_t n = 0;
    while (s[n]) n++;
    return n;
}

#define LINE_MAX 128

void _start(void) {
    char line[LINE_MAX];

    for (;;) {
        write_str("tuna> ");

        /* Read a line */
        uint64_t pos = 0;
        for (;;) {
            char c = read_char();

            if (c == '\r' || c == '\n') {
                write_str("\n");
                break;
            }

            if (c == 0x7f || c == '\b') {
                /* Backspace */
                if (pos > 0) {
                    pos--;
                    write_str("\b \b");
                }
                continue;
            }

            if (pos < LINE_MAX - 1) {
                line[pos++] = c;
                write_char(c);
            }
        }
        line[pos] = '\0';

        /* Skip empty input */
        if (pos == 0)
            continue;

        /* Parse and execute */
        if (str_eq(line, "ls")) {
            syscall0(SYS_LISTFILES);
        } else if (str_eq(line, "exit")) {
            syscall2(SYS_EXIT, 0, 0);
        } else {
            /* Try to exec as a program name */
            int64_t ret = syscall2(SYS_EXEC, (uint64_t)(uintptr_t)line, str_len(line));
            if (ret < 0) {
                write_str("unknown command: ");
                write_str(line);
                write_str("\n");
            }
        }
    }
}
