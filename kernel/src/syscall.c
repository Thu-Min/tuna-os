#include "syscall.h"
#include "serial.h"

#include <stddef.h>
#include <stdint.h>

#define MAX_SYSCALLS 16

#define SYS_WRITE 1

/* Syscall handler signature: args from RDI, RSI, RDX, R10, R8, R9 */
typedef int64_t (*syscall_fn)(uint64_t, uint64_t, uint64_t,
                              uint64_t, uint64_t, uint64_t);

static syscall_fn syscall_table[MAX_SYSCALLS];

static void syscall_register(uint64_t number, syscall_fn handler) {
    if (number < MAX_SYSCALLS)
        syscall_table[number] = handler;
}

/*
 * sys_write(buf, len)
 *   RDI = buf pointer
 *   RSI = length
 * Returns bytes written.
 */
static int64_t sys_write(uint64_t buf, uint64_t len,
                         uint64_t arg3 __attribute__((unused)),
                         uint64_t arg4 __attribute__((unused)),
                         uint64_t arg5 __attribute__((unused)),
                         uint64_t arg6 __attribute__((unused))) {
    const char *str = (const char *)(uintptr_t)buf;

    /* Basic pointer validation: must be non-null and in user range */
    if (!str || buf + len < buf)
        return -1;

    for (uint64_t i = 0; i < len; i++)
        serial_write_char(str[i]);

    return (int64_t)len;
}

void syscall_init(void) {
    for (int i = 0; i < MAX_SYSCALLS; i++)
        syscall_table[i] = NULL;

    syscall_register(SYS_WRITE, sys_write);

    serial_write("[syscall] dispatch table initialized\n");
}

void syscall_dispatch(struct interrupt_frame *frame) {
    uint64_t number = frame->rax;

    if (number >= MAX_SYSCALLS || !syscall_table[number]) {
        serial_write("[syscall] invalid number=");
        serial_write_dec_u64(number);
        serial_write("\n");
        frame->rax = (uint64_t)(int64_t)-1;
        return;
    }

    int64_t result = syscall_table[number](
        frame->rdi, frame->rsi, frame->rdx,
        frame->r10, frame->r8,  frame->r9
    );

    frame->rax = (uint64_t)result;
}
