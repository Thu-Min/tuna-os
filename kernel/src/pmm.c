#include "pmm.h"
#include "multiboot2.h"
#include "serial.h"

extern char _kernel_start[];
extern char _kernel_end[];

static uint8_t bitmap[PMM_MAX_FRAMES / 8];
static uint32_t total_frames;
static uint32_t used_frames;

static void bitmap_set(uint32_t frame) {
    bitmap[frame / 8] |= (1 << (frame % 8));
}

static void bitmap_clear(uint32_t frame) {
    bitmap[frame / 8] &= ~(1 << (frame % 8));
}

static int bitmap_test(uint32_t frame) {
    return bitmap[frame / 8] & (1 << (frame % 8));
}

void pmm_init(const struct mmap_region *regions, uint32_t count) {
    /* Mark all frames as used initially */
    for (uint32_t i = 0; i < PMM_MAX_FRAMES / 8; i++)
        bitmap[i] = 0xFF;

    total_frames = 0;
    used_frames = PMM_MAX_FRAMES;

    /* Clear bits for usable RAM regions */
    for (uint32_t i = 0; i < count; i++) {
        if (regions[i].type != MULTIBOOT2_MMAP_TYPE_AVAILABLE)
            continue;

        uint64_t base = regions[i].base;
        uint64_t end  = base + regions[i].length;

        uint32_t frame_start = (uint32_t)((base + PMM_PAGE_SIZE - 1) / PMM_PAGE_SIZE);
        uint32_t frame_end   = (uint32_t)(end / PMM_PAGE_SIZE);

        if (frame_end > PMM_MAX_FRAMES)
            frame_end = PMM_MAX_FRAMES;

        for (uint32_t f = frame_start; f < frame_end; f++) {
            bitmap_clear(f);
            total_frames++;
            used_frames--;
        }
    }

    /* Re-mark low memory (below 1 MiB) as used */
    uint32_t low_end = (1024 * 1024) / PMM_PAGE_SIZE; /* frame 256 */
    for (uint32_t f = 0; f < low_end && f < PMM_MAX_FRAMES; f++) {
        if (!bitmap_test(f)) {
            bitmap_set(f);
            used_frames++;
        }
    }

    /* Re-mark kernel region as used */
    uint64_t k_start = (uint64_t)(uintptr_t)_kernel_start;
    uint64_t k_end   = (uint64_t)(uintptr_t)_kernel_end;

    uint32_t kf_start = (uint32_t)(k_start / PMM_PAGE_SIZE);
    uint32_t kf_end   = (uint32_t)((k_end + PMM_PAGE_SIZE - 1) / PMM_PAGE_SIZE);

    if (kf_end > PMM_MAX_FRAMES)
        kf_end = PMM_MAX_FRAMES;

    for (uint32_t f = kf_start; f < kf_end; f++) {
        if (!bitmap_test(f)) {
            bitmap_set(f);
            used_frames++;
        }
    }

    serial_write("pmm: total frames=");
    serial_write_dec_u64(total_frames);
    serial_write(" used=");
    serial_write_dec_u64(used_frames);
    serial_write(" free=");
    serial_write_dec_u64(total_frames - used_frames);
    serial_write("\n");
}

uint64_t pmm_alloc_frame(void) {
    for (uint32_t i = 0; i < PMM_MAX_FRAMES / 8; i++) {
        if (bitmap[i] == 0xFF)
            continue;

        for (uint8_t bit = 0; bit < 8; bit++) {
            uint32_t frame = i * 8 + bit;
            if (frame >= PMM_MAX_FRAMES)
                return 0;

            if (!bitmap_test(frame)) {
                bitmap_set(frame);
                used_frames++;
                return (uint64_t)frame * PMM_PAGE_SIZE;
            }
        }
    }

    return 0;
}

void pmm_free_frame(uint64_t addr) {
    uint32_t frame = (uint32_t)(addr / PMM_PAGE_SIZE);

    if (frame >= PMM_MAX_FRAMES)
        return;

    if (bitmap_test(frame)) {
        bitmap_clear(frame);
        used_frames--;
    }
}

uint32_t pmm_get_free_count(void) {
    return total_frames - used_frames;
}

uint32_t pmm_get_total_count(void) {
    return total_frames;
}
