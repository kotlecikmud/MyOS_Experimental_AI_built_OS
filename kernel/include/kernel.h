#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Port I/O functions */
#ifdef __x86_64__
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}
#else
static inline void outb(uint16_t port, uint8_t val) {
    (void)port; (void)val;
}

static inline uint8_t inb(uint16_t port) {
    (void)port;
    return 0;
}

static inline void io_wait(void) {
}
#endif

/* Halt CPU */
static inline void halt(void) {
    for (;;) {
#ifdef __x86_64__
        __asm__ volatile ("hlt");
#endif
    }
}

void demo_run(void);

#endif /* KERNEL_H */
