#pragma once
#include <stdint.h>

#define VMM_PAGE_SIZE   4096

#define VMM_FLAG_PRESENT  0x001
#define VMM_FLAG_WRITE    0x002
#define VMM_FLAG_USER     0x004

void vmm_init(void);
void vmm_map_page(uint64_t virt, uint64_t phys, uint32_t flags);
void vmm_unmap_page(uint64_t virt);
