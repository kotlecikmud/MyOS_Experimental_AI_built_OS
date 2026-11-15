#include "gdt.h"
#include "serial.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gp;

extern void gdt_flush(uint64_t);

void gdt_init(void) {
    serial_print("[GDT] Initializing Global Descriptor Table...\n");
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (uint64_t)&gdt;

    // Null descriptor
    gdt[0] = (struct gdt_entry){0, 0, 0, 0, 0, 0};

    // Kernel Code Segment (64-bit)
    gdt[1] = (struct gdt_entry){0, 0, 0, 0x9A, 0x20, 0};

    // Kernel Data Segment (64-bit)
    gdt[2] = (struct gdt_entry){0, 0, 0, 0x92, 0x00, 0};

    // Note: Limine sets up a valid 64-bit GDT, but initializing our own is good practice.
    serial_print("[GDT] GDT initialized successfully.\n");
}
