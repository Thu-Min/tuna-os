#include "usermode.h"
#include "gdt.h"
#include "pmm.h"
#include "serial.h"
#include "vmm.h"

#define USER_STACK_VADDR 0x800000  /* 8 MiB — well above kernel/heap */
#define USER_STACK_SIZE  VMM_PAGE_SIZE

/*
 * User-mode test function.  Written entirely in assembly to avoid
 * compiler-generated references to .rodata (which isn't USER-mapped).
 *
 * 1. Calls sys_write (syscall 1) to print "Hello from ring 3!\n" (AC-3)
 * 2. Calls invalid syscall 999 to test error return (AC-4)
 * 3. Calls sys_write to print "syscall test ok\n"
 * 4. Executes cli to trigger GPF and halt
 */
static void __attribute__((naked)) user_function(void) {
    __asm__ volatile (
        /* Build "Hello from ring 3!\n" on stack (20 bytes, 8-byte aligned = 24) */
        "subq $24, %%rsp\n\t"
        "movb $'H', 0(%%rsp)\n\t"
        "movb $'e', 1(%%rsp)\n\t"
        "movb $'l', 2(%%rsp)\n\t"
        "movb $'l', 3(%%rsp)\n\t"
        "movb $'o', 4(%%rsp)\n\t"
        "movb $' ', 5(%%rsp)\n\t"
        "movb $'f', 6(%%rsp)\n\t"
        "movb $'r', 7(%%rsp)\n\t"
        "movb $'o', 8(%%rsp)\n\t"
        "movb $'m', 9(%%rsp)\n\t"
        "movb $' ', 10(%%rsp)\n\t"
        "movb $'r', 11(%%rsp)\n\t"
        "movb $'i', 12(%%rsp)\n\t"
        "movb $'n', 13(%%rsp)\n\t"
        "movb $'g', 14(%%rsp)\n\t"
        "movb $' ', 15(%%rsp)\n\t"
        "movb $'3', 16(%%rsp)\n\t"
        "movb $'!', 17(%%rsp)\n\t"
        "movb $'\\n', 18(%%rsp)\n\t"

        /* sys_write(rsp, 19) */
        "movq $1, %%rax\n\t"       /* syscall 1 = sys_write */
        "movq %%rsp, %%rdi\n\t"    /* buf = stack string */
        "movq $19, %%rsi\n\t"      /* len = 19 */
        "int $0x80\n\t"

        "addq $24, %%rsp\n\t"

        /* Test invalid syscall (number 999) — should return -1 */
        "movq $999, %%rax\n\t"
        "int $0x80\n\t"
        /* rax now contains -1 (0xFFFFFFFFFFFFFFFF) */

        /* Build "syscall test ok\n" on stack (16 bytes) */
        "subq $16, %%rsp\n\t"
        "movb $'s', 0(%%rsp)\n\t"
        "movb $'y', 1(%%rsp)\n\t"
        "movb $'s', 2(%%rsp)\n\t"
        "movb $'c', 3(%%rsp)\n\t"
        "movb $'a', 4(%%rsp)\n\t"
        "movb $'l', 5(%%rsp)\n\t"
        "movb $'l', 6(%%rsp)\n\t"
        "movb $' ', 7(%%rsp)\n\t"
        "movb $'t', 8(%%rsp)\n\t"
        "movb $'e', 9(%%rsp)\n\t"
        "movb $'s', 10(%%rsp)\n\t"
        "movb $'t', 11(%%rsp)\n\t"
        "movb $' ', 12(%%rsp)\n\t"
        "movb $'o', 13(%%rsp)\n\t"
        "movb $'k', 14(%%rsp)\n\t"
        "movb $'\\n', 15(%%rsp)\n\t"

        /* sys_write(rsp, 16) */
        "movq $1, %%rax\n\t"
        "movq %%rsp, %%rdi\n\t"
        "movq $16, %%rsi\n\t"
        "int $0x80\n\t"

        "addq $16, %%rsp\n\t"

        /* Done — cli triggers GPF from ring 3, halting */
        "cli\n\t"
        "1: hlt\n\t"
        "jmp 1b\n\t"
        ::: "memory"
    );
}

/* End marker for page spanning check */
static void user_function_end(void) {}

void usermode_test(void) {
    serial_write("[usermode] setting up ring 3 test\n");

    /* Allocate and map a user stack page */
    uint64_t stack_frame = pmm_alloc_frame();
    if (!stack_frame) {
        serial_write("[usermode] failed to allocate stack frame\n");
        return;
    }
    vmm_map_page(USER_STACK_VADDR, stack_frame,
                 VMM_FLAG_PRESENT | VMM_FLAG_WRITE | VMM_FLAG_USER);

    /*
     * Re-map the page containing user_function with the USER flag so
     * CPL=3 can execute it.
     */
    uint64_t fn_addr = (uint64_t)(uintptr_t)user_function;
    uint64_t fn_page = fn_addr & ~(uint64_t)0xFFF;
    vmm_map_page(fn_page, fn_page,
                 VMM_FLAG_PRESENT | VMM_FLAG_WRITE | VMM_FLAG_USER);

    /* If the function spans a page boundary, map the next page too */
    uint64_t fn_end_addr = (uint64_t)(uintptr_t)user_function_end;
    uint64_t fn_end_page = fn_end_addr & ~(uint64_t)0xFFF;
    if (fn_end_page != fn_page) {
        vmm_map_page(fn_end_page, fn_end_page,
                     VMM_FLAG_PRESENT | VMM_FLAG_WRITE | VMM_FLAG_USER);
    }

    /* Flush TLB for the affected pages */
    __asm__ volatile ("invlpg (%0)" : : "r"(fn_page) : "memory");
    __asm__ volatile ("invlpg (%0)" : : "r"(USER_STACK_VADDR) : "memory");

    serial_write("[usermode] user fn=");
    serial_write_hex_u64(fn_addr);
    serial_write(" stack=");
    serial_write_hex_u64(USER_STACK_VADDR + USER_STACK_SIZE);
    serial_write("\n");

    serial_write("[usermode] jumping to ring 3 via iretq\n");

    uint64_t user_ss     = GDT_USER_DATA_RPL3;
    uint64_t user_rsp    = USER_STACK_VADDR + USER_STACK_SIZE;
    uint64_t user_rflags = 0x202;
    uint64_t user_cs     = GDT_USER_CODE_RPL3;
    uint64_t user_rip    = fn_addr;

    __asm__ volatile (
        "pushq %0\n\t"
        "pushq %1\n\t"
        "pushq %2\n\t"
        "pushq %3\n\t"
        "pushq %4\n\t"
        "iretq\n\t"
        :
        : "r"(user_ss), "r"(user_rsp), "r"(user_rflags),
          "r"(user_cs), "r"(user_rip)
        : "memory"
    );

    for (;;)
        __asm__ volatile ("hlt");
}
