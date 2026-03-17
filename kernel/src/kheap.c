#include "kheap.h"
#include "pmm.h"
#include "vmm.h"
#include "serial.h"

#define HEADER_SIZE 16
#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))

struct block_header {
    uint64_t size;     /* total block size including header */
    uint32_t free;
    uint32_t _pad;
};

static uint64_t heap_start;
static uint64_t heap_end;
static struct block_header *free_list;

static void kheap_map_region(uint64_t start, uint64_t size) {
    for (uint64_t off = 0; off < size; off += PMM_PAGE_SIZE) {
        uint64_t phys = pmm_alloc_frame();
        if (phys == 0) {
            serial_write("kheap: out of physical frames\n");
            return;
        }
        vmm_map_page(start + off, phys, VMM_FLAG_PRESENT | VMM_FLAG_WRITE);
    }
}

void kheap_init(void) {
    heap_start = KHEAP_START;
    heap_end = heap_start + KHEAP_INITIAL_SIZE;

    serial_write("kheap: mapping heap at ");
    serial_write_hex_u64(heap_start);
    serial_write(" size=");
    serial_write_dec_u64(KHEAP_INITIAL_SIZE);
    serial_write("\n");

    kheap_map_region(heap_start, KHEAP_INITIAL_SIZE);

    /* Initialize with one large free block */
    free_list = (struct block_header *)(uintptr_t)heap_start;
    free_list->size = KHEAP_INITIAL_SIZE;
    free_list->free = 1;
    free_list->_pad = 0;

    serial_write("kheap: ready\n");
}

static int kheap_expand(uint64_t min_size) {
    uint64_t expand = ALIGN_UP(min_size, PMM_PAGE_SIZE);
    if (expand < PMM_PAGE_SIZE)
        expand = PMM_PAGE_SIZE;

    if (heap_end + expand > KHEAP_START + KHEAP_MAX_SIZE) {
        serial_write("kheap: max size reached\n");
        return 0;
    }

    serial_write("kheap: expanding by ");
    serial_write_dec_u64(expand);
    serial_write(" bytes\n");

    kheap_map_region(heap_end, expand);

    /* Create a new free block at the expansion point */
    struct block_header *new_block = (struct block_header *)(uintptr_t)heap_end;
    new_block->size = expand;
    new_block->free = 1;
    new_block->_pad = 0;

    /* Try to coalesce with the last block if it's free */
    struct block_header *cur = (struct block_header *)(uintptr_t)heap_start;
    struct block_header *last = 0;
    while ((uint64_t)(uintptr_t)cur < heap_end) {
        last = cur;
        cur = (struct block_header *)((uint8_t *)cur + cur->size);
    }

    heap_end += expand;

    if (last && last->free) {
        last->size += new_block->size;
    }

    return 1;
}

void *kmalloc(uint64_t size) {
    if (size == 0)
        return 0;

    /* Align to 8 bytes, ensure minimum usable block */
    uint64_t total = ALIGN_UP(size + HEADER_SIZE, 8);
    if (total < KHEAP_MIN_BLOCK)
        total = KHEAP_MIN_BLOCK;

retry:;
    struct block_header *cur = (struct block_header *)(uintptr_t)heap_start;

    while ((uint64_t)(uintptr_t)cur < heap_end) {
        if (cur->free && cur->size >= total) {
            /* Split if remainder is large enough */
            if (cur->size >= total + KHEAP_MIN_BLOCK) {
                struct block_header *split =
                    (struct block_header *)((uint8_t *)cur + total);
                split->size = cur->size - total;
                split->free = 1;
                split->_pad = 0;
                cur->size = total;
            }

            cur->free = 0;
            return (void *)((uint8_t *)cur + HEADER_SIZE);
        }

        cur = (struct block_header *)((uint8_t *)cur + cur->size);
    }

    /* No block found — expand heap and retry */
    if (kheap_expand(total)) {
        goto retry;
    }

    return 0;
}

void kfree(void *ptr) {
    if (!ptr)
        return;

    struct block_header *block =
        (struct block_header *)((uint8_t *)ptr - HEADER_SIZE);
    block->free = 1;

    /* Coalesce adjacent free blocks (linear walk) */
    struct block_header *cur = (struct block_header *)(uintptr_t)heap_start;
    while ((uint64_t)(uintptr_t)cur < heap_end) {
        if (cur->free) {
            struct block_header *next =
                (struct block_header *)((uint8_t *)cur + cur->size);
            while ((uint64_t)(uintptr_t)next < heap_end && next->free) {
                cur->size += next->size;
                next = (struct block_header *)((uint8_t *)cur + cur->size);
            }
        }
        cur = (struct block_header *)((uint8_t *)cur + cur->size);
    }
}
