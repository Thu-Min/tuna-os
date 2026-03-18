#include "pit.h"

#include <stdint.h>

#include "idt.h"
#include "io.h"
#include "pic.h"
#include "serial.h"
#include "task.h"

#define PIT_CHANNEL0_DATA 0x40
#define PIT_COMMAND       0x43
#define PIT_BASE_FREQ     1193182

#define SCHEDULE_INTERVAL 10  /* context switch every 10 ticks (100 ms at 100 Hz) */

static volatile uint64_t tick_count = 0;
static uint32_t configured_frequency = 0;

static void pit_irq_handler(struct interrupt_frame *frame) {
    (void)frame;
    tick_count++;

    if (tick_count % configured_frequency == 0) {
        serial_write("[pit] tick=");
        serial_write_dec_u64(tick_count);
        serial_write("\n");
    }

    if (tick_count % SCHEDULE_INTERVAL == 0) {
        /*
         * Send EOI before schedule() — if schedule() switches to another
         * task, control won't return here until this task is switched back.
         * Without early EOI, the timer would stop firing.
         */
        pic_eoi(0);
        schedule();
    }
}

void pit_init(uint32_t frequency) {
    configured_frequency = frequency;

    uint16_t divisor = (uint16_t)(PIT_BASE_FREQ / frequency);

    // Channel 0, lobyte/hibyte, mode 2 (rate generator)
    outb(PIT_COMMAND, 0x34);
    outb(PIT_CHANNEL0_DATA, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_DATA, (uint8_t)((divisor >> 8) & 0xFF));

    irq_register_handler(0, pit_irq_handler);
    irq_unmask(0);

    serial_write("[pit] initialized at ");
    serial_write_dec_u64(frequency);
    serial_write(" Hz\n");
}

uint64_t pit_get_ticks(void) {
    return tick_count;
}
