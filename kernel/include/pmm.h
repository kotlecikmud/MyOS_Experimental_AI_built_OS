#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PMM_PAGE_SIZE 4096

// Shared Physical Memory Allocator Interface
void pmm_init(void);
void pmm_init_region(uint64_t base, uint64_t size);
void pmm_mark_region_reserved(uint64_t base, uint64_t size);
void* pmm_alloc_block(void);
void pmm_free_block(void* ptr);
uint64_t pmm_get_total_memory(void);
uint64_t pmm_get_free_memory(void);

// Architecture specific memory initialization hooks
void x86_64_pmm_init(void* limine_memmap_response, uint64_t hhdm_offset);
void armv6_pmm_init(void);
void aarch64_pmm_init(void);

#endif /* PMM_H */
