#include "kernel.h"
#include "serial.h"
#include "gdt.h"
#include "idt.h"
#include "memory.h"
#include "pmm.h"
#include "console.h"
#include "vfs.h"
#include "shell.h"
#include "auth.h"
#include "demo.h"

/* Limine 12.x protocol structures and requests. Keep these definitions in sync
 * with the official limine-protocol/include/limine.h header. */
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

struct limine_memmap_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_memmap_response* response;
};

struct limine_hhdm_response {
    uint64_t revision;
    uint64_t offset;
};

struct limine_hhdm_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_hhdm_response* response;
};

__attribute__((used, section(".requests_start_marker"), aligned(8)))
static volatile uint64_t requests_start_marker[] = {
    0xf6b8f4b39de7d1ae, 0xfab91a6940fcb9cf,
    0x785c6ed015d3e316, 0x181e920a7852b9d9
};

/* Base revision 3: Limine writes zero to element 2 when supported. */
__attribute__((used, section(".requests"), aligned(8)))
static volatile uint64_t base_revision[] = {
    0xf9562b2d5c95a6c8, 0x6a7b384944536bdc, 3
};

__attribute__((used, section(".requests"), aligned(8)))
static volatile struct limine_memmap_request memmap_request = {
    .id = {
        0xc7b1dd30df4c8b88, 0x0a82e883a194f07b,
        0x67cf3d9d378a806f, 0xe304acdfc50c3c62
    },
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".requests"), aligned(8)))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = {
        0xc7b1dd30df4c8b88, 0x0a82e883a194f07b,
        0x48dcf1cb8ad2b852, 0x63984e959a98244b
    },
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".requests_end_marker"), aligned(8)))
static volatile uint64_t requests_end_marker[] = {
    0xadc0e0531bb10d03, 0x9572709f31764c62
};

void kernel_main(void) {
    // 1. Initialize Serial Console (COM1) immediately for debugging & logging
    serial_init();

    serial_print("\n");
    serial_print("==================================================\n");
    serial_print("          MyOS - Milestone 1 Booting              \n");
    serial_print("       Custom x86-64 Operating System             \n");
    serial_print("==================================================\n");

    // Check Limine base revision
    if (base_revision[2] != 0) {
        serial_print("[BOOT] Error: Limine base revision 3 is not supported!\n");
        halt();
    }
    if (memmap_request.response == NULL || hhdm_request.response == NULL) {
        serial_print("[BOOT] Error: Limine did not provide required memory responses!\n");
        halt();
    }
    serial_print("[BOOT] Limine protocol and memory requests verified.\n");

    // 2. Initialize CPU Descriptor Tables (GDT & IDT)
    gdt_init();
    idt_init();

    // 3. Initialize Memory Management using Limine memory map
    memory_init(memmap_request.response);
    pmm_init();
    x86_64_pmm_init((void*)memmap_request.response, hhdm_request.response->offset);

    serial_print("==================================================\n");
    serial_print(" MyOS Milestone 1 & 5 Initialization Complete.    \n");
    serial_print(" Launching Interactive Shell Environment...       \n");
    serial_print("==================================================\n");

    // 4. Initialize Console, VFS, Auth, and Shell (Milestones 5, 6, 7)
    console_init();
    vfs_init();
    auth_init();
    shell_init();


    /* Temporary demo-first boot path.
     * The current VFS is RAM-only, so it cannot detect a physical /.demo file.
     * Start the demo directly rather than presenting the authentication screen.
     * Press any key to leave the demo and enter the shell. */
    demo_run();
    shell_run();
    halt();
}
