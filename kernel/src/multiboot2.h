#pragma once
#include <stdint.h>

#define MULTIBOOT2_TAG_TYPE_END   0
#define MULTIBOOT2_TAG_TYPE_MMAP  6

#define MULTIBOOT2_MMAP_TYPE_AVAILABLE     1
#define MULTIBOOT2_MMAP_TYPE_RESERVED      2
#define MULTIBOOT2_MMAP_TYPE_ACPI_RECLAIM  3
#define MULTIBOOT2_MMAP_TYPE_ACPI_NVS      4
#define MULTIBOOT2_MMAP_TYPE_BAD_RAM       5

#define MMAP_MAX_REGIONS 32

struct multiboot2_fixed_header {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed));

struct multiboot2_tag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct multiboot2_mmap_entry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

struct multiboot2_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
} __attribute__((packed));

struct mmap_region {
    uint64_t base;
    uint64_t length;
    uint32_t type;
};

void multiboot2_parse_mmap(uint64_t multiboot2_addr);
const struct mmap_region *multiboot2_get_mmap(uint32_t *count);
void multiboot2_print_mmap(void);
