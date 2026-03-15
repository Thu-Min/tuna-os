#include "serial.h"

#include "io.h"

static inline int tx_empty(void) {
    // Line Status Register (LSR) bit 5 = THR empty
    return inb(0x3F8 + 5) & (1 << 5);
}

void serial_init(void) {
    // COM1 base = 0x3F8
    outb(0x3F8 + 1, 0x00); // Disable all interrupts
    outb(0x3F8 + 3, 0x80); // Enable DLAB
    outb(0x3F8 + 0, 0x03); // Divisor low (38400 baud if base 115200)
    outb(0x3F8 + 1, 0x00); // Divisor high
    outb(0x3F8 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(0x3F8 + 2, 0xC7); // Enable FIFO, clear, 14-byte threshold
    outb(0x3F8 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void serial_write_char(char c) {
    while (!tx_empty()) {}
    outb(0x3F8, (uint8_t)c);
}

void serial_write(const char *s) {
    for (; *s; s++) {
        if (*s == '\n') serial_write_char('\r');
        serial_write_char(*s);
    }
}

void serial_write_hex_u64(uint64_t value) {
    static const char hex[] = "0123456789ABCDEF";
    int started = 0;

    serial_write("0x");
    for (int shift = 60; shift >= 0; shift -= 4) {
        uint8_t nibble = (uint8_t)((value >> shift) & 0xF);
        if (nibble != 0 || started || shift == 0) {
            serial_write_char(hex[nibble]);
            started = 1;
        }
    }
}

void serial_write_dec_u64(uint64_t value) {
    char buf[21];
    int i = 0;

    if (value == 0) {
        serial_write_char('0');
        return;
    }

    while (value > 0) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0) {
        serial_write_char(buf[--i]);
    }
}
