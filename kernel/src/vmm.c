#include "vmm.h"
#include "pmm.h"
#include "serial.h"

extern char _kernel_start[];
extern char _kernel_end[];

#define PML4_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define PD_INDEX(addr)   (((addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)   (((addr) >> 12) & 0x1FF)

#define PAGE_MASK (~(uint64_t)0xFFF)

static uint64_t *pml4;

static uint64_t *alloc_table(void) {
    uint64_t frame = pmm_alloc_frame();
    if (frame == 0)
        return 0;

    uint64_t *table = (uint64_t *)(uintptr_t)frame;
    for (int i = 0; i < 512; i++)
        table[i] = 0;

    return table;
}

void vmm_map_page(uint64_t virt, uint64_t phys, uint32_t flags) {
    uint64_t *cur_pml4 = pml4;

    /* PML4 → PDPT */
    uint32_t pml4_idx = PML4_INDEX(virt);
    uint64_t *pdpt;
    if (cur_pml4[pml4_idx] & VMM_FLAG_PRESENT) {
        pdpt = (uint64_t *)(uintptr_t)(cur_pml4[pml4_idx] & PAGE_MASK);
    } else {
        pdpt = alloc_table();
        if (!pdpt) return;
        cur_pml4[pml4_idx] = (uint64_t)(uintptr_t)pdpt | VMM_FLAG_PRESENT | VMM_FLAG_WRITE;
    }

    /* PDPT → PD */
    uint32_t pdpt_idx = PDPT_INDEX(virt);
    uint64_t *pd;
    if (pdpt[pdpt_idx] & VMM_FLAG_PRESENT) {
        pd = (uint64_t *)(uintptr_t)(pdpt[pdpt_idx] & PAGE_MASK);
    } else {
        pd = alloc_table();
        if (!pd) return;
        pdpt[pdpt_idx] = (uint64_t)(uintptr_t)pd | VMM_FLAG_PRESENT | VMM_FLAG_WRITE;
    }

    /* PD → PT */
    uint32_t pd_idx = PD_INDEX(virt);
    uint64_t *pt;
    if (pd[pd_idx] & VMM_FLAG_PRESENT) {
        pt = (uint64_t *)(uintptr_t)(pd[pd_idx] & PAGE_MASK);
    } else {
        pt = alloc_table();
        if (!pt) return;
        pd[pd_idx] = (uint64_t)(uintptr_t)pt | VMM_FLAG_PRESENT | VMM_FLAG_WRITE;
    }

    /* PT → page */
    uint32_t pt_idx = PT_INDEX(virt);
    pt[pt_idx] = (phys & PAGE_MASK) | flags;
}

void vmm_unmap_page(uint64_t virt) {
    uint64_t *cur_pml4 = pml4;

    uint32_t pml4_idx = PML4_INDEX(virt);
    if (!(cur_pml4[pml4_idx] & VMM_FLAG_PRESENT)) return;
    uint64_t *pdpt = (uint64_t *)(uintptr_t)(cur_pml4[pml4_idx] & PAGE_MASK);

    uint32_t pdpt_idx = PDPT_INDEX(virt);
    if (!(pdpt[pdpt_idx] & VMM_FLAG_PRESENT)) return;
    uint64_t *pd = (uint64_t *)(uintptr_t)(pdpt[pdpt_idx] & PAGE_MASK);

    uint32_t pd_idx = PD_INDEX(virt);
    if (!(pd[pd_idx] & VMM_FLAG_PRESENT)) return;
    uint64_t *pt = (uint64_t *)(uintptr_t)(pd[pd_idx] & PAGE_MASK);

    uint32_t pt_idx = PT_INDEX(virt);
    pt[pt_idx] = 0;

    __asm__ volatile ("invlpg (%0)" : : "r"(virt) : "memory");
}

void vmm_init(void) {
    pml4 = alloc_table();
    if (!pml4) {
        serial_write("vmm: failed to allocate PML4\n");
        return;
    }

    uint64_t k_end = (uint64_t)(uintptr_t)_kernel_end;
    /* Round up to next page boundary */
    uint64_t map_end = (k_end + VMM_PAGE_SIZE - 1) & PAGE_MASK;

    serial_write("vmm: identity-mapping 0x0 to ");
    serial_write_hex_u64(map_end);
    serial_write("\n");

    uint32_t pages_mapped = 0;
    for (uint64_t addr = 0; addr < map_end; addr += VMM_PAGE_SIZE) {
        vmm_map_page(addr, addr, VMM_FLAG_PRESENT | VMM_FLAG_WRITE);
        pages_mapped++;
    }

    serial_write("vmm: mapped ");
    serial_write_dec_u64(pages_mapped);
    serial_write(" pages\n");

    serial_write("vmm: loading CR3\n");
    uint64_t pml4_phys = (uint64_t)(uintptr_t)pml4;
    __asm__ volatile ("mov %0, %%cr3" : : "r"(pml4_phys) : "memory");

    serial_write("vmm: page tables active\n");
}
