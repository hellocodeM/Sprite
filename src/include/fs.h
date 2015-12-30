#ifndef SPRITE_FS_H
#define SPRITE_FS_H

#include "types.h"

/* forward declaration */
struct task_struct;

constexpr uint32_t kMBRSecno = 1;
constexpr uint32_t kSuperBlockSecno = 2;
constexpr uint32_t kNameLen = 14;
constexpr uint32_t kSuperMagic = 0x137F;
constexpr uint32_t kBlockSize = 1024;
constexpr uint32_t kZoneSize = kBlockSize * 4;
/**
 * File segment.
 * zero segment: kZoneSize * 7
 * first segment: kZoneSize * 512
 * second segment: kZoneSize * 512 * 512
 */
constexpr uint32_t kFileFirstSegment = kZoneSize * 7;
constexpr uint32_t kFileSecondSegment = kZoneFirstSegment + kZoneSize * 512;
constexpr uint32_t kFileMaxSize = kZoneSecondSegment + kZoneSize * 512 * 512;

struct buffer_head {
    char* data;       /* pointer to data block (1024 bytes) */
    uint32_t blocknr; /* block number */
    uint16_t dev;     /* device (0 = free) */
    uint8_t uptodate;
    uint8_t dirt;  /* 0-clean,1-dirty */
    uint8_t count; /* users using this block */
    uint8_t lock;  /* 0 - ok, 1 -locked */
    task_struct* wait;
    buffer_head* prev;
    buffer_head* next;
    buffer_head* prev_free;
    buffer_head* next_free;
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
};

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
};

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
    struct buffer_head* imap[8];
    struct buffer_head* zmap[8];
    uint16_t dev;
    struct m_inode* isup;
    struct m_inode* imount;
    uint32_t time;
    task_struct* wait;
    uint8_t lock;
    uint8_t rd_only;
    uint8_t dirt;
};

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
};

/**
 * file system interface
 */
size_t open(const char* name);
size_t read(size_t inode_no, void* dst, size_t count);

/**
 * inode management
 */
class InodeManager {
public:
    InodeManager(m_inode* node = nullptr);

    static InodeManager find_file(const char* name);

    bool is_directory();
    bool is_file();
    size_t read(size_t pos, void* buff, size_t count);
    dir_entry* list_dir();
    static m_inode* to_ptr(uint16_t inode);
    static m_inode* root_inode();

private:
    static m_inode* search_file(m_inode* root, const char* name);

    /* data memebers */
    m_inode* node_;
};

void init_fs();
void test_fs();
#endif
