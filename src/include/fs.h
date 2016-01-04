#ifndef SPRITE_FS_H
#define SPRITE_FS_H

#include "types.h"
#include "string.h"
#include "new.h"
#include "common.h"

/* forward declaration */
struct task_struct;

#define S_IFMT 00170000
#define S_IFREG 0100000
#define S_IFBLK 0060000
#define S_IFDIR 0040000

constexpr uint32_t kMBRBNO = 0;         // Master Boot Record Block NO
constexpr uint32_t kSuperBlockBNO = 1;  // Super Block NO
constexpr uint32_t kRootINO = 1;        // root inode NO
constexpr uint32_t kNameLen = 14;       // maximum length of file name
constexpr uint32_t kSuperMagic = 0x137F;
constexpr uint32_t kBlockSectorFactor = 2;
constexpr uint32_t kBlockSize = 1024;
constexpr uint32_t kZoneBlockFactor = 1;
constexpr uint32_t kZoneSize = kBlockSize * kZoneBlockFactor;
constexpr uint32_t kNumInodes = 128;

constexpr bool is_regular(uint16_t mode) { return (mode & S_IFMT) == S_IFREG; }

constexpr bool is_directory(uint16_t mode) { return (mode & S_IFMT) == S_IFDIR; }

static constexpr uint32_t block_to_sector(uint32_t bno) { return bno * kBlockSectorFactor; }

static constexpr uint32_t sector_to_block(uint32_t sno) { return sno / kBlockSectorFactor; }

static constexpr uint32_t zone_to_block(uint32_t zone_idx) { return zone_idx * kZoneBlockFactor; }

static constexpr uint32_t block_to_zone(uint32_t block_idx) { return block_idx / kZoneBlockFactor; }

constexpr uint32_t block_addr(uint16_t block_idx) { return (uint32_t)block_idx * kBlockSize; }

/**
 * File segment.
 * zero segment: kZoneSize * 7
 * first segment: kZoneSize * 512
 * second segment: kZoneSize * 512 * 512
 */
constexpr uint32_t kFileFirstSegment = kZoneSize * 7;
constexpr uint32_t kFileSecondSegment = kFileFirstSegment + kZoneSize * 512;
constexpr uint32_t kFileMaxSize = kFileSecondSegment + kZoneSize * 512 * 512;

struct block_buffer {
    uint8_t* data;    /* pointer to data block (1024 bytes) */
    uint32_t blocknr; /* block number */
    uint16_t dev;     /* device (0 = free) */
    uint8_t uptodate;
    uint8_t dirt;  /* 0-clean,1-dirty */
    uint8_t count; /* users using this block */
    uint8_t lock;  /* 0 - ok, 1 -locked */
    task_struct* wait;

    block_buffer() = default;

    block_buffer(const block_buffer& other) {
        memcpy(this, &other, sizeof(other));
        data = new uint8_t[kBlockSize];
        memcpy(data, other.data, kBlockSize);
    }

    block_buffer& operator= (const block_buffer& other) {
        memcpy(this, &other,sizeof(other));
        data = new uint8_t[kBlockSize];
        memcpy(data, other.data, kBlockSize);
        return *this;
    }

    ~block_buffer() {
        delete data;
    }
};

/**
 * in disk inode
 */
struct d_inode {
    uint16_t mode;      // file type and RWX mode
    uint16_t uid;       // user ID
    uint32_t size;      // file size
    uint32_t mtime;     // modify time since 1970
    uint8_t gid;        // group id
    uint8_t num_links;  // hard link number
    uint16_t zone[9];   // zone[0] ~ zone[6] direct pointer, zone[7] points to first level index
                        // table, zone[8] second level.
} __attribute__((packed));

/**
 * in memory inode
 */
struct m_inode {
    uint16_t mode;
    uint16_t uid;
    uint16_t size;
    uint32_t mtime;
    uint8_t gid;
    uint8_t num_links;
    uint16_t zone[9];

    task_struct* wait;
    uint32_t atime;
    uint32_t ctime;
    uint16_t dev;
    uint16_t num;
    uint16_t count;
    uint8_t lock;
    uint8_t dir;
    uint8_t pipe;
    uint8_t mount;
    uint8_t seek;
    uint8_t update;
} __attribute__((packed));

struct file {
    uint16_t mode;
    uint16_t flags;
    uint16_t count;
    m_inode* inode;
    uint16_t pos;
};

struct super_block {
    uint16_t num_inodes;
    uint16_t num_zones;
    uint16_t num_imap_blocks;
    uint16_t num_zmap_blocks;
    uint16_t first_data_zone;
    uint16_t log_zone_size;
    uint32_t max_size;
    uint16_t magic;
    /* These are only in memory */
    block_buffer* imap[8];
    block_buffer* zmap[8];
    uint16_t dev;
    m_inode* isup;
    m_inode* imount;
    uint32_t time;
    task_struct* wait;
    uint8_t lock;
    uint8_t rd_only;
    uint8_t dirt;
} __attribute__((packed));

struct d_super_block {
    uint16_t num_inodes;
    uint16_t num_zones;
    uint16_t num_imap_blocks;
    uint16_t num_zmap_blocks;
    uint16_t first_data_zone;
    uint16_t log_zone_size;
    uint32_t max_size;
    uint16_t magic;
};

struct dir_entry {
    uint16_t inode;
    char name[kNameLen];
} __attribute__((packed));

/*
 * ide layer
 *
 * int ide_read_secs(uint32_t secno, void* dst, uint32_t nsecs);
 * int ide_write_secs(uint32_t secon, const void* src, uint32_t nsecs);
 */

/* block layer */
int read_blocks(uint32_t bno, void* dst, uint32_t count);
int write_blocks(uint32_t bno, const void* src, uint32_t count);

inline int read_super_block(super_block* sb) {
    uint8_t buff[kBlockSize];
    if (int res = read_blocks(kSuperBlockBNO, buff, 1)) return res;
    memcpy(sb, buff, sizeof(super_block));
    return 0;
}

/* zone layer */
int read_zones(uint32_t zno, void* dst, uint32_t count);
int write_zones(uint32_t zno, const void* src, uint32_t count);

/* inode layer */
template <class T, size_t N>
class LRUCache;
extern d_inode g_inode_table[kNumInodes];
extern LRUCache<block_buffer, 307> g_block_cache;

static inline uint32_t inode_zone_bno() {
    super_block sb;
    read_super_block(&sb);
    return 2 + sb.num_imap_blocks + sb.num_zmap_blocks;
}

static int read_root_inode(m_inode* root) {
    uint32_t root_bno = inode_zone_bno();
    uint8_t buff[kBlockSize];

    if (int res = read_blocks(root_bno, buff, kRootINO)) return res;
    memcpy(root, buff, sizeof(m_inode));
    return 0;
}

static d_inode* read_inode(uint16_t ino) {
    super_block sb;
    read_super_block(&sb);
    int num_blocks = sb.num_inodes * sizeof(d_inode) / kBlockSize;
    read_blocks(inode_zone_bno(), g_inode_table, num_blocks);

    return g_inode_table + ino - 1;
}

/* file layer */

void init_fs();
void test_fs();
#endif
