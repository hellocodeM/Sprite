#include "fs.h"
#include "ide.h"
#include "string.h"
#include "printk.h"
#include "debug.h"
#include "kmalloc.h"

d_inode inode_table[kNumInodes];

int read_zones(uint32_t zno, void* dst, uint32_t count) {
    return read_blocks(zone_to_block(zno), dst, zone_to_block(count));
}

int write_zones(uint32_t zno, const void* src, uint32_t count) {
    return write_blocks(zone_to_block(zno), src, zone_to_block(count));
}

int read_blocks(uint32_t bno, void* dst, uint32_t count) {
    return ide_read_secs(block_to_sector(bno), dst, block_to_sector(count));
}

int write_blocks(uint32_t bno, const void* src, uint32_t count) {
    return ide_write_secs(block_to_sector(bno), src, block_to_sector(count));
}

void init_fs() {}

void test_fs() {
    {
        // test super block
        super_block sb;

        if (read_super_block(&sb) == 0) {
            printk("number of inodes: %d\n", (uint32_t)sb.num_inodes);
            printk("number of zones:  %d\n", (uint32_t)sb.num_zones);
            printk("number of inode bitmap blocks: %d\n", (uint32_t)sb.num_imap_blocks);
            printk("number of zone bitmap blocks: %d\n", (uint32_t)sb.num_zmap_blocks);
            printk("first data zone: %d\n", (uint32_t)sb.first_data_zone);
            printk("log zone size: %d\n", (uint32_t)sb.log_zone_size);
            printk("max file size: %dbytes\n", (uint32_t)sb.max_size);
            printk("magic number: 0x%x\n", (uint32_t)sb.magic);
            printk("-------------------------------------------\n");
        } else {
            assert(false, "read failed");
        }
    }

    {
        // test open and read a file
        m_inode root;
        
        assert(read_root_inode(&root) == 0, "read root inode");
        printk("root inode size: %d; ", root.size);
        printk("root inode mode: %x; ", root.mode);
        if (is_directory(root.mode)) {
            printk("is directory\n");
            uint8_t buff[kBlockSize];
            read_blocks(root.zone[1], buff, 1);
            dir_entry* entry = reinterpret_cast<dir_entry*>(buff);
            dir_entry* end = entry + root.size / sizeof(dir_entry);
            for (; entry != end; entry++) {
                printk(entry->name);
                printk("\n");
                if (strcmp(entry->name, "test.txt") == 0) {
                    printk("context of test.txt:\n");
                    d_inode* node = read_inode(entry->inode);
                    read_blocks(node->zone[0], buff, 1);
                    printk(buff);
                }
            }
        }
        printk("\n------------------------------------------------\n");
    }
}
