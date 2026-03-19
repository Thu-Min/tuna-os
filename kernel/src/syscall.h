#pragma once

#include "idt.h"

void syscall_init(void);
void syscall_dispatch(struct interrupt_frame *frame);
void syscall_set_shell_elf(const void *image, uint64_t size);
