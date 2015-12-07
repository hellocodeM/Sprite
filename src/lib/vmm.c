#include "vmm.h"
#include "idt.h"
#include "printk.h"

PageDictionary page_dictionary __attribute__((aligned(PAGE_SIZE)));

void page_fault_handler(pt_regs* regs) {
    uint32_t addr;
    asm volatile("mov %%cr2, %0" :  "=r"(addr));
    uint32_t err_code = regs->err_code;

    // explain it
    printk("page fault at 0x%x:  ", addr);
    if (!(err_code & PAGE_WRITE)) {
        printk("read error\n");
    } else {
        printk("write error\n");
    }
    
    // if page is not present, allocate a physical page for it
    if (!(err_code & PAGE_PRESENT)) {
        printk("allocate physical page for it, pma: 0x%x\n", page_dictionary[addr]);
    }
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

void test_vmm() {
    int i = 0;
    int* p = &i;
    printk("variable address: 0x%x\n", reinterpret_cast<uint32_t>(p));
    printk("variable value: %d\n", *p);

    printk("value at 0xc0000000: 0x%x\n", *(reinterpret_cast<uint32_t*>(0xc0000000)));
    printk("value at 0xc0001000: 0x%x\n", *(reinterpret_cast<uint32_t*>(0xc0001000)));
    printk("value at 0xc0002000: 0x%x\n", *(reinterpret_cast<uint32_t*>(0xc0002000)));

    // these will cause a page fault
    printk("read 0x10002000: 0x%x\n", *(reinterpret_cast<uint32_t*>(0x10002000)));
    printk("write 0x0 to 0x10003000\n");
    *(reinterpret_cast<uint32_t*>(0x10003000)) = 0;
}
