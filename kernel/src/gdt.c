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

static struct gdt_descriptor gdt[3];

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
    gdt[1] = make_descriptor(0x9A, 0xA0); // Kernel code: present, ring0, executable, long mode.
    gdt[2] = make_descriptor(0x92, 0xC0); // Kernel data: present, ring0, writable.

    ptr.limit = (uint16_t)(sizeof(gdt) - 1);
    ptr.base = (uint64_t)gdt;
    gdt_flush(&ptr);
}
