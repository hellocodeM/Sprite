#ifndef VMM_H
#define VMM_H

#include "types.h"
#include "pmm.h"
#include "debug.h"

#define PAGE_OFFSET 0xC0000000
#define PMA(x) ((x) - PAGE_OFFSET)
#define VMA(x) ((x) + PAGE_OFFSET)

#define PAGE_PRESENT 0b1
#define PAGE_WRITE 0b10
#define PAGE_USER 0b100

#define PAGE_MASK 0xFFFFF000
#define ENTRY_ADDR 0xFFFFF000
#define PGD_INDEX(x) ((x) >> 22 & 0x3FF)
#define PTE_INDEX(x) ((x) >> 12 & 0x3FF)

#define PAGE_SIZE 4096
#define TABLE_MAX_SIZE 1024
#define DICTIONARY_MAX_SIZE 1024
#define TABLE_RESERVE 128

typedef uint32_t pde_t;
typedef uint32_t pte_t;

void init_vmm();

void test_vmm();

class PageTable {
public:
    /**
     * retieve mapping from virtual address to physical address,
     * if miss, return 0
     */
    uint32_t at(uint32_t vma) const {
        uint32_t table_index_ = table_index(vma);
        uint32_t page_offset_ = page_offset(vma);

        uint32_t pte = entries[table_index_];
        if (!(pte & PAGE_PRESENT)) return 0;
        return (pte & ENTRY_ADDR) | page_offset_;
    }

    /**
     * mapping virtual memory address to physical address,
     * if miss, allocate a physical page for it.
     */
    uint32_t operator[](uint32_t vma) {
        uint32_t table_index_ = table_index(vma);
        uint32_t page_offset_ = page_offset(vma);

        uint32_t& pte = entries[table_index_];
        if (!(pte & PAGE_PRESENT)) {
            pte = pmm_alloc_page() | PAGE_PRESENT | PAGE_WRITE;
        }

        return (pte & ENTRY_ADDR) | page_offset_;
    }

    /**
     * directly mapping a virtual page to a physical page.
     */
    void insert(uint32_t vma, uint32_t pma) {
        uint32_t table_index_ = table_index(vma);

        uint32_t& pte = entries[table_index_];
        pte = (pma & ENTRY_ADDR) | PAGE_PRESENT | PAGE_WRITE;
    }

private:
    static inline uint32_t table_index(uint32_t addr) { return (addr >> 12) & 0x3FF; }

    static inline uint32_t page_offset(uint32_t addr) { return (addr & 0xFFF); }

    /* data members */
    uint32_t entries[TABLE_MAX_SIZE] = {0};
};

class PageDictionary {
public:
    /**
     * retrieve physical memory address from virtual address.
     * if miss, return 0
     */
    uint32_t at(uint32_t vma) const {
        uint32_t dict_index_ = dict_index(vma);

        PageTable* pt = entries[dict_index_];
        uint32_t pde = reinterpret_cast<uint32_t>(pt);
        if (!(pde & PAGE_PRESENT)) return 0;
        return real_page_table(pt)->at(vma);
    }

    /**
     * mapping virtual memory address to physical memory address,
     * if miss, allocate a page table or physical page for it.
     */
    uint32_t operator[](uint32_t vma) {
        uint32_t dict_index_ = dict_index(vma);

        // retrieve page table
        PageTable*& pt = entries[dict_index_];
        uint32_t& pde = reinterpret_cast<uint32_t&>(pt);
        if (!(pde & PAGE_PRESENT)) {
            set_page_table(pt, alloc_table());
        }
        return (*real_page_table(pt))[vma];
    }

    /**
     * directly map a virtual memory page to a physical page.
     */
    void insert(uint32_t vma, uint32_t pma) {
        uint32_t dict_index_ = dict_index(vma);

        // retrieve page table
        PageTable*& pt = entries[dict_index_];
        uint32_t& pde = reinterpret_cast<uint32_t&>(pt);
        if (!(pde & PAGE_PRESENT)) {
            set_page_table(pt, alloc_table());
        }
        real_page_table(pt)->insert(vma, pma);
    }

    void register_dictionary() const {
        uint32_t addr = reinterpret_cast<uint32_t>(&entries);
        addr = PMA(addr);
        asm volatile("mov %0, %%cr3;" : : "r"(addr));
    }

private:
    /**
     * set page dictionary entry.
     * we need to set limits bits and convert it to physical address.
     * @return physical address with limits bits
     */
    uint32_t set_pde(uint32_t& pde, uint32_t raw) {
        pde = PMA(raw & ENTRY_ADDR) | PAGE_PRESENT | PAGE_WRITE;
        return pde;
    }

    PageTable* set_page_table(PageTable*& table, PageTable* raw) {
        uint32_t& pde = reinterpret_cast<uint32_t&>(table);
        uint32_t raw_ = reinterpret_cast<uint32_t>(raw);
        return reinterpret_cast<PageTable*>(set_pde(pde, raw_));
    }

    /**
     * Since the pt pointer is not a virtual address which represent the PageTable object,
     * we need to restore it to the real address.
     */
    PageTable* real_page_table(PageTable* pt) const {
        uint32_t pde = reinterpret_cast<uint32_t>(pt);
        return reinterpret_cast<PageTable*>(VMA(ENTRY_ADDR & pde));
    }

    /**
     * Allocate a page table
     */
    PageTable* alloc_table() {
        assert(table_count < TABLE_RESERVE, "no page table available");
        PageTable* addr = &reserve[table_count++];
        return addr;
    }

    uint32_t dict_index(uint32_t addr) const { return (addr >> 22) & 0x3FF; }

    /* data members */
    PageTable* entries[DICTIONARY_MAX_SIZE] = {0};
    PageTable reserve[TABLE_RESERVE] ;
    uint32_t table_count = 0;
};


#endif
