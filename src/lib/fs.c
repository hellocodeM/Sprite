#include "fs.h"
#include "ide.h"
#include "string.h"
#include "printk.h"

void test_fs() {
    uint8_t buff[1024];
    super_block sb;

    ide_read_secs(1, buff, 1);
    memcpy(&sb, buff, sizeof(sb));
    printk("number of inodes: %d\n", (uint32_t)sb.num_inodes);
    printk("number of zones:  %d\n", (uint32_t)sb.num_zones);
}


