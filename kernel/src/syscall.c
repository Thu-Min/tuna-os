#include "syscall.h"
#include "elf.h"
#include "gdt.h"
#include "io.h"
#include "pmm.h"
#include "ramfs.h"
#include "serial.h"
#include "vmm.h"

#include <stddef.h>
#include <stdint.h>

#define MAX_SYSCALLS 16

#define SYS_READ      0
#define SYS_WRITE     1
#define SYS_LISTFILES 2
#define SYS_EXEC      3
#define SYS_EXIT      4

/* Syscall handler signature: args from RDI, RSI, RDX, R10, R8, R9 */
typedef int64_t (*syscall_fn)(uint64_t, uint64_t, uint64_t,
                              uint64_t, uint64_t, uint64_t);

static syscall_fn syscall_table[MAX_SYSCALLS];

/* Saved caller frame for sys_exec/sys_exit return */
static struct interrupt_frame saved_frame;
static int child_running;

/* Shell ELF image pointer for re-loading after child overwrites pages */
static const void *shell_elf_image;
static uint64_t    shell_elf_size;

#define CHILD_STACK_VADDR 0x900000
#define CHILD_STACK_PAGES 2

static void syscall_register(uint64_t number, syscall_fn handler) {
    if (number < MAX_SYSCALLS)
        syscall_table[number] = handler;
}

/*
 * sys_read() — read one byte from serial (polling).
 * Returns the byte value (0–255).
 */
static int64_t sys_read(uint64_t arg1 __attribute__((unused)),
                        uint64_t arg2 __attribute__((unused)),
                        uint64_t arg3 __attribute__((unused)),
                        uint64_t arg4 __attribute__((unused)),
                        uint64_t arg5 __attribute__((unused)),
                        uint64_t arg6 __attribute__((unused))) {
    /* Poll until data ready (LSR bit 0) */
    while (!(inb(0x3F8 + 5) & 0x01))
        ;
    return (int64_t)(uint8_t)inb(0x3F8);
}

/*
 * sys_write(buf, len) — write buffer to serial.
 * Returns bytes written.
 */
static int64_t sys_write(uint64_t buf, uint64_t len,
                         uint64_t arg3 __attribute__((unused)),
                         uint64_t arg4 __attribute__((unused)),
                         uint64_t arg5 __attribute__((unused)),
                         uint64_t arg6 __attribute__((unused))) {
    const char *str = (const char *)(uintptr_t)buf;
    if (!str || buf + len < buf)
        return -1;
    for (uint64_t i = 0; i < len; i++)
        serial_write_char(str[i]);
    return (int64_t)len;
}

/*
 * sys_listfiles() — list ramfs files to serial.
 * Returns number of files.
 */
static int64_t sys_listfiles(uint64_t arg1 __attribute__((unused)),
                             uint64_t arg2 __attribute__((unused)),
                             uint64_t arg3 __attribute__((unused)),
                             uint64_t arg4 __attribute__((unused)),
                             uint64_t arg5 __attribute__((unused)),
                             uint64_t arg6 __attribute__((unused))) {
    ramfs_list();
    return 0;
}

void syscall_set_shell_elf(const void *image, uint64_t size) {
    shell_elf_image = image;
    shell_elf_size  = size;
}

void syscall_init(void) {
    for (int i = 0; i < MAX_SYSCALLS; i++)
        syscall_table[i] = NULL;

    syscall_register(SYS_READ, sys_read);
    syscall_register(SYS_WRITE, sys_write);
    syscall_register(SYS_LISTFILES, sys_listfiles);
    /* SYS_EXEC and SYS_EXIT handled directly in syscall_dispatch */

    child_running = 0;
    serial_write("[syscall] dispatch table initialized\n");
}

/*
 * Handle sys_exec: save caller frame, load child ELF, jump to it.
 */
static void handle_exec(struct interrupt_frame *frame) {
    const char *name = (const char *)(uintptr_t)frame->rdi;
    uint64_t name_len = frame->rsi;

    if (!name || name_len == 0 || name_len > 127) {
        frame->rax = (uint64_t)(int64_t)-1;
        return;
    }

    /* Copy filename to kernel buffer */
    char namebuf[128];
    for (uint64_t i = 0; i < name_len; i++)
        namebuf[i] = name[i];
    namebuf[name_len] = '\0';

    /* Look up in ramfs */
    const struct ramfs_file *f = ramfs_find(namebuf);
    if (!f) {
        serial_write("[exec] not found: ");
        serial_write(namebuf);
        serial_write("\n");
        frame->rax = (uint64_t)(int64_t)-1;
        return;
    }

    /* Load child ELF */
    uint64_t entry = elf_load(f->data, f->size);
    if (!entry) {
        serial_write("[exec] ELF load failed\n");
        frame->rax = (uint64_t)(int64_t)-1;
        return;
    }

    /* Allocate child stack */
    for (uint32_t i = 0; i < CHILD_STACK_PAGES; i++) {
        uint64_t pf = pmm_alloc_frame();
        if (!pf) {
            serial_write("[exec] stack alloc failed\n");
            frame->rax = (uint64_t)(int64_t)-1;
            return;
        }
        vmm_map_page(CHILD_STACK_VADDR + (uint64_t)i * VMM_PAGE_SIZE, pf,
                     VMM_FLAG_PRESENT | VMM_FLAG_WRITE | VMM_FLAG_USER);
    }

    /* Save caller (shell) frame */
    saved_frame = *frame;
    child_running = 1;

    serial_write("[exec] running: ");
    serial_write(namebuf);
    serial_write("\n");

    /* Modify frame to jump to child */
    frame->rip = entry;
    frame->rsp = CHILD_STACK_VADDR + (uint64_t)CHILD_STACK_PAGES * VMM_PAGE_SIZE;
    frame->cs  = GDT_USER_CODE_RPL3;
    frame->ss  = GDT_USER_DATA_RPL3;
    frame->rflags = 0x202;
    /* IRETQ will pop this modified frame → child starts */
}

/*
 * Handle sys_exit: restore caller frame, re-load shell ELF.
 */
static void handle_exit(struct interrupt_frame *frame) {
    if (!child_running) {
        /* Shell itself called exit — halt */
        serial_write("[exit] shell exited, halting\n");
        for (;;)
            __asm__ volatile ("cli; hlt");
    }

    serial_write("[exit] child finished\n");
    child_running = 0;

    /* Re-load shell ELF to restore code pages the child overwrote */
    if (shell_elf_image) {
        elf_load(shell_elf_image, shell_elf_size);
    }

    /* Restore caller (shell) frame */
    *frame = saved_frame;
    frame->rax = 0; /* sys_exec returns 0 = success */
}

void syscall_dispatch(struct interrupt_frame *frame) {
    uint64_t number = frame->rax;

    /* Handle exec/exit specially (they manipulate the frame directly) */
    if (number == SYS_EXEC) {
        handle_exec(frame);
        return;
    }
    if (number == SYS_EXIT) {
        handle_exit(frame);
        return;
    }

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
