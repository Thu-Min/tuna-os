#include "multiboot2.h"
#include "serial.h"

#define ALIGN_UP(val, align) (((val) + (align) - 1) & ~((align) - 1))

static struct mmap_region regions[MMAP_MAX_REGIONS];
static uint32_t region_count;

void multiboot2_parse_mmap(uint64_t multiboot2_addr) {
    region_count = 0;

    uint8_t *base = (uint8_t *)(uintptr_t)multiboot2_addr;
    uint8_t *ptr = base + 8; /* skip fixed header */

    for (;;) {
        struct multiboot2_tag *tag = (struct multiboot2_tag *)ptr;

        if (tag->type == MULTIBOOT2_TAG_TYPE_END)
            break;

        if (tag->type == MULTIBOOT2_TAG_TYPE_MMAP) {
            struct multiboot2_tag_mmap *mmap = (struct multiboot2_tag_mmap *)ptr;
            uint8_t *entries = ptr + sizeof(struct multiboot2_tag_mmap);
            uint8_t *entries_end = ptr + mmap->size;

            while (entries < entries_end && region_count < MMAP_MAX_REGIONS) {
                struct multiboot2_mmap_entry *entry =
                    (struct multiboot2_mmap_entry *)entries;
                regions[region_count].base   = entry->base_addr;
                regions[region_count].length = entry->length;
                regions[region_count].type   = entry->type;
                region_count++;
                entries += mmap->entry_size;
            }
        }

        ptr += ALIGN_UP(tag->size, 8);
    }
}

const struct mmap_region *multiboot2_get_mmap(uint32_t *count) {
    *count = region_count;
    return regions;
}

static const char *mmap_type_string(uint32_t type) {
    switch (type) {
    case MULTIBOOT2_MMAP_TYPE_AVAILABLE:    return "available";
    case MULTIBOOT2_MMAP_TYPE_RESERVED:     return "reserved";
    case MULTIBOOT2_MMAP_TYPE_ACPI_RECLAIM: return "ACPI reclaimable";
    case MULTIBOOT2_MMAP_TYPE_ACPI_NVS:     return "ACPI NVS";
    case MULTIBOOT2_MMAP_TYPE_BAD_RAM:      return "bad memory";
    default:                                return "unknown";
    }
}

void multiboot2_print_mmap(void) {
    serial_write("mmap: detected ");
    serial_write_dec_u64(region_count);
    serial_write(" regions\n");

    for (uint32_t i = 0; i < region_count; i++) {
        serial_write("mmap: base=");
        serial_write_hex_u64(regions[i].base);
        serial_write(" length=");
        serial_write_hex_u64(regions[i].length);
        serial_write(" type=");
        serial_write(mmap_type_string(regions[i].type));
        serial_write("\n");
    }
}
