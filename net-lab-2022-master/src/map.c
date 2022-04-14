#include <string.h>
#include "map.h"

/**
 * @brief 初始化map
 * 
 * @param map 要初始化的map
 * @param key_len 键的长度
 * @param value_len 值的长度
 * @param max_size 最大容量，为0则根据MAP_MAX_LEN自动设置
 * @param timeout 超时秒数，为0则永不超时
 * @param value_constuctor 形如memcpy的构造函数，用于拷贝值到容器中，为NULL则使用memcpy
 */
void map_init(map_t *map, size_t key_len, size_t value_len, size_t max_size, time_t timeout, map_constuctor_t value_constuctor)
{
    if (max_size == 0 || max_size * (key_len + value_len + sizeof(time_t)) > MAP_MAX_LEN)
        max_size = MAP_MAX_LEN / (key_len + value_len + sizeof(time_t));
    if (value_constuctor == NULL)
        value_constuctor = (map_constuctor_t)memcpy;

    memset(map, 0, sizeof(map_t));
    map->key_len = key_len;
    map->value_len = value_len;
    map->max_size = max_size;
    map->timeout = timeout;
    map->value_constuctor = value_constuctor;
}

/**
 * @brief 获取map当前大小
 * 
 * @param map 要获取的map
 * @return size_t map大小
 */
size_t map_size(map_t *map)
{
    return map->size;
}

/**
 * @brief 内部函数，获取第n个物理位置的键值对
 * 
 * @param map 要获取的map
 * @param pos 位置
 * @return void* 键值对指针
 */
void *map_entry_get(map_t *map, size_t pos)
{
    if (pos >= map->max_size)
        return NULL;
    return map->data + pos * (map->key_len + map->value_len + sizeof(time_t));
}

/**
 * @brief 内部函数，判断键值对是否有效
 * 
 * @param map 要判断的map
 * @param entry 键值对指针
 * @return int 1为合法，0为不合法
 */
int map_entry_valid(map_t *map, const void *entry)
{
    time_t entry_time = *(time_t *)((uint8_t *)entry + map->key_len + map->value_len);
    return entry_time && (!map->timeout || entry_time + map->timeout >= time(NULL));
}

/**
 * @brief 获取map中指定键的值
 * 
 * @param map 要获取的map
 * @param key 键指针
 * @return void* 值指针，找不到为NULL 
 */
void *map_get(map_t *map, const void *key)
{
    if (key == NULL)
        return NULL;
    for (size_t i = 0; i < map->max_size; i++)
    {
        uint8_t *entry = map_entry_get(map, i);
        if (map_entry_valid(map, entry) && !memcmp(key, entry, map->key_len))
            return entry + map->key_len;
    }
    return NULL;
}

/**
 * @brief 插入或更新map中指定键的值
 * 
 * @param map 要操作的map
 * @param key 键指针
 * @param value 值指针
 * @return int 成功为0，失败为-1
*/
int map_set(map_t *map, const void *key, const void *value)
{
    uint8_t *old_value = map_get(map, key);
    if (old_value)
    {
        map->value_constuctor(old_value, value, map->value_len);
        *(time_t *)(old_value + map->value_len) = time(NULL);
        return 0;
    }
    if (map->size == map->max_size)
        return -1;
    for (size_t i = 0; i < map->max_size; i++)
    {
        uint8_t *entry = map_entry_get(map, i);
        if (!map_entry_valid(map, entry))
        {
            memcpy(entry, key, map->key_len);
            map->value_constuctor(entry + map->key_len, value, map->value_len);
            *(time_t *)(entry + map->key_len + map->value_len) = time(NULL);
            map->size++;
            return 0;
        }
    }

    return -1;
}

/**
 * @brief 删除map中指定的键
 * 
 * @param map 要操作的map
 * @param key 键指针
 */
void map_delete(map_t *map, const void *key)
{
    uint8_t *value = map_get(map, key);
    if (value)
    {
        *(time_t *)(value + map->value_len) = 0;
        map->size--;
    }
}

/**
 * @brief 遍历map
 * 
 * @param map 要遍历的map
 * @param handler 对每个键值对应用的回调函数，参数为（键指针，值指针，更新时间指针）
 */
void map_foreach(map_t *map, map_entry_handler_t handler)
{
    for (size_t i = 0; i < map->max_size; i++)
    {
        uint8_t *entry = map_entry_get(map, i);
        if (map_entry_valid(map, entry))
            handler(entry, entry + map->key_len, (time_t *)(entry + map->key_len + map->value_len));
    }
}
