#include "buf.h"
#include <stdio.h>
#include <string.h>
/**
 * @brief 初始化buffer为给定的长度，用于装载数据包
 * 
 * @param buf 要初始化的buffer
 * @param len 数据初始长度
 * @return int 成功为0，失败为-1
 */
int buf_init(buf_t *buf, size_t len)
{
    if (len >= BUF_MAX_LEN / 2)
    {
        fprintf(stderr, "Error in buf_init:%zu\n", len);
        return -1;
    }

    buf->len = len;
    buf->data = buf->payload + BUF_MAX_LEN / 2 - len;
    return 0;
}

/**
 * @brief 为buffer在头部增加一段长度，用于添加协议头
 * 
 * @param buf 要修改的buffer
 * @param len 增加的长度
 * @return int 成功为0，失败为-1
 */
int buf_add_header(buf_t *buf, size_t len)
{
    if (buf->data - len < buf->payload)
    {
        fprintf(stderr, "Error in buf_add_header:%zu+%zu\n", buf->len, len);
        return -1;
    }
    buf->len += len;
    buf->data -= len;
    return 0;
}

/**
 * @brief 为buffer在头部减少一段长度，去除协议头
 * 
 * @param buf 要修改的buffer
 * @param len 减少的长度
 * @return int 成功为0，失败为-1
 */
int buf_remove_header(buf_t *buf, size_t len)
{
    if (buf->len < len)
    {
        fprintf(stderr, "Error in buf_remove_header:%zu-%zu\n", buf->len, len);
        return -1;
    }
    buf->len -= len;
    buf->data += len;
    return 0;
}

/**
 * @brief 为buffer在尾部添加一段长度，填充0
 * 
 * @param buf 要修改的buffer
 * @param len 添加的长度
 * @return int 成功为0，失败为-1
 */
int buf_add_padding(buf_t *buf, size_t len)
{
    if (buf->data + buf->len + len >= buf->payload + BUF_MAX_LEN)
    {
        fprintf(stderr, "Error in buf_add_padding:%zu+%zu\n", buf->len, len);
        return -1;
    }
    memset(buf->data + buf->len, 0, len);
    buf->len += len;
    return 0;
}

/**
 * @brief 为buffer在尾部减少一段长度，去除填充
 * 
 * @param buf 要修改的buffer
 * @param len 减少的长度
 * @return int 成功为0，失败为-1
 */
int buf_remove_padding(buf_t *buf, size_t len)
{
    if (buf->len < len)
    {
        fprintf(stderr, "Error in buf_remove_padding:%zu-%zu\n", buf->len, len);
        return -1;
    }
    buf->len -= len;
    return 0;
}

/**
 * @brief buf拷贝构造函数
 * 
 * @param pdst 目的buffer
 * @param psrc 源buffer
 * @param len 占位用，与memcpy保持形式一致，无意义
 */
void buf_copy(void *pdst, const void *psrc, size_t len)
{
    buf_t *dst = pdst;
    const buf_t *src = psrc;
    buf_init(dst, src->len);
    memcpy(dst->payload, src->payload, BUF_MAX_LEN);
}
