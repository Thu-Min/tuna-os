#pragma once

#include "idt.h"

void syscall_init(void);
void syscall_dispatch(struct interrupt_frame *frame);
