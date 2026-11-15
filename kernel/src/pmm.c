#include "pmm.h"
#include "serial.h"

/* Bitmap is indexed by physical page number. Bits set to 1 are reserved. */
static uint8_t* pmm_bitmap = NULL;
static uint64_t pmm_max_blocks = 0;
static uint64_t pmm_used_blocks = 0;
static uint64_t pmm_total_memory = 0;
static uint64_t pmm_bitmap_phys = 0;

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

#define LIMINE_MEMMAP_USABLE 0
#define BITMAP_BYTES(n) (((n) + 7) / 8)
#define ALIGN_UP(v, a) (((v) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(v, a) ((v) & ~((a) - 1))

static void bitmap_set(uint64_t bit) {
    pmm_bitmap[bit / 8] |= (uint8_t)(1u << (bit % 8));
}

static void bitmap_unset(uint64_t bit) {
    pmm_bitmap[bit / 8] &= (uint8_t)~(1u << (bit % 8));
}

static int bitmap_test(uint64_t bit) {
    return (pmm_bitmap[bit / 8] & (uint8_t)(1u << (bit % 8))) != 0;
}

static int64_t bitmap_find_first_free(void) {
    for (uint64_t bit = 0; bit < pmm_max_blocks; bit++) {
        if (!bitmap_test(bit)) return (int64_t)bit;
    }
    return -1;
}

void pmm_init(void) {
    serial_print("[PMM] Physical Memory Allocator initializing...\n");
    pmm_bitmap = NULL;
    pmm_max_blocks = 0;
    pmm_used_blocks = 0;
    pmm_total_memory = 0;
    pmm_bitmap_phys = 0;
}

void pmm_init_region(uint64_t base, uint64_t size) {
    /* Legacy fixed-map hook retained for the ARM architecture backends. */
    (void)base;
    pmm_total_memory += size;
    pmm_max_blocks = pmm_total_memory / PMM_PAGE_SIZE;
    serial_print("[PMM] Registered memory region: Base=");
    serial_print_hex(base);
    serial_print(" Size=");
    serial_print_hex(size);
    serial_print("\n");
}

void pmm_mark_region_reserved(uint64_t base, uint64_t size) {
    if (pmm_bitmap == NULL || size == 0) return;
    uint64_t first = base / PMM_PAGE_SIZE;
    uint64_t last = ALIGN_UP(base + size, PMM_PAGE_SIZE) / PMM_PAGE_SIZE;
    if (last > pmm_max_blocks) last = pmm_max_blocks;
    for (uint64_t bit = first; bit < last; bit++) {
        if (!bitmap_test(bit)) {
            bitmap_set(bit);
            pmm_used_blocks++;
        }
    }
}

void* pmm_alloc_block(void) {
    if (pmm_bitmap == NULL || pmm_used_blocks >= pmm_max_blocks) {
        serial_print("[PMM OOM] Allocator is unavailable or out of memory.\n");
        return NULL;
    }
    int64_t bit = bitmap_find_first_free();
    if (bit < 0) return NULL;
    bitmap_set((uint64_t)bit);
    pmm_used_blocks++;
    /* Return the physical address; callers can map it through the HHDM. */
    return (void*)((uint64_t)bit * PMM_PAGE_SIZE);
}

void pmm_free_block(void* ptr) {
    if (pmm_bitmap == NULL || ptr == NULL) return;
    uint64_t addr = (uint64_t)ptr;
    if ((addr & (PMM_PAGE_SIZE - 1)) != 0) return;
    uint64_t bit = addr / PMM_PAGE_SIZE;
    if (bit >= pmm_max_blocks || !bitmap_test(bit)) return;
    /* Never free the bitmap's own backing pages. */
    uint64_t bitmap_pages = ALIGN_UP(BITMAP_BYTES(pmm_max_blocks), PMM_PAGE_SIZE) / PMM_PAGE_SIZE;
    uint64_t bitmap_first = pmm_bitmap_phys / PMM_PAGE_SIZE;
    if (bit >= bitmap_first && bit < bitmap_first + bitmap_pages) return;
    bitmap_unset(bit);
    pmm_used_blocks--;
}

