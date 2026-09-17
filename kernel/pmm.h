#ifndef PMM_H
#define PMM_H

#include "../include/types.h"

/* Physical Memory Manager */

#define PAGE_SIZE       4096
#define MAX_PAGES       4096

/* Initialize physical memory manager */
void pmm_init(void);

/* Allocate one physical page */
void *pmm_alloc_page(void);

/* Free a previously allocated page */
void pmm_free_page(void *page);

/* Memory statistics */
uint32_t pmm_total_pages(void);
uint32_t pmm_free_pages(void);
uint32_t pmm_used_pages(void);

#endif
