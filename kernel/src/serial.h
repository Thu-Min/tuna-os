#pragma once
#include <stdint.h>

void serial_init(void);
void serial_write_char(char c);
void serial_write(const char *s);
void serial_write_hex_u64(uint64_t value);
void serial_write_dec_u64(uint64_t value);
