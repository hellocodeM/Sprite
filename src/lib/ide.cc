#include "types.h"
#include "common.h"
#include "debug.h"
#include "block_dev.h"
#include "ide.h"

/*
// IDE 设备结构
block_dev_t ide_main_dev = {
        .name = "IDE_MAIN",
        .block_size = SECTSIZE,
        .ops = {
                .init = &ide_init,
                .device_valid = &ide_device_valid,
                .get_desc = &ide_get_desc,
                .get_nr_block = &ide_get_nr_block,
                .request = &ide_request,
                .ioctl = ide_ioctl
        }
};
*/

// 等待IDE设备可用
int32_t ide_wait_ready(uint16_t iobase, bool check_error) {
    int r = 0;
    while ((r = inb(iobase + ISA_STATUS)) & STAT_BSY) {
        cpu_hlt();
    }
    if (check_error && (r & (STAT_DF | STAT_ERR)) != 0) {
        return -1;
    }

    return 0;
}

// 获取IDE设备描述
const char *ide_get_desc(void) { return (const char *)(ide_device.desc); }

// 初始化IDE设备
int ide_init(void) {
    ide_wait_ready(IOBASE, 0);

    // 1: 选择要操作的设备
    outb(IOBASE + ISA_SDH, 0xE0);
    ide_wait_ready(IOBASE, 0);

    // 2: 发送IDE信息获取命令
    outb(IOBASE + ISA_COMMAND, IDE_CMD_IDENTIFY);
    ide_wait_ready(IOBASE, 0);

    // 3: 检查设备是否存在
    if (inb(IOBASE + ISA_STATUS) == 0 || ide_wait_ready(IOBASE, 1) != 0) {
        return -1;
    }

    ide_device.valid = 1;

    // 读取IDE设备信息
    uint32_t buffer[128];
    insl(IOBASE + ISA_DATA, buffer, sizeof(buffer) / sizeof(uint32_t));

    uint8_t *ident = (uint8_t *)buffer;
    uint32_t cmdsets = *(uint32_t *)(ident + IDE_IDENT_CMDSETS);
    uint32_t sectors;

    // 检查设备使用48-bits还是28-bits地址
    if (cmdsets & (1 << 26)) {
        sectors = *(uint32_t *)(ident + IDE_IDENT_MAX_LBA_EXT);
    } else {
        sectors = *(uint32_t *)(ident + IDE_IDENT_MAX_LBA);
    }
    ide_device.sets = cmdsets;
    ide_device.size = sectors;

    char *desc = ide_device.desc;
    char *data = (char *)((uint32_t)ident + IDE_IDENT_MODEL);

    // 复制设备描述信息
    int i, length = IDE_DESC_LEN;
    for (i = 0; i < length; i += 2) {
        desc[i] = data[i + 1];
        desc[i + 1] = data[i];
    }
    do {
        desc[i] = '\0';
    } while (i-- > 0 && desc[i] == ' ');

    return 0;
}

// 检测指定IDE设备是否可用
bool ide_device_valid(void) { return ide_device.valid == 1; }

// 获得设备默认块数量
int ide_get_nr_block(void) {
    if (ide_device_valid()) {
        return ide_device.size;
    }

    return 0;
}

// 设备操作请求
int ide_request(io_request_t *req) {
    switch (req->io_type) {
        case IO_READ:
            if (req->bsize < SECTSIZE * req->nsecs) {
                return -1;
            }
            return ide_read_secs(req->secno, req->buffer, req->nsecs);
        case IO_WRITE:
            if (req->bsize < SECTSIZE * req->nsecs) {
                return -1;
            }
            return ide_write_secs(req->secno, req->buffer, req->nsecs);
        default:
            return -1;
    }

    return -1;
}

// 读取指定IDE设备若干扇区
int ide_read_secs(uint32_t secno, void *dst, uint32_t nsecs) {
    assert(ide_device_valid(), "ide device error");
    // assert(nsecs <= MAX_NSECS, "too many nsecs");
    assert(secno < MAX_DISK_NSECS && secno + nsecs <= MAX_DISK_NSECS, "secno error!");

    uint8_t *out = static_cast<uint8_t *>(dst);
    do {
        uint32_t n = min(nsecs, kMaxSectors);
        nsecs -= n;

        ide_wait_ready(IOBASE, 0);

        outb(IOCTRL + ISA_CTRL, 0);
        outb(IOBASE + ISA_SECCNT, n);
        outb(IOBASE + ISA_LBA_LOW, secno & 0xFF);
        outb(IOBASE + ISA_LBA_MID, (secno >> 8) & 0xFF);
        outb(IOBASE + ISA_LBA_HIGH, (secno >> 16) & 0xFF);
        outb(IOBASE + ISA_LBA_TOP, 0xE0 | ((secno >> 24) & 0xF));
        outb(IOBASE + ISA_COMMAND, IDE_CMD_READ);

        int ret = 0;
        for (; n > 0; n--, out += SECTSIZE) {
            if ((ret = ide_wait_ready(IOBASE, 1)) != 0) {
                return ret;
            }
            insl(IOBASE, out, SECTSIZE / sizeof(uint32_t));
        }

        secno += n;
        out += n * SECTSIZE;
    } while (nsecs);

    return 0;
}

// 写入指定IDE设备若干扇区
int ide_write_secs(uint32_t secno, const void *src, uint32_t nsecs) {
    assert(ide_device.valid == 1, "ide device error");
    // assert(nsecs <= MAX_NSECS, "too many nsecs");
    assert(secno < MAX_DISK_NSECS && secno + nsecs <= MAX_DISK_NSECS, "secno error!");

    auto input = static_cast<const uint8_t *>(src);
    do {
        uint32_t n = min(nsecs, kMaxSectors);
        nsecs -= n;
        ide_wait_ready(IOBASE, 0);

        outb(IOCTRL + ISA_CTRL, 0);
        outb(IOBASE + ISA_SECCNT, n);
        outb(IOBASE + ISA_LBA_LOW, secno & 0xFF);
        outb(IOBASE + ISA_LBA_MID, (secno >> 8) & 0xFF);
        outb(IOBASE + ISA_LBA_HIGH, (secno >> 16) & 0xFF);
        outb(IOBASE + ISA_LBA_TOP, 0xE0 | ((secno >> 24) & 0xF));
        outb(IOBASE + ISA_COMMAND, IDE_CMD_WRITE);

        int ret = 0;
        for (; n > 0; n--, input += SECTSIZE) {
            if ((ret = ide_wait_ready(IOBASE, 1)) != 0) {
                return ret;
            }
            outsl(IOBASE, input, SECTSIZE / sizeof(uint32_t));
        }

        secno += n;
        input += n * SECTSIZE;
    } while (nsecs);

    return 0;
}

// IDE设备选项设置
int ide_ioctl(int op, int flag) {
    if (op != 0 && flag != 0) {
        return -1;
    }

    return 0;
}

void test_ide() {
    uint8_t buffer[512];
    const int secno = 1;

    // write
    for (int i = 0; i < 512; i++) {
        buffer[i] = i;
    }
    ide_write_secs(secno, buffer, 1);

    // read
    bzero(buffer, 512);
    ide_read_secs(secno, buffer, 1);

    for (int i = 0; i < 512; i++) {
        printk("%d ", buffer[i]);
        assert(buffer[i] == (i % 256), "ide disk error");
    }
    printk("\npass test: ide\n");
}
