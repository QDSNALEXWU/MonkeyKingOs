
#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "system_call.h"

// constants and masks
#define END_KERNEL 0x800000
#define MAX_NUMBER_PAGE	 1024
#define TABLE_ALLIGN 4096
#define KERNEL_ADDR 0x400000
#define KERNEL_PAGE_MASK 0x0183
#define VIDEO_MEMORY_PAGE_DIRE_MASK 0x0003
#define VIDEO_MEMORY_PAGE_TABL_MASK 0x0003
#define INIT_MASK 0x03
#define PAGE_SIZE 4096
#define CR0_MASK 0x80000001
#define CR4_MASK 0x00000010
#define PROCESS_PAGE_MASK 	0x0087
#define VIRTUAL_ADDR_START 0x08000000
#define FOUR_MB 0x400000
#define PDE_MASK 0xffc00000
#define MAX_PROCESS 6

extern void init_paging();

int32_t find_avail_process();

void free_process(uint32_t index);

uint32_t allocate_new_page(uint32_t index);

uint32_t* get_pd_ptr(int32_t pid);
#endif /* PAGING_H */
