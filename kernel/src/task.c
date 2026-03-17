#include "task.h"
#include "kheap.h"
#include "serial.h"

static uint64_t next_id;
static struct task *current_task;
static struct task *task_list;

void task_init(void) {
    /* Wrap the current kernel_main execution as task 0 (boot/idle task) */
    struct task *boot = kmalloc(sizeof(struct task));
    if (!boot) {
        serial_write("task: failed to allocate boot task\n");
        return;
    }

    boot->id    = next_id++;
    boot->rsp   = 0;  /* not used — boot task is already running */
    boot->state = TASK_RUNNING;
    boot->stack = 0;  /* boot task uses the boot stack from boot.S */
    boot->next  = boot; /* circular list: points to itself */

    current_task = boot;
    task_list    = boot;

    serial_write("task: boot task id=");
    serial_write_dec_u64(boot->id);
    serial_write("\n");
}

struct task *task_create(void (*entry)(void)) {
    struct task *t = kmalloc(sizeof(struct task));
    if (!t) {
        serial_write("task: failed to allocate TCB\n");
        return 0;
    }

    uint8_t *stack = kmalloc(TASK_STACK_SIZE);
    if (!stack) {
        serial_write("task: failed to allocate stack\n");
        kfree(t);
        return 0;
    }

    t->id    = next_id++;
    t->state = TASK_READY;
    t->stack = stack;

    /*
     * Set up the initial stack so that the context switch can
     * pop callee-saved registers and ret into the entry function.
     *
     * Stack layout (growing downward, top of stack at lowest address):
     *
     *   [high address]  stack + TASK_STACK_SIZE
     *                   ...
     *   rsp + 56:       task_exit   (return address for when entry() returns)
     *   rsp + 48:       entry       (ret target for context switch)
     *   rsp + 40:       0           (R15)
     *   rsp + 32:       0           (R14)
     *   rsp + 24:       0           (R13)
     *   rsp + 16:       0           (R12)
     *   rsp + 8:        0           (RBP)
     *   rsp + 0:        0           (RBX)
     *   [low address]
     *
     * The context switch does:
     *   pop rbx, pop rbp, pop r12, pop r13, pop r14, pop r15, ret
     * which pops 6 registers then rets into entry().
     * When entry() returns, it rets into task_exit().
     */
    uint64_t *sp = (uint64_t *)(stack + TASK_STACK_SIZE);

    /* Align to 16 bytes (sp is naturally aligned since TASK_STACK_SIZE is 8192) */

    *(--sp) = (uint64_t)(uintptr_t)task_exit;  /* return address for entry() */
    *(--sp) = (uint64_t)(uintptr_t)entry;       /* ret target for switch_to */
    *(--sp) = 0;  /* R15 */
    *(--sp) = 0;  /* R14 */
    *(--sp) = 0;  /* R13 */
    *(--sp) = 0;  /* R12 */
    *(--sp) = 0;  /* RBP */
    *(--sp) = 0;  /* RBX */

    t->rsp = (uint64_t)(uintptr_t)sp;

    /* Insert into circular task list */
    if (task_list) {
        struct task *tail = task_list;
        while (tail->next != task_list)
            tail = tail->next;
        tail->next = t;
        t->next = task_list;
    } else {
        t->next = t;
        task_list = t;
    }

    serial_write("task: created id=");
    serial_write_dec_u64(t->id);
    serial_write(" entry=");
    serial_write_hex_u64((uint64_t)(uintptr_t)entry);
    serial_write(" stack=");
    serial_write_hex_u64((uint64_t)(uintptr_t)stack);
    serial_write("\n");

    return t;
}

void task_destroy(struct task *t) {
    if (!t)
        return;

    /* Remove from circular task list */
    if (t->next == t) {
        /* Only task in the list */
        task_list = 0;
    } else {
        struct task *prev = task_list;
        while (prev->next != t)
            prev = prev->next;
        prev->next = t->next;
        if (task_list == t)
            task_list = t->next;
    }

    serial_write("task: destroyed id=");
    serial_write_dec_u64(t->id);
    serial_write("\n");

    if (t->stack)
        kfree(t->stack);
    kfree(t);
}

void task_exit(void) {
    serial_write("task: exit id=");
    serial_write_dec_u64(current_task->id);
    serial_write("\n");

    current_task->state = TASK_DEAD;

    /* Halt until the scheduler reaps us (F-011) */
    for (;;)
        __asm__ volatile ("hlt");
}

struct task *task_get_current(void) {
    return current_task;
}

struct task *task_get_list(void) {
    return task_list;
}
