#ifndef VMM_H
#define VMM_H

#include "../include/types.h"
#include "pmm.h"

/* Virtual Memory Manager */

#define PAGE_PRESENT   0x1
#define PAGE_WRITABLE  0x2
#define PAGE_USER      0x4

typedef struct {
    uint32_t virtual_addr;
    void *physical_addr;
    uint32_t flags;
} page_entry_t;

/* Initialize virtual memory manager */
void vmm_init(void);

/* Map one virtual page to one physical page */
int vmm_map_page(uint32_t virtual_addr, void *physical_addr, uint32_t flags);

/* Translate virtual address to physical address */
void *vmm_translate(uint32_t virtual_addr);

/* Statistics */
uint32_t vmm_mapped_pages(void);

#endif
