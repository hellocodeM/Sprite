#include "vmm.h"
#include "idt.h"
#include "printk.h"

PageDictionary page_dictionary __attribute__((aligned(PAGE_SIZE)));

void page_fault_handler(pt_regs* regs) {
    assert(false, "page fault happen");
}

/**
 * initialize virtual memory mapping.
 * 1. initialize page dictionary and page table.
 * 2. maping kernel space 
 * 3. register page fault handler
 */
void init_vmm() {
    // mapping 128MB kernel space
    for (int i = 0; i < 128 * 1024 / 4; i++) {
        page_dictionary.insert(VMA(i << 12) , i << 12);
    }
    
    register_isr(14, &page_fault_handler);
    page_dictionary.register_dictionary();
}
