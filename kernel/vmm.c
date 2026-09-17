#include "vmm.h"

/*
 * Simple Virtual Memory Manager
 *
 * This stage maintains a small software page mapping table.
 * It demonstrates the basic idea of mapping virtual pages
 * to physical pages.
 */

#define MAX_MAPPINGS 256

static page_entry_t page_table[MAX_MAPPINGS];
static uint32_t mapping_count;

void vmm_init(void)
{
    uint32_t i;

    mapping_count = 0;

    for (i = 0; i < MAX_MAPPINGS; i++) {
        page_table[i].virtual_addr = 0;
        page_table[i].physical_addr = 0;
        page_table[i].flags = 0;
    }
}

int vmm_map_page(uint32_t virtual_addr, void *physical_addr, uint32_t flags)
{
    uint32_t i;

    if (physical_addr == 0)
        return -1;

    /* Virtual addresses must be page aligned. */
    if ((virtual_addr % PAGE_SIZE) != 0)
        return -1;

    /* Update an existing mapping if it already exists. */
    for (i = 0; i < mapping_count; i++) {
        if (page_table[i].virtual_addr == virtual_addr) {
            page_table[i].physical_addr = physical_addr;
            page_table[i].flags = flags | PAGE_PRESENT;
            return 0;
        }
    }

    if (mapping_count >= MAX_MAPPINGS)
        return -1;

    page_table[mapping_count].virtual_addr = virtual_addr;
    page_table[mapping_count].physical_addr = physical_addr;
    page_table[mapping_count].flags = flags | PAGE_PRESENT;

    mapping_count++;

    return 0;
}

void *vmm_translate(uint32_t virtual_addr)
{
    uint32_t i;
    uint32_t page_addr;
    uint32_t offset;

    page_addr = virtual_addr & ~(PAGE_SIZE - 1);
    offset = virtual_addr & (PAGE_SIZE - 1);

    for (i = 0; i < mapping_count; i++) {
        if (page_table[i].virtual_addr == page_addr &&
            (page_table[i].flags & PAGE_PRESENT)) {

            return (void *)((uint8_t *)page_table[i].physical_addr + offset);
        }
    }

    return 0;
}

uint32_t vmm_mapped_pages(void)
{
    return mapping_count;
}
