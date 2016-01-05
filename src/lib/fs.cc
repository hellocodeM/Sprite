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

bool write_block(uint32_t bno) {
    if (auto buff = g_block_cache.Get(bno)) {
        ide_write_secs(block_to_sector(bno), buff, block_to_sector(1));
        return true;
    }
    return false;
}

/**
 * Read blocks from disk.
 * The destination memory address is continuous, but address on disk is not, it is fragmented into
 * blocks.
 * So we need to read the blocks one by one.
 *
 * param Input : source type
 * param Output: destination type
 * param start_bon: source block number
 * param dst: destination address
 * param count: number of items to copy
 * param size: size of each item
 */
template <class Input, class Output>
bool read_blocks(size_t start_bno, void* dst, size_t count, size_t size) {
    auto block = reinterpret_cast<Input*>(read_block(start_bno));
    auto src = block;
    auto dst_ = reinterpret_cast<Output*>(dst);

    for (int i = 0; i < count; i++, dst_++, src++) {
        if ((uint8_t*)src >= (uint8_t*)block + kBlockSize) {
            src = block = reinterpret_cast<Input*>(read_block(++start_bno));
            if (!src) return false;
        }
        memcpy(dst_, src, size);
    }
    return true;
}

template <class Input, class Output>
bool write_blocks(void* src, size_t output_bno, size_t count, size_t size) {
    auto block = reinterpret_cast<Output*>(get_block(output_bno));
    auto dst = block;
    auto src_ = reinterpret_cast<Input*>(src);

    for (int i = 0; i < count; i++, dst++, src_++) {
        if ((uint8_t*)dst >= (uint8_t*)block + kBlockSize) {
            write_block(output_bno++);
            block = dst = reinterpret_cast<Output*>(get_block(output_bno));
            if (!block) return false;
        }
        memcpy(dst, src_, size);
    }
    write_block(output_bno);
    return true;
}

/**
 * Callback for cache
 */
static void uncache(size_t bno, block_buffer* buff) { write_block(bno); }

/**
 * Synchronization data to disk.
 */
void sync_super_block() {
    auto block = get_block(kSuperBlockBNO);
    memcpy(block, &g_super_block, sizeof(d_super_block));
    write_block(kSuperBlockBNO);
}

void sync_imap() {
    int imap_start = 2;
    int count = g_super_block.num_inodes / 8;
    write_blocks<uint8_t, uint8_t>(g_imap.data_, imap_start, count, sizeof(uint8_t));
}

void sync_zmap() {
    int zmap_start = 2 + g_super_block.num_imap_blocks;
    int count = g_super_block.num_zones / 8;
    write_blocks<uint8_t, uint8_t>(g_zmap.data_, zmap_start, count, sizeof(uint8_t));
}

void sync_inodes() {
    int inode_start = 2 + g_super_block.num_imap_blocks + g_super_block.num_zmap_blocks;
    write_blocks<m_inode, d_inode>(g_inode_table + 1, inode_start, g_super_block.num_inodes,
                                   sizeof(d_inode));
}

void sync_all() {
    sync_super_block();
    sync_imap();
    sync_zmap();
    sync_inodes();
}

/* inode operations */

/**
 * Read data zone of a inode.
 * param block: number of zone
 */
block_buffer* read_zone(const m_inode* inode, uint32_t zone) {
    if (zone < 7) {
        return read_block(inode->zone[zone]);
    } else if (zone < 7 + 512) {
        zone -= 7;
        auto zones = reinterpret_cast<uint16_t*>(read_block(inode->zone[7]));
        return read_block(zones[zone]);
    } else {
        zone -= 7 + 512;
        auto zone_table = reinterpret_cast<uint16_t*>(read_block(inode->zone[8]));
        auto zones = reinterpret_cast<uint16_t*>(read_block(zone_table[zone / 512]));
        return read_block(zones[zone % 512]);
    }
}

/**
 * Traverse a directory, with a callback.
 * The callback takes in an entry of directory, and return a bool value, if return true, the
 * traversal will stop.
 */
template <class Fn>
m_inode* traverse_dir(const m_inode* dir, Fn cb) {
    if (!is_directory(dir->mode)) return nullptr;

    int zone_cnt = 0;
    auto zone = reinterpret_cast<dir_entry*>(read_zone(dir, zone_cnt));
    auto entry = zone;
    int num_entries = dir->size / sizeof(dir_entry);

    for (int i = 0; i < num_entries; i++, ++entry) {
        if ((uint8_t*)entry >= (uint8_t*)zone + kZoneSize) {
            zone = entry = reinterpret_cast<dir_entry*>(read_zone(dir, ++zone_cnt));
        }
        if (cb(entry)) return g_inode_table + entry->inode;
    }
    return nullptr;
}

/**
 * Find an entry in a dir, it could be a directory or a regular file.
 */
m_inode* find_entry(const m_inode* dir, const char* name) {
    if (!is_directory(dir->mode)) return nullptr;

    return traverse_dir(dir, [name](dir_entry* entry) { return strcmp(entry->name, name) == 0; });
}

/**
 * Find the inode for the specific name
 */
m_inode* namei(const char* path) {
    assert(path[0] == '/', "path must start with /");
    m_inode* inode = g_inode_table + kRootINO;
    const char* delim = path;

    while (true) {
        path = delim + 1;
        delim = strfind(path, '/');
        inode = find_entry(inode, path);
        if (!*delim)  // reach the last entry
            break;
    }
    return inode;
}

/* FS operations */

void init_fs() {
    // init super block
    memcpy(&g_super_block, read_block(kSuperBlockBNO), sizeof(g_super_block));
    // init block cache
    g_block_cache.SetUncacheCallback(&uncache);
    // init inode table
    int block_start = 2 + g_super_block.num_imap_blocks + g_super_block.num_zmap_blocks;
    read_blocks<d_inode, m_inode>(block_start, g_inode_table + 1, g_super_block.num_inodes,
                                  sizeof(d_inode));
    // init bitmap
    int imap_start = 2;
    read_blocks<uint8_t, uint8_t>(imap_start, g_imap.data_, g_super_block.num_inodes / 8,
                                  sizeof(uint8_t));
    int zmap_start = 2 + g_super_block.num_imap_blocks;
    read_blocks<uint8_t, uint8_t>(zmap_start, g_zmap.data_, g_super_block.num_zones / 8,
                                  sizeof(uint8_t));
}

void list_entries(const m_inode* inode) {
    if (is_directory(inode->mode)) {
        traverse_dir(inode, [&](dir_entry* entry) {
            auto node = g_inode_table + entry->inode;
            if (entry->name[0] != '.') {
                printk("%s\t%d\t%s\n", entry->name, node->size, is_directory(node->mode) ? "dir" : "file");
                list_entries(node);
            }
            return false;
        });
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
        printk("files in /\nname\tsize\ttype\n");
        list_entries(g_inode_table + kRootINO);

        if (auto node = namei("/test/test.txt")) {
            auto contents = reinterpret_cast<char*>(read_zone(node, 0));
            printk("size: %d; contents of test.txt: ", node->size);
            printk(contents);
            printk("\n");

            memcpy(contents, "hello world", 12);
            node->size = 12;
            write_block(node->zone[0]);
        }
        sync_all();
    }
}
