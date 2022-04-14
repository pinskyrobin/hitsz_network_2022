#ifndef IP_H
#define IP_H

#include "net.h"

#pragma pack(1)
typedef struct ip_hdr
{
    uint8_t hdr_len : 4;        // 首部长, 4字节为单位
    uint8_t version : 4;        // 版本号
    uint8_t tos;                // 服务类型
    uint16_t total_len16;       // 总长度
    uint16_t id16;              // 标识符
    uint16_t flags_fragment16;  // 标志与分段
    uint8_t ttl;                // 存活时间
    uint8_t protocol;           // 上层协议
    uint16_t hdr_checksum16;    // 首部校验和
    uint8_t src_ip[NET_IP_LEN]; // 源IP
    uint8_t dst_ip[NET_IP_LEN]; // 目标IP
} ip_hdr_t;
#pragma pack()

#define IP_HDR_LEN_PER_BYTE 4      //ip包头长度单位
#define IP_HDR_OFFSET_PER_BYTE 8   //ip分片偏移长度单位
#define IP_VERSION_4 4             //ipv4
#define IP_MORE_FRAGMENT (1 << 13) //ip分片mf位
void ip_in(buf_t *buf, uint8_t *src_mac);
void ip_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol);
void ip_init();
#endif