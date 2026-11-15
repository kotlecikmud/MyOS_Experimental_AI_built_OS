#include "memory.h"
#include "serial.h"

// Limine memory map definitions
#define LIMINE_MEMMAP_USABLE             0
#define LIMINE_MEMMAP_RESERVED           1
#define LIMINE_MEMMAP_ACPI_RECLAIMABLE   2
#define LIMINE_MEMMAP_ACPI_NVS           3
#define LIMINE_MEMMAP_BAD_MEMORY         4
#define LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define LIMINE_MEMMAP_KERNEL_AND_MODULES 6
#define LIMINE_MEMMAP_FRAMEBUFFER        7
#define LIMINE_MEMMAP_RESERVED_MAPPED    8

struct limine_memmap_entry {
    uint64_t base;
    uint64_t length;
    uint64_t type;
};

struct limine_memmap_response {
    uint64_t revision;
    uint64_t entry_count;
    struct limine_memmap_entry** entries;
};

static const char* memmap_type_to_str(uint64_t type) {
    switch (type) {
        case LIMINE_MEMMAP_USABLE:             return "Usable RAM";
        case LIMINE_MEMMAP_RESERVED:           return "Reserved";
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:   return "ACPI Reclaimable";
        case LIMINE_MEMMAP_ACPI_NVS:           return "ACPI NVS";
        case LIMINE_MEMMAP_BAD_MEMORY:         return "Bad Memory";
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: return "Bootloader Reclaimable";
        case LIMINE_MEMMAP_KERNEL_AND_MODULES: return "Executable / Modules";
        case LIMINE_MEMMAP_FRAMEBUFFER:        return "Framebuffer";
        case LIMINE_MEMMAP_RESERVED_MAPPED: return "Reserved / Mapped";
        default:                               return "Unknown";
    }
}

void memory_init(void* memmap_response_ptr) {
    serial_print("[MEMORY] Initializing System Memory Management...\n");
    if (!memmap_response_ptr) {
        serial_print("[MEMORY] Warning: No memory map response provided by Limine!\n");
        return;
    }

    struct limine_memmap_response* response = (struct limine_memmap_response*)memmap_response_ptr;
    if (response->entries == NULL) {
        serial_print("[MEMORY] Invalid memory map entries pointer.\n");
        return;
    }
    serial_print("[MEMORY] Limine Memory Map entries: ");
    serial_print_hex(response->entry_count);
    serial_print("\n");

    uint64_t total_usable_memory = 0;

    for (uint64_t i = 0; i < response->entry_count; i++) {
        struct limine_memmap_entry* entry = response->entries[i];
        serial_print("  [MemMap] Base: ");
        serial_print_hex(entry->base);
        serial_print(" | Length: ");
        serial_print_hex(entry->length);
        serial_print(" | Type: ");
        serial_print(memmap_type_to_str(entry->type));
        serial_print("\n");

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            total_usable_memory += entry->length;
        }
    }

    serial_print("[MEMORY] Total Usable System RAM: ");
    serial_print_hex(total_usable_memory);
    serial_print(" bytes\n");
    serial_print("[MEMORY] Memory management initialized successfully.\n");
}
