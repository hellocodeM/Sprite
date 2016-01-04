#include "fs.h"
#include "ide.h"
#include "string.h"
#include "printk.h"
#include "debug.h"
#include "kmalloc.h"
#include "lrucache.hpp"

// global variables
m_inode g_inode_table[kNumInodes];
LRUCache<block_buffer, 307> g_block_cache;
super_block g_super_block;

/**
 * Block operations
 */
block_buffer* get_block(uint32_t bno) {
    auto buff = g_block_cache.Get(bno);
    if (!buff) {
        buff = g_block_cache.Alloc(bno);
    }
    return buff;
}

block_buffer* read_block(uint32_t bno) {
    // the data in cache is always uptodate, so we could directly return it
    auto buff = g_block_cache.Get(bno);
    if (!buff) {
        buff = g_block_cache.Alloc(bno);
        ide_read_secs(block_to_sector(bno), buff, block_to_sector(1));
    }
    return buff;
}

void write_block(uint32_t bno) {
    auto buff = g_block_cache.Get(bno);
    ide_write_secs(block_to_sector(bno), buff, block_to_sector(1));
}

/* inode operations */

/**
 * Find an entry in a dir, it could be a directory or a regular file.
 */
m_inode* find_entry(const m_inode* dir, const char* name) {
    assert(is_directory(dir->mode), "is not a dir");
    auto entry = reinterpret_cast<dir_entry*>(read_block(dir->zone[0]));
    auto end = entry + dir->size / sizeof(dir_entry);
    for (; entry != end; ++entry) {
        if (strcmp(entry->name, name) == 0) return &g_inode_table[entry->inode];
    }
    return nullptr;
}

/* FS operations */

void init_fs() {
    // init super block
    memcpy(&g_super_block, read_block(kSuperBlockBNO), sizeof(g_super_block));

    // init inode table
    int block_start = 2 + g_super_block.num_imap_blocks + g_super_block.num_zmap_blocks;
    int num_blocks = (g_super_block.num_inodes * sizeof(d_inode) + kBlockSize) / kBlockSize - 1;

    int inode_cnt = 1;
    for (uint32_t i = block_start; i < block_start + num_blocks; i++) {
        d_inode* inodes = reinterpret_cast<d_inode*>(read_block(i));
        for (int j = 0, end = kNumInodesPerBlock; j < end; j++) {
            memcpy(&g_inode_table[inode_cnt], inodes + j, sizeof(d_inode));
            ++inode_cnt;
        }
    }
}

void test_fs() {
    {
        // test super block
        super_block& sb = g_super_block;
        printk("number of inodes: %d\n", (uint32_t)sb.num_inodes);
        printk("number of zones:  %d\n", (uint32_t)sb.num_zones);
        printk("number of inode bitmap blocks: %d\n", (uint32_t)sb.num_imap_blocks);
        printk("number of zone bitmap blocks: %d\n", (uint32_t)sb.num_zmap_blocks);
        printk("first data zone: %d\n", (uint32_t)sb.first_data_zone);
        printk("log zone size: %d\n", (uint32_t)sb.log_zone_size);
        printk("max file size: %dbytes\n", (uint32_t)sb.max_size);
        printk("magic number: 0x%x\n", (uint32_t)sb.magic);
        printk("-------------------------------------------\n");
    }

    {
        // test open and read a file
        m_inode& root = g_inode_table[kRootINO];

        auto node = find_entry(&root, "test.txt");
        auto contents = (char*)(read_block(node->zone[0]));
        printk("contents of test.txt\n");
        printk(contents);
    }
}
