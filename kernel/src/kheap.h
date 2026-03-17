#pragma once
#include <stdint.h>

#define KHEAP_START        0x200000      /* 2 MiB */
#define KHEAP_INITIAL_SIZE (64 * 1024)   /* 64 KiB */
#define KHEAP_MAX_SIZE     (16 * 1024 * 1024) /* 16 MiB */
#define KHEAP_MIN_BLOCK    32

void  kheap_init(void);
void *kmalloc(uint64_t size);
void  kfree(void *ptr);
