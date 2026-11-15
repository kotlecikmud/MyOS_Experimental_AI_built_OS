#include "mailbox.h"

#define MMIO32(a) (*(volatile uint32_t *)(uintptr_t)(a))

void rpi_mailbox_write(uint8_t channel, uint32_t data) {
    while (MMIO32(RPI_MAILBOX_BASE + 0x18) & MAILBOX_FULL)
        __asm__ volatile("nop");
    MMIO32(RPI_MAILBOX_BASE + 0x20) = (data & 0xfffffff0u) | (channel & 0x0fu);
}

uint32_t rpi_mailbox_read(uint8_t channel) {
    for (;;) {
        while (MMIO32(RPI_MAILBOX_BASE + 0x18) & MAILBOX_EMPTY)
            __asm__ volatile("nop");
        uint32_t value = MMIO32(RPI_MAILBOX_BASE);
        if ((value & 0x0fu) == channel) return value & 0xfffffff0u;
    }
}

int rpi_mailbox_property(void *tag_buffer) {
    uintptr_t address = (uintptr_t)tag_buffer;
    if ((address & 0x0fu) != 0 || address >= 0x40000000u) return -1;
    __asm__ volatile("mcr p15, 0, %0, c7, c10, 5"
                 :
                 : "r"(0)
                 : "memory");
    /* ARM physical RAM address -> VideoCore bus address (uncached alias). */
    uint32_t bus_address = 0x40000000u | ((uint32_t)address & 0x3ffffff0u);
    rpi_mailbox_write(MAILBOX_CHANNEL_PROP, bus_address);
    uint32_t response = rpi_mailbox_read(MAILBOX_CHANNEL_PROP);
    __asm__ volatile("mcr p15, 0, %0, c7, c10, 5"
                 :
                 : "r"(0)
                 : "memory");
    return response == bus_address ? 0 : -1;
}
