#ifndef SPRITE_FS_H
#define SPRITE_FS_H

#include "types.h"

struct super_block {
    uint16_t num_inodes;        // number of inodes
    uint16_t num_zones;         // number of zones
    uint16_t num_imap_blk;     // number of inode bitmap blocks
    uint16_t num_zmap_blk;     // number of zone bitmap blocks
    uint16_t first_data_zone;  // first data zone
    uint16_t log_zone_size;    // log2(zone_size / block_size)
    uint32_t max_size;         // maximum file size
    uint16_t magic;            // magic number
    uint16_t state;            // mount state
} __attribute__((packed));

/* in disk inode */
struct d_inode {
    uint16_t mode;      // file type and RWX mode
    uint16_t uid;       // user ID
    uint32_t size;      // file size
    uint32_t mtime;     // modify time since 1970
    uint8_t gid;        // group id
    uint8_t num_links;  // hard link number
    uint16_t zone[9];   // zone[0] ~ zone[6] direct pointer, zone[7] points to first level index
                        // table, zone[8] second level.
};

#define I_BUSY 0x1
#define I_VALID 0x2
/* in memory inode */
struct inode {
    uint16_t dev;
    uint32_t ino;
    uint16_t ref;
    uint16_t flags;
    uint16_t atime;
    uint16_t ctime;
    // in disk and memory
    uint16_t mode;
    uint16_t uid;
    uint16_t size;
    uint32_t mtime;
    uint8_t gid;
    uint8_t nlinks;
    uint16_t zone[9];
};


void test_fs();
#endif
