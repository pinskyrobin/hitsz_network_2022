#ifndef BUF_H
#define BUF_H

#include <stdlib.h>
#include <stdint.h>
#include "config.h"

typedef struct buf //协议栈的通用数据包buffer, 可以在头部装卸数据，以供协议头的添加和去除
{
    size_t len;                   // 包中有效数据大小
    uint8_t *data;                // 包的数据起始地址
    uint8_t payload[BUF_MAX_LEN]; // 最大负载数据量
} buf_t;

int buf_init(buf_t *buf, size_t len);
int buf_add_header(buf_t *buf, size_t len);
int buf_remove_header(buf_t *buf, size_t len);
int buf_add_padding(buf_t *buf, size_t len);
int buf_remove_padding(buf_t *buf, size_t len);
void buf_copy(void *pdst, const void *psrc, size_t len);

#endif