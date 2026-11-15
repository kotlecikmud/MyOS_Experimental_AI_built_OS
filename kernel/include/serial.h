#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stddef.h>

#define COM1_PORT 0x3F8

void serial_init(void);
void serial_putchar(char c);
char serial_getchar(void);
void serial_print(const char* s);
void serial_print_hex(uint64_t val);

#endif /* SERIAL_H */
