#include "gdt.h"

#include <stdint.h>

struct gdt_descriptor {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

/*
 * 7 entries: null, kernel code, kernel data, user data, user code, TSS low, TSS high.
 * TSS descriptor is 16 bytes (2 slots) in 64-bit mode.
 */
static struct gdt_descriptor gdt[7];

extern void gdt_flush(const struct gdt_ptr *ptr);

static struct gdt_descriptor make_descriptor(uint8_t access, uint8_t flags) {
    struct gdt_descriptor desc;
    desc.limit_low = 0xFFFF;
    desc.base_low = 0;
    desc.base_mid = 0;
    desc.access = access;
    desc.granularity = (uint8_t)(0x0F | (flags & 0xF0));
    desc.base_high = 0;
    return desc;
}

void gdt_init(void) {
    struct gdt_ptr ptr;

    gdt[0] = (struct gdt_descriptor){0};
    gdt[1] = make_descriptor(0x9A, 0xA0); /* Kernel code: present, ring0, exec, long mode */
    gdt[2] = make_descriptor(0x92, 0xC0); /* Kernel data: present, ring0, writable */
    gdt[3] = make_descriptor(0xF2, 0xC0); /* User data: present, DPL=3, writable */
    gdt[4] = make_descriptor(0xFA, 0xA0); /* User code: present, DPL=3, exec, long mode */
    gdt[5] = (struct gdt_descriptor){0};   /* TSS low — filled by gdt_load_tss */
    gdt[6] = (struct gdt_descriptor){0};   /* TSS high — filled by gdt_load_tss */

    ptr.limit = (uint16_t)(sizeof(gdt) - 1);
    ptr.base = (uint64_t)gdt;
    gdt_flush(&ptr);
}

void gdt_load_tss(uint64_t tss_base, uint32_t tss_limit) {
    /*
     * 64-bit TSS descriptor (16 bytes / 2 GDT slots):
     *   Slot 5 (low):  standard system segment descriptor
     *   Slot 6 (high): upper 32 bits of base + reserved
     */
    gdt[5].limit_low   = (uint16_t)(tss_limit & 0xFFFF);
    gdt[5].base_low    = (uint16_t)(tss_base & 0xFFFF);
    gdt[5].base_mid    = (uint8_t)((tss_base >> 16) & 0xFF);
    gdt[5].access      = 0x89; /* present, type=9 (64-bit TSS available) */
    gdt[5].granularity = (uint8_t)((tss_limit >> 16) & 0x0F);
    gdt[5].base_high   = (uint8_t)((tss_base >> 24) & 0xFF);

    /* Upper half: base[63:32] in first 4 bytes, rest reserved/zero */
    uint32_t base_upper = (uint32_t)((tss_base >> 32) & 0xFFFFFFFF);
    uint8_t *slot6 = (uint8_t *)&gdt[6];
    slot6[0] = (uint8_t)(base_upper);
    slot6[1] = (uint8_t)(base_upper >> 8);
    slot6[2] = (uint8_t)(base_upper >> 16);
    slot6[3] = (uint8_t)(base_upper >> 24);
    slot6[4] = 0;
    slot6[5] = 0;
    slot6[6] = 0;
    slot6[7] = 0;

    /* Load TSS register */
    uint16_t tss_selector = GDT_TSS;
    __asm__ volatile ("ltr %w0" : : "r"(tss_selector));
}
