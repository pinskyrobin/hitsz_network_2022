#include "net.h"
#include "ip.h"
#include "ethernet.h"
#include "arp.h"
#include "icmp.h"

/**
 * @brief 处理一个收到的数据包
 * 
 * @param buf 要处理的数据包
 * @param src_mac 源mac地址
 */
void ip_in(buf_t *buf, uint8_t *src_mac)
{
    ip_hdr_t *ip_hdr = (ip_hdr_t *)buf->data;

    // 检查数据包的长度和IP头部长度
    if (buf->len < sizeof(ip_hdr_t))
        return;
    
    // 报头检测
    if (ip_hdr->version != IP_VERSION_4 || 
        swap16(ip_hdr->total_len16) > buf->len || 
        ip_hdr->hdr_len < 5)
        return;

    // 校验和检测
    // 是否需要大小端转换？—— 可以，但没必要
    uint16_t hdr_checksum = ip_hdr->hdr_checksum16;
    ip_hdr->hdr_checksum16 = 0;
    
    if (hdr_checksum != checksum16((uint16_t *)ip_hdr, ip_hdr->hdr_len * IP_HDR_LEN_PER_BYTE))
        return;

    ip_hdr->hdr_checksum16 = hdr_checksum;
    
    // 对比目标IP和本机IP
    if (memcmp(net_if_ip, ip_hdr->dst_ip, NET_IP_LEN) != 0)
        return;

    // 去除填充字段
    if (buf->len > swap16(ip_hdr->total_len16))
        buf_remove_padding(buf, buf->len - swap16(ip_hdr->total_len16));
    
    uint8_t src_ip[NET_IP_LEN];
    memcpy(src_ip, ip_hdr->src_ip, NET_IP_LEN);
    uint8_t protocol = ip_hdr->protocol;
    if(protocol != NET_PROTOCOL_ICMP && protocol != NET_PROTOCOL_UDP){
        icmp_unreachable(buf, src_ip, ICMP_CODE_PROTOCOL_UNREACH);
        return;
    }
    buf_remove_header(buf, ip_hdr->hdr_len * IP_HDR_LEN_PER_BYTE);

    // 向上层传递数据包
    net_in(buf, protocol, src_ip);
}

/**
 * @brief 处理一个要发送的ip分片
 * 
 * @param buf 要发送的分片
 * @param ip 目标ip地址
 * @param protocol 上层协议
 * @param id 数据包id
 * @param offset 分片offset，必须被8整除
 * @param mf 分片mf标志，是否有下一个分片
 */
void ip_fragment_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol, int id, uint16_t offset, int mf)
{
    // 增加IP数据报头部缓存空间
    buf_add_header(buf, sizeof(ip_hdr_t));

    ip_hdr_t *ip_hdr = (ip_hdr_t *)buf->data;

    // 填写IP数据报头部字段

    ip_hdr->version = IP_VERSION_4;
    ip_hdr->hdr_len = 5;
    ip_hdr->tos = 0;
    ip_hdr->total_len16 = swap16(buf->len);
    ip_hdr->id16 = swap16(id);

    if (mf) ip_hdr->flags_fragment16 = swap16(IP_MORE_FRAGMENT | offset);
    else    ip_hdr->flags_fragment16 = swap16(offset);

    ip_hdr->ttl = IP_DEFALUT_TTL;
    ip_hdr->protocol = protocol;
    ip_hdr->hdr_checksum16 = 0;

    memcpy(ip_hdr->src_ip, net_if_ip, NET_IP_LEN);
    memcpy(ip_hdr->dst_ip, ip, NET_IP_LEN);

    // 计算校验和
    uint16_t hdr_checksum = checksum16((uint16_t*)ip_hdr, sizeof(ip_hdr_t));
    ip_hdr->hdr_checksum16 = hdr_checksum;

    // 封装到buf中，并发送
    arp_out(buf, ip);
}

/**
 * @brief 处理一个要发送的ip数据包
 * 
 * @param buf 要处理的包
 * @param ip 目标ip地址
 * @param protocol 上层协议
 */
void ip_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol)
{
    static uint16_t ip_id = 0;

    // 若无需分段，则直接发送
    if (buf->len <= ETHERNET_MAX_TRANSPORT_UNIT - 20) {
        ip_fragment_out(buf, ip, protocol, ip_id++, 0, 0);
        return;
    }

    uint16_t cur = 0;

    // 分段发送，每段长度为1480B
    buf_t __fragment;
    buf_t *fragment = &__fragment;
    while (buf->len > 1480) {
        buf_init(fragment, 1480);
        memcpy(fragment->data, buf->data, 1480);
        buf_remove_header(buf, 1480);
        ip_fragment_out(fragment, ip, protocol, ip_id, cur/IP_HDR_OFFSET_PER_BYTE, 1);
        cur += 1480;
    }

    // 发送最后一个分片
    if (buf->len > 0) {
        buf_init(fragment, buf->len);
        memcpy(fragment->data, buf->data, buf->len);
        buf_remove_header(buf, buf->len);
        ip_fragment_out(fragment, ip, protocol, ip_id, cur/IP_HDR_OFFSET_PER_BYTE, 0);
    }
    ip_id++;
}

/**
 * @brief 初始化ip协议
 * 
 */
void ip_init()
{
    net_add_protocol(NET_PROTOCOL_IP, ip_in);
}