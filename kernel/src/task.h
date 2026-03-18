#pragma once
#include <stdint.h>

#define TASK_STACK_SIZE 8192  /* 8 KiB */

#define TASK_READY   0
#define TASK_RUNNING 1
#define TASK_DEAD    2

struct task {
    uint64_t     id;
    uint64_t     rsp;
    uint32_t     state;
    uint32_t     _pad;
    uint8_t     *stack;   /* base of allocated stack (for kfree) */
    struct task *next;
};

void         task_init(void);
struct task  *task_create(void (*entry)(void));
void         task_destroy(struct task *t);
void         task_exit(void);
struct task  *task_get_current(void);
struct task  *task_get_list(void);
