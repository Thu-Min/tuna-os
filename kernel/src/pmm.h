#pragma once
#include <stdint.h>

#define PMM_PAGE_SIZE  4096
#define PMM_MAX_FRAMES 32768  /* 128 MiB */

struct mmap_region;

void     pmm_init(const struct mmap_region *regions, uint32_t count);
uint64_t pmm_alloc_frame(void);
void     pmm_free_frame(uint64_t addr);
uint32_t pmm_get_free_count(void);
uint32_t pmm_get_total_count(void);
