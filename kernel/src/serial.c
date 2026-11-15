#include "serial.h"
#include "kernel.h"

#if defined(__arm__)
/* BCM2835 PL011 UART0 and GPIO14/15 on Raspberry Pi Zero W. */
#define MMIO32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define GPIO_BASE 0x20200000u
#define GPFSEL1   (GPIO_BASE + 0x04u)
#define GPPUD     (GPIO_BASE + 0x94u)
#define GPPUDCLK0 (GPIO_BASE + 0x98u)
#define UART0_BASE 0x20201000u
#define UART_DR   (UART0_BASE + 0x00u)
#define UART_FR   (UART0_BASE + 0x18u)
#define UART_IBRD (UART0_BASE + 0x24u)
#define UART_FBRD (UART0_BASE + 0x28u)
#define UART_LCRH (UART0_BASE + 0x2cu)
#define UART_CR   (UART0_BASE + 0x30u)
#define UART_IMSC (UART0_BASE + 0x38u)
#define UART_ICR  (UART0_BASE + 0x44u)

void serial_init(void) {
    /* Select ALT0 (TXD0/RXD0) for GPIO14 and GPIO15. */
    uint32_t fsel = MMIO32(GPFSEL1);
    fsel &= ~((7u << 12) | (7u << 15));
    fsel |=  ((4u << 12) | (4u << 15));
    MMIO32(GPFSEL1) = fsel;

    /* Disable pull-up/down on GPIO14/15 (legacy BCM2835 sequence). */
    MMIO32(GPPUD) = 0;
    for (volatile unsigned i = 0; i < 150; ++i) __asm__ volatile("nop");
    MMIO32(GPPUDCLK0) = (1u << 14) | (1u << 15);
    for (volatile unsigned i = 0; i < 150; ++i) __asm__ volatile("nop");
    MMIO32(GPPUDCLK0) = 0;

    MMIO32(UART_CR) = 0;
    MMIO32(UART_ICR) = 0x7ff;
    /* Assumes the firmware's standard 48 MHz PL011 UART clock. */
    MMIO32(UART_IBRD) = 26;
    MMIO32(UART_FBRD) = 3;
    MMIO32(UART_LCRH) = (3u << 5) | (1u << 4); /* 8N1, FIFO */
    MMIO32(UART_IMSC) = 0;
    MMIO32(UART_CR) = (1u << 9) | (1u << 8) | 1u; /* RX, TX, UART */
}

int serial_received(void) { return (MMIO32(UART_FR) & (1u << 4)) == 0; }
char serial_read(void) { while (!serial_received()) {} return (char)MMIO32(UART_DR); }
void serial_putchar(char c) {
    if (c == '\n') serial_putchar('\r');
    while (MMIO32(UART_FR) & (1u << 5)) {}
    MMIO32(UART_DR) = (uint32_t)c;
}
char serial_getchar(void) { while (!serial_received()) {} return (char)MMIO32(UART_DR); }

#else
void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x80);
    outb(COM1_PORT + 0, 0x03);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);
    outb(COM1_PORT + 2, 0xC7);
    outb(COM1_PORT + 4, 0x0B);
}
int serial_received(void) { return inb(COM1_PORT + 5) & 1; }
char serial_read(void) { while (!serial_received()) {} return inb(COM1_PORT); }
void serial_putchar(char c) {
    while ((inb(COM1_PORT + 5) & 0x20) == 0) {}
    outb(COM1_PORT, (uint8_t)c);
}
char serial_getchar(void) { return serial_read(); }
#endif

void serial_print(const char *s) {
    while (*s) serial_putchar(*s++);
}
void serial_print_hex(uint64_t value) {
    const char *hex = "0123456789ABCDEF";
    serial_print("0x");
    for (int i = 15; i >= 0; --i)
        serial_putchar(hex[(value >> (i * 4)) & 0xf]);
}
