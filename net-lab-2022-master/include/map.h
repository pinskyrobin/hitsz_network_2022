#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "config.h"

typedef void (*map_constuctor_t)(void *dst, const void *src, size_t len);
typedef void (*map_entry_handler_t)(void *key, void *value, time_t *timestamp);

typedef struct map //协议栈的通用泛型map，即键值对的容器，支持超时时间与非平凡值类型
{
    size_t key_len;                    //键的长度
    size_t value_len;                  //值的长度
    size_t size;                       //当前大小
    size_t max_size;                   //最大容量
    time_t timeout;                    //超时时间，0为永不超时
    map_constuctor_t value_constuctor; //形如memcpy的值构造函数，用于拷贝非平凡数据结构到容器中，如buf_copy
    uint8_t data[MAP_MAX_LEN];         //数据
} map_t;

void map_init(map_t *map, size_t key_len, size_t value_len, size_t max_len, time_t timeout, map_constuctor_t value_constuctor);
size_t map_size(map_t *map);
void *map_get(map_t *map, const void *key);
int map_set(map_t *map, const void *key, const void *value);
void map_delete(map_t *map, const void *key);
void map_foreach(map_t *map, map_entry_handler_t handler);

#endif