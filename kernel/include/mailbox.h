#ifndef RPI_MAILBOX_H
#define RPI_MAILBOX_H

#include <stdint.h>

/* BCM2835 Mailbox base address */
#define RPI_MAILBOX_BASE    0x2000B880

#define MAILBOX_READ        (volatile uint32_t*)(RPI_MAILBOX_BASE + 0x00)
#define MAILBOX_STATUS      (volatile uint32_t*)(RPI_MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE       (volatile uint32_t*)(RPI_MAILBOX_BASE + 0x20)

#define MAILBOX_FULL        0x80000000
#define MAILBOX_EMPTY       0x40000000

#define MAILBOX_CHANNEL_PROP 8

void rpi_mailbox_write(uint8_t channel, uint32_t data);
uint32_t rpi_mailbox_read(uint8_t channel);
int rpi_mailbox_property(void* tag_buffer);

#endif
