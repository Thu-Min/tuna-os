#pragma once

#include <stdint.h>

#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_DATA   0x18
#define GDT_USER_CODE   0x20
#define GDT_TSS         0x28

/* Selectors with RPL=3 for user mode */
#define GDT_USER_DATA_RPL3 (GDT_USER_DATA | 3)  /* 0x1B */
#define GDT_USER_CODE_RPL3 (GDT_USER_CODE | 3)  /* 0x23 */

void gdt_init(void);
void gdt_load_tss(uint64_t tss_base, uint32_t tss_limit);
