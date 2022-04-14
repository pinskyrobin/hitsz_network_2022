#ifndef ARP_H
#define ARP_H

#include "net.h"

#define ARP_HW_ETHER 0x1 // 以太网
#define ARP_REQUEST 0x1  // ARP请求包
#define ARP_REPLY 0x2    // ARP响应包

#pragma pack(1)
typedef struct arp_pkt
{
    uint16_t hw_type16;              // 硬件类型
    uint16_t pro_type16;             // 协议类型
    uint8_t hw_len;                  // 硬件地址长
    uint8_t pro_len;                 // 协议地址长
    uint16_t opcode16;               // 请求/响应
    uint8_t sender_mac[NET_MAC_LEN]; // 发送包硬件地址
    uint8_t sender_ip[NET_IP_LEN];   // 发送包协议地址
    uint8_t target_mac[NET_MAC_LEN]; // 接收方硬件地址
    uint8_t target_ip[NET_IP_LEN];   // 接收方协议地址
} arp_pkt_t;

#pragma pack()

void arp_init();
void arp_print();
void arp_in(buf_t *buf, uint8_t *src_mac);
void arp_out(buf_t *buf, uint8_t *ip);
void arp_req(uint8_t *target_ip);
void arp_resp(uint8_t *target_ip, uint8_t *target_mac);
#endif