uint64_t pmm_get_total_memory(void) { return pmm_total_memory; }
uint64_t pmm_get_free_memory(void) {
    return (pmm_max_blocks - pmm_used_blocks) * PMM_PAGE_SIZE;
}

void x86_64_pmm_init(void* memmap_ptr, uint64_t hhdm_offset) {
    struct limine_memmap_response* response = (struct limine_memmap_response*)memmap_ptr;
    if (response == NULL || response->entries == NULL || response->entry_count == 0) {
        serial_print("[PMM x86-64] Invalid Limine memory map.\n");
        return;
    }

    uint64_t highest_address = 0;
    pmm_total_memory = 0;

    for (uint64_t i = 0; i < response->entry_count; i++) {
        struct limine_memmap_entry* entry = response->entries[i];
        if (entry == NULL || entry->length == 0 || entry->base > UINT64_MAX - entry->length) continue;
        uint64_t end = entry->base + entry->length;
        if (end > highest_address) highest_address = end;
        if (entry->type == LIMINE_MEMMAP_USABLE) pmm_total_memory += entry->length;
    }

    pmm_max_blocks = ALIGN_UP(highest_address, PMM_PAGE_SIZE) / PMM_PAGE_SIZE;
    uint64_t bitmap_size = BITMAP_BYTES(pmm_max_blocks);
    uint64_t bitmap_storage_size = ALIGN_UP(bitmap_size, PMM_PAGE_SIZE);
    uint64_t bitmap_phys = 0;
    int found = 0;

    for (uint64_t i = 0; i < response->entry_count; i++) {
        struct limine_memmap_entry* entry = response->entries[i];
        if (entry == NULL || entry->type != LIMINE_MEMMAP_USABLE) continue;
        uint64_t start = ALIGN_UP(entry->base, PMM_PAGE_SIZE);
        uint64_t end = ALIGN_DOWN(entry->base + entry->length, PMM_PAGE_SIZE);
        if (end >= start && end - start >= bitmap_storage_size) {
            bitmap_phys = start;
            found = 1;
            break;
        }
    }

    if (!found || bitmap_phys > UINT64_MAX - hhdm_offset) {
        serial_print("[PMM x86-64] Could not place the physical memory bitmap.\n");
        pmm_max_blocks = 0;
        return;
    }

    pmm_bitmap_phys = bitmap_phys;
    pmm_bitmap = (uint8_t*)(uintptr_t)(hhdm_offset + bitmap_phys);
    for (uint64_t i = 0; i < bitmap_size; i++) pmm_bitmap[i] = 0xFF;
    pmm_used_blocks = pmm_max_blocks;

    /* Only full pages wholly contained in usable ranges become allocatable. */
    for (uint64_t i = 0; i < response->entry_count; i++) {
        struct limine_memmap_entry* entry = response->entries[i];
        if (entry == NULL || entry->type != LIMINE_MEMMAP_USABLE) continue;
        uint64_t start = ALIGN_UP(entry->base, PMM_PAGE_SIZE) / PMM_PAGE_SIZE;
        uint64_t end = ALIGN_DOWN(entry->base + entry->length, PMM_PAGE_SIZE) / PMM_PAGE_SIZE;
        for (uint64_t bit = start; bit < end && bit < pmm_max_blocks; bit++) {
            if (bitmap_test(bit)) { bitmap_unset(bit); pmm_used_blocks--; }
        }
    }

    pmm_mark_region_reserved(0, PMM_PAGE_SIZE);
    pmm_mark_region_reserved(bitmap_phys, bitmap_storage_size);
    serial_print("[PMM x86-64] Bitmap initialized. Total usable RAM: ");
    serial_print_hex(pmm_total_memory);
    serial_print(" bytes; free: ");
    serial_print_hex(pmm_get_free_memory());
    serial_print(" bytes.\n");
}

void armv6_pmm_init(void) {
    serial_print("[PMM ARMv6] Initializing Raspberry Pi Zero W memory map (256MB/512MB RAM)...\n");
    pmm_init_region(0x00000000, 0x10000000); // 256MB base physical memory
}

void aarch64_pmm_init(void) {
    serial_print("[PMM AArch64] Initializing Raspberry Pi Zero 2 W memory map (512MB RAM)...\n");
    pmm_init_region(0x00000000, 0x20000000); // 512MB base physical memory
}
