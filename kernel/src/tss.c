#include "tss.h"
#include "serial.h"

static struct tss tss;

void tss_init(uint64_t kernel_stack) {
    __builtin_memset(&tss, 0, sizeof(tss));
    tss.rsp0 = kernel_stack;
    tss.iopb_offset = sizeof(tss); /* no IOPB — offset past end */

    serial_write("[tss] rsp0=");
    serial_write_hex_u64(tss.rsp0);
    serial_write(" size=");
    serial_write_dec_u64(sizeof(tss));
    serial_write("\n");
}

void tss_set_rsp0(uint64_t rsp0) {
    tss.rsp0 = rsp0;
}

struct tss *tss_get(void) {
    return &tss;
}
