#pragma once

#include <stdint.h>

void usermode_exec_elf(const void *elf_image, uint64_t elf_size);
