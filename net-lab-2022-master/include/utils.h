#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <time.h>

uint16_t checksum16(uint16_t *data, size_t len);
#define swap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF)) //为16位数据交换大小端

char *iptos(uint8_t *ip);
char *mactos(uint8_t *mac);
char *timetos(time_t timestamp);
uint8_t ip_prefix_match(uint8_t *ipa, uint8_t *ipb);
#endif