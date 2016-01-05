#include "fs.h"
#include "ide.h"
#include "string.h"
#include "printk.h"
#include "debug.h"
#include "kmalloc.h"
#include "lrucache.hpp"
#include "bitmap.hpp"

// global variables
m_inode g_inode_table[kNumInodes];
LRUCache<block_buffer, 307> g_block_cache;
super_block g_super_block;
bitmap<128> g_imap;
bitmap<10240> g_zmap;

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

/**
 * Callback for cache
 */
static void uncache(size_t bno, block_buffer* buff) { write_block(bno); }

/* inode operations */

/**
 * Read data zone of a inode.
 * param block: number of zone
 */
block_buffer* read_inode(const m_inode* inode, uint32_t zone) {}

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

/**
 * Find the inode for the specific name
 */
m_inode* namei(const char* name) {}

/* FS operations */

void init_fs() {
    // init super block
    memcpy(&g_super_block, read_block(kSuperBlockBNO), sizeof(g_super_block));

    // init block cache
    g_block_cache.SetUncacheCallback(&uncache);

    // init inode table
    int block_start = 2 + g_super_block.num_imap_blocks + g_super_block.num_zmap_blocks;
    int num_blocks = (g_super_block.num_inodes * sizeof(d_inode) - 1) / kBlockSize + 1;

    int inode_cnt = 1;
    for (uint32_t i = block_start; i < block_start + num_blocks; i++) {
        d_inode* inodes = reinterpret_cast<d_inode*>(read_block(i));
        for (int j = 0, end = kNumInodesPerBlock; j < end; j++) {
            memcpy(&g_inode_table[inode_cnt], inodes + j, sizeof(d_inode));
            ++inode_cnt;
        }
    }

    // init bitmap
    int imap_start = 2;
    int imap_last = imap_start + g_super_block.num_imap_blocks - 1;
    int byte_cnt = 0;
    for (int i = imap_start; i <= imap_last; i++) {
        auto block = read_block(i);
        if (i < imap_last) {
            memcpy(g_imap.data_+byte_cnt, block, kBlockSize);
            byte_cnt += kBlockSize;
        } else {
            memcpy(g_imap.data_+byte_cnt, block, g_super_block.num_inodes / 8 - byte_cnt);
        }
    }
    

    int zmap_start = 2 + g_super_block.num_imap_blocks;
    int zmap_last = zmap_start + g_super_block.num_zmap_blocks - 1;
    byte_cnt = 0;

    for (int i = zmap_start; i <= zmap_last; i++) {
        auto block = read_block(i);
        if (i < zmap_last) {
            memcpy(g_zmap.data_ + byte_cnt, block, kBlockSize);
            byte_cnt += kBlockSize;
        } else {
            memcpy(g_zmap.data_ + byte_cnt, block, g_super_block.num_zones / 8 - byte_cnt);
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
