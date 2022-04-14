#ifndef ETHERNET_H
#define ETHERNET_H

#include "net.h"

#define ETHERNET_MIN_TRANSPORT_UNIT 46 //以太网最小传输单元

#pragma pack(1)

typedef struct ether_hdr
{
    uint8_t dst[NET_MAC_LEN]; // 目标mac地址
    uint8_t src[NET_MAC_LEN]; // 源mac地址
    uint16_t protocol16;      // 协议/长度
} ether_hdr_t;
#pragma pack()
void ethernet_init();
void ethernet_in(buf_t *buf);
void ethernet_out(buf_t *buf, const uint8_t *mac, net_protocol_t protocol);
void ethernet_poll();
static const uint8_t ether_broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //以太网广播mac地址
#endif