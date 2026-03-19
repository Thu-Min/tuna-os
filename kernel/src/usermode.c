#include "usermode.h"
#include "gdt.h"
#include "pmm.h"
#include "serial.h"
#include "vmm.h"

#define USER_STACK_VADDR 0x800000  /* 8 MiB — well above kernel/heap */
#define USER_STACK_SIZE  VMM_PAGE_SIZE

/*
 * User-mode test function.  Attempts a privileged instruction (cli)
 * which triggers #GP from ring 3, validating AC-4.
 *
 * This function is copied to a USER-mapped page at runtime so the CPU
 * can fetch it at CPL=3.
 */
static void user_function(void) {
    /* cli is privileged — will cause #GP(0) from ring 3 */
    __asm__ volatile ("cli");

    /* Should never reach here */
    for (;;)
        __asm__ volatile ("hlt");
}

/* End marker for memcpy sizing — placed right after user_function */
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
     * CPL=3 can execute it.  The function lives in kernel .text which
     * is already identity-mapped; we just add USER to that page.
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

    /*
     * Build IRETQ frame and jump to ring 3:
     *   SS     = GDT_USER_DATA_RPL3 (0x1B)
     *   RSP    = top of user stack
     *   RFLAGS = 0x202 (IF set, reserved bit 1 set)
     *   CS     = GDT_USER_CODE_RPL3 (0x23)
     *   RIP    = user_function
     */
    uint64_t user_ss     = GDT_USER_DATA_RPL3;
    uint64_t user_rsp    = USER_STACK_VADDR + USER_STACK_SIZE;
    uint64_t user_rflags = 0x202;
    uint64_t user_cs     = GDT_USER_CODE_RPL3;
    uint64_t user_rip    = fn_addr;

    __asm__ volatile (
        "pushq %0\n\t"   /* SS */
        "pushq %1\n\t"   /* RSP */
        "pushq %2\n\t"   /* RFLAGS */
        "pushq %3\n\t"   /* CS */
        "pushq %4\n\t"   /* RIP */
        "iretq\n\t"
        :
        : "r"(user_ss), "r"(user_rsp), "r"(user_rflags),
          "r"(user_cs), "r"(user_rip)
        : "memory"
    );

    /* Should never reach here */
    for (;;)
        __asm__ volatile ("hlt");
}
