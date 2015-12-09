#ifndef BLOCK_DEV_H
#define BLOCK_DEV_H

#include "types.h"


enum io_type_t { IO_READ = 0, IO_WRITE = 1 };

struct io_request_t {
    io_type_t io_type;  // IO操作类型
    uint32_t secno;     // 起始位置
    uint32_t nsecs;     // 读写数量
    void *buffer;       // 读写缓冲区
    uint32_t bsize;     // 缓冲区尺寸
} ;

#endif
