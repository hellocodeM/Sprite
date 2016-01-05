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
#define kNumInodesPerBlock (kBlockSize / sizeof(d_inode))

constexpr bool is_regular(uint16_t mode) { return (mode & S_IFMT) == S_IFREG; }

constexpr bool is_directory(uint16_t mode) { return (mode & S_IFMT) == S_IFDIR; }

static constexpr uint32_t block_to_sector(uint32_t bno) { return bno * kBlockSectorFactor; }

static constexpr uint32_t sector_to_block(uint32_t sno) { return sno / kBlockSectorFactor; }

static constexpr uint32_t zone_to_block(uint32_t zone_idx) { return zone_idx * kZoneBlockFactor; }

static constexpr uint32_t block_to_zone(uint32_t block_idx) { return block_idx / kZoneBlockFactor; }

constexpr uint32_t block_addr(uint16_t block_idx) { return (uint32_t)block_idx * kBlockSize; }

struct block_buffer {
    uint8_t data[kBlockSize]; /* block data */
    uint8_t count;            /* users using this block */
    uint8_t lock;             /* 0 - ok, 1 -locked */
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
    uint32_t size;
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

/*---------------------ide layer -------------------------------*/
/**
 * int ide_read_secs(uint32_t secno, void* dst, uint32_t nsecs);
 * int ide_write_secs(uint32_t secon, const void* src, uint32_t nsecs);
 */

/* --------------------block layer  ---------------------------*/
template <class T, size_t N>
class LRUCache;

extern super_block g_super_block;
extern LRUCache<block_buffer, 307> g_block_cache;

/**
 * Get a block for write.
 */
block_buffer* get_block(uint32_t bno);

/**
 * Read a block, maybe from cache instead of disk.
 */
block_buffer* read_block(uint32_t bno);

/**
 * write data to a block, the buffer should come from get_block
 */
void write_block(block_buffer* bb);

/*----------------------- inode layer ----------------------------*/
template <size_t N>
class bitmap;

extern m_inode g_inode_table[kNumInodes];
extern bitmap<128> g_imap;
extern bitmap<10240> g_zmap;

m_inode* find_entry(const m_inode* dir, const char* name);

/*------------------------ fs layer -------------------------------*/

void init_fs();
void test_fs();
#endif
