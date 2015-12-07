#include "pmm.h"
#include "multiboot.h"
#include "printk.h"
#include "types.h"
#include "debug.h"

// 管理物理内存的栈
static uint32_t pmm_stack[PAGE_MAX_SIZE + 1];

// 栈顶
static uint32_t pmm_stack_top;

// 物理页的数量
uint32_t phy_page_count;

void show_mmap() {
    uint32_t mmap_addr = glb_mboot_ptr->mmap_addr;
    uint32_t mmap_length = glb_mboot_ptr->mmap_length;

    printk("Memory map:\n");

    mmap_entry_t *mmap = (mmap_entry_t *)mmap_addr;
    for (; (uint32_t)mmap < mmap_addr + mmap_length; mmap++) {
        printk("base_addr= 0x%x%x, length = %x%x, type = %x\n", (uint32_t)mmap->base_addr_high,
               (uint32_t)mmap->base_addr_low, (uint32_t)mmap->length_high,
               (uint32_t)mmap->length_low, (uint32_t)mmap->type);
    }
}

void init_pmm() {
    pmm_stack_top = -1;
    phy_page_count = 0;

    mmap_entry_t *mmap_start_addr = (mmap_entry_t *)glb_mboot_ptr->mmap_addr;
    mmap_entry_t *mmap_end_addr =
        (mmap_entry_t *)glb_mboot_ptr->mmap_addr + glb_mboot_ptr->mmap_length;

    mmap_entry_t *mmap_entry = mmap_start_addr;
    for (; mmap_entry < mmap_end_addr; mmap_entry++) {
        // 1表示可用内存, 0x100000指的是1M以上的地址空间
        if (mmap_entry->type == 1 && mmap_entry->base_addr_low == 0x100000) {
            uint32_t page_addr = mmap_entry->base_addr_low + (uint32_t)(kern_end - kern_start);
            uint32_t length = mmap_entry->base_addr_low + mmap_entry->length_low;

            while (page_addr < length && page_addr <= PMM_MAX_SIZE) {
                pmm_free_page(page_addr);
                page_addr += PMM_PAGE_SIZE;
                phy_page_count++;
            }
        }
    }
}

uint32_t pmm_alloc_page() {
    assert(pmm_stack_top != 0, "out of memory");
    return pmm_stack[pmm_stack_top--];
}

uint32_t pmm_alloc_page(uint32_t pma) {
    for (uint32_t i = 0; i <= pmm_stack_top; i++) {
        if (pmm_stack[i] == pma) {
            uint32_t res = pmm_stack[i];
            pmm_stack[i] = pmm_stack[pmm_stack_top--];
            return res;
        }
    }
    return 0;
}

void pmm_free_page(uint32_t p) {
    assert(pmm_stack_top != PAGE_MAX_SIZE, "out of pmm_stack");
    pmm_stack[++pmm_stack_top] = p;
}

