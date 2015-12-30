#include "fs.h"
#include "ide.h"
#include "string.h"
#include "printk.h"
#include "debug.h"

InodeManager::InodeManager(m_inode* node): node_(node) {}

size_t InodeManager::read(size_t pos, void* buff, count) {
    uint8_t sector_buff[kSectorSize];

    if (pos <= kZoneFirstLevel) {
        uint32_t secno = pos / kSectorSize;
        ide_read_secs(secno, sectr_buff, )
    } else if (pos <= kZoneSize * kZoneSecondLevel) {
        
    } else if (pos <= kFileMaxSize) {

    } 
    return 0;
}

static InodeManager InodeManager::find_file(const char* name) {
    m_inode* inode = search_file(root_inode(), name);
    return InodeManager(inode);
}

dir_entry* InodeManager::list_dir() {
    assert(is_directory());
}

static m_inode* InodeManager::search_file(m_inode* root, const char* name) {
    dir_entry* entry = list_entry();
    
    for (; entry ; ++entry) {
        if (strcmp(entry->name, name) == 0) {
            return to_ptr(entry->inode);
        }
    }
    return nullptr;
}

void init_fs() {

}

void test_fs() {
    {
        // test super block
        uint8_t buff[kSectorSize];
        d_super_block sb;

        ide_read_secs(kSuperBlockSecno, buff, 1);
        memcpy(&sb, buff, sizeof(sb));
        printk("number of inodes: %d\n", (uint32_t)sb.num_inodes);
        printk("number of zones:  %d\n", (uint32_t)sb.num_zones);
        printk("number of inode bitmap blocks: %d\n", (uint32_t)sb.num_imap_blocks);
        printk("number of zone bitmap blocks: %d\n", (uint32_t)sb.num_zmap_blocks);
        printk("first data zone: %d\n", (uint32_t)sb.first_data_zone);
        printk("log zone size: %d\n", (uint32_t)sb.log_zone_size);
        printk("max file size: %dbytes\n", (uint32_t)sb.max_size);
        printk("magic number: 0x%x\n", (uint32_t)sb.magic);
    }

    {
        // test open and read a file
        char buff[512];
        const char* file = "/test.txt";

        InodeManager im = InodeManager::find_file(file);
        if (!im.empty()) {
            im.read(buff, sizeof(buff));
            printk(buff);
        }
    }
}
