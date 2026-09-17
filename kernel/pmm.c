#include "pmm.h"

/*
 * Simple Physical Memory Manager
 *
 * Each entry in page_bitmap represents one 4 KB physical page.
 * 0 = free
 * 1 = allocated
 */

static uint8_t page_bitmap[MAX_PAGES];
static uint32_t free_page_count;

/* Simulated physical memory used by the educational kernel. */
static uint8_t physical_memory[MAX_PAGES * PAGE_SIZE]
    __attribute__((aligned(PAGE_SIZE)));

void pmm_init(void)
{
    uint32_t i;

    for (i = 0; i < MAX_PAGES; i++) {
        page_bitmap[i] = 0;
    }

    free_page_count = MAX_PAGES;
}

void *pmm_alloc_page(void)
{
    uint32_t i;

    for (i = 0; i < MAX_PAGES; i++) {
        if (page_bitmap[i] == 0) {
            page_bitmap[i] = 1;
            free_page_count--;

            return (void *)&physical_memory[i * PAGE_SIZE];
        }
    }

    /* No free physical pages available. */
    return 0;
}

void pmm_free_page(void *page)
{
    uint32_t index;
    uint32_t offset;

    if (page == 0)
        return;

    offset = (uint32_t)((uint8_t *)page - physical_memory);

    if (offset >= (MAX_PAGES * PAGE_SIZE))
        return;

    if ((offset % PAGE_SIZE) != 0)
        return;

    index = offset / PAGE_SIZE;

    if (page_bitmap[index] != 0) {
        page_bitmap[index] = 0;
        free_page_count++;
    }
}

uint32_t pmm_total_pages(void)
{
    return MAX_PAGES;
}

uint32_t pmm_free_pages(void)
{
    return free_page_count;
}

uint32_t pmm_used_pages(void)
{
    return MAX_PAGES - free_page_count;
}
