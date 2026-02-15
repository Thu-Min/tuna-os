#include "serial.h"

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

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

static void serial_putc(char c) {
    while (!tx_empty()) {}
    outb(0x3F8, (uint8_t)c);
}

void serial_write(const char *s) {
    for (; *s; s++) {
        if (*s == '\n') serial_putc('\r');
        serial_putc(*s);
    }
}