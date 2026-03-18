/*
 * switch_to(uint64_t *old_rsp, uint64_t new_rsp)
 *
 * Save callee-saved registers on the current stack, store RSP into *old_rsp,
 * load new_rsp, restore callee-saved registers, and ret into the new task.
 *
 * Register layout must match the initial stack built by task_create():
 *   [top]  RBX  RBP  R12  R13  R14  R15  <ret addr>
 */
.section .text
.code64

.global switch_to
switch_to:
    /* Save callee-saved registers on old task's stack */
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    /* Save old RSP: *old_rsp = rsp */
    movq %rsp, (%rdi)

    /* Load new RSP */
    movq %rsi, %rsp

    /* Restore callee-saved registers from new task's stack */
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx

    /* Return into the new task */
    ret
