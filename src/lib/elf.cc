#include "common.h"
#include "string.h"
#include "elf.h"
#include "vmm.h"

// 从 multiboot_t 结构获取ELF信息
elf_t elf_from_multiboot(multiboot_t *mb) {
    elf_t elf;
    elf_section_header_t *sh = (elf_section_header_t *)mb->addr;

    // section header string table
    auto shstrtab = reinterpret_cast<const char*>(sh[mb->shndx].addr);
    for (uint32_t i = 0; i < mb->num; i++) {
        const char *name = VMA(shstrtab + sh[i].name);
        // 在 GRUB 提供的 multiboot 信息中寻找
        // 内核 ELF 格式所提取的字符串表和符号表
        if (strcmp(name, ".strtab") == 0) {
            elf.strtab = (const char *)sh[i].addr;
            elf.strtabsz = sh[i].size;
        }
        if (strcmp(name, ".symtab") == 0) {
            elf.symtab = (elf_symbol_t *)sh[i].addr;
            elf.symtabsz = sh[i].size;
        }
    }

    return elf;
}

// 查看ELF的符号信息
const char *elf_lookup_symbol(uint32_t addr, elf_t *elf) {
    for (uint32_t i = 0; i < (elf->symtabsz / sizeof(elf_symbol_t)); i++) {
        elf_symbol_t& symbol = elf->symtab[i];
        if (ELF32_ST_TYPE(symbol.info) != 0x2) {
            continue;
        }
        // 通过函数调用地址查到函数的名字
        if (symbol.value <= addr && addr < symbol.value + symbol.size) {
            return (elf->strtab + symbol.name);
        }
    }

    return nullptr;
}
