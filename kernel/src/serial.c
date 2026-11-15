#include "serial.h"
#include "kernel.h"

void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38,400 baud
    outb(COM1_PORT + 1, 0x00);    // (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear with 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // Enable IRQs, RTS/DSR set
}

static int is_transmit_empty(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_putchar(char c) {
    if (c == '\n') {
        while (!is_transmit_empty());
        outb(COM1_PORT, '\r');
    }
    while (!is_transmit_empty());
    outb(COM1_PORT, c);
}

char serial_getchar(void) {
    /* Blocking COM1 input; console_getline polls this while waiting for input. */
    while ((inb(COM1_PORT + 5) & 0x01) == 0) {
        __asm__ volatile ("pause");
    }
    return (char)inb(COM1_PORT);
}

void serial_print(const char* s) {
    while (*s) {
        serial_putchar(*s++);
    }
}

void serial_print_hex(uint64_t val) {
    serial_print("0x");
    char buf[17];
    buf[16] = '\0';
    int i = 15;
    while (i >= 0) {
        int nibble = val & 0xF;
        buf[i] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
        val >>= 4;
        i--;
    }
    serial_print(buf);
}
