#include "kmalloc.h"
#include "ILAllocator.hpp"
#include "printk.h"
#include "debug.h"
#include "new.hpp"

const uint32_t kHeapStart = 0xD0000000;
const uint32_t kHeapEnd = 0xD1000000;
static ImplicitListAllocator allocator(kHeapStart, kHeapEnd);

void init_heap() { new (&allocator) ImplicitListAllocator(kHeapStart, kHeapEnd); }

void *kmalloc(uint32_t size) { return allocator.alloc(size); }

void kfree(void *pos) { allocator.free(pos); }

void test_kmalloc() {
    printk("test kmalloc: \n");
    int kCnt = 0x1000;

    while (--kCnt) {
        printk("kmalloc cnt: %d\n", kCnt);
        void *addr1 = kmalloc(50);
        assert(reinterpret_cast<uint32_t>(addr1) == 0xD0000004, "kmalloc error");
        void *addr2 = kmalloc(500);
        assert(reinterpret_cast<uint32_t>(addr2) == 0xD000003A, "kmalloc error");
        void *addr3 = kmalloc(5000);
        assert(reinterpret_cast<uint32_t>(addr3) == 0xD0000232, "kmalloc error");

        kfree(addr1);
        kfree(addr2);
        kfree(addr3);
    }
    printk("pass test: kmalloc\n");
}
