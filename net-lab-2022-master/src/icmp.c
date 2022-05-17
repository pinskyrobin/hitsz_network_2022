#include "net.h"
#include "icmp.h"
#include "ip.h"

/**
 * @brief 发送icmp响应
 * 
 * @param req_buf 收到的icmp请求包
 * @param src_ip 源ip地址
 */
static void icmp_resp(buf_t *req_buf, uint8_t *src_ip)
{
    buf_init(&txbuf, req_buf->len);
    memcpy(txbuf.data, req_buf->data, req_buf->len);

    icmp_hdr_t *req_icmp_hdr = (icmp_hdr_t *)req_buf->data;

    // 封装头部数据
    icmp_hdr_t *icmp_hdr = (icmp_hdr_t*)txbuf.data;
    icmp_hdr->type = ICMP_TYPE_ECHO_REPLY;
    icmp_hdr->code = 0;
    icmp_hdr->checksum16 = 0;
    icmp_hdr->id16 = req_icmp_hdr->id16;
    icmp_hdr->seq16 = req_icmp_hdr->seq16;

    // 计算校验和
    icmp_hdr->checksum16 = checksum16((uint16_t *)txbuf.data, txbuf.len);

    // 发送数据报
    ip_out(&txbuf, src_ip, NET_PROTOCOL_ICMP);
}

/**
 * @brief 处理一个收到的数据包
 * 
 * @param buf 要处理的数据包
 * @param src_ip 源ip地址
 */
void icmp_in(buf_t *buf, uint8_t *src_ip)
{
    // 报头检测
    if (buf->len < sizeof(icmp_hdr_t))   return;
    icmp_hdr_t *icmp_hdr = (icmp_hdr_t *)buf->data;
    if (icmp_hdr->type == ICMP_TYPE_ECHO_REQUEST &&
        icmp_hdr->code == 0)
            icmp_resp(buf, src_ip);
}

/**
 * @brief 发送icmp不可达
 * 
 * @param recv_buf 收到的ip数据包
 * @param src_ip 源ip地址
 * @param code icmp code，协议不可达或端口不可达
 */
void icmp_unreachable(buf_t *recv_buf, uint8_t *src_ip, icmp_code_t code)
{
    // 结构 icmp_hdr + ip_hdr + data(8 bytes)
    buf_init(&txbuf, sizeof(ip_hdr_t) + 8);
    memcpy(txbuf.data, recv_buf->data, sizeof(ip_hdr_t) + 8);

    buf_add_header(&txbuf, sizeof(icmp_hdr_t));

    // 构造icmp头部
    icmp_hdr_t *icmp_hdr = (icmp_hdr_t*)txbuf.data;
    icmp_hdr->type = ICMP_TYPE_UNREACH;
    icmp_hdr->code = code;
    icmp_hdr->checksum16 = 0;
    icmp_hdr->id16 = 0;
    icmp_hdr->seq16 = 0;
    
    // 计算校验和
    icmp_hdr->checksum16 = checksum16((uint16_t *)txbuf.data, txbuf.len);

    // 发送数据报
    ip_out(&txbuf, src_ip, NET_PROTOCOL_ICMP);
}

/**
 * @brief 初始化icmp协议
 * 
 */
void icmp_init(){
    net_add_protocol(NET_PROTOCOL_ICMP, icmp_in);
}