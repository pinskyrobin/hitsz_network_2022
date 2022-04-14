#include <pcap.h>
#include "driver.h"

#ifdef _WIN32
#include <tchar.h>
/**
 * @brief npcp官方提供的加载npcap的dll库函数
 * 
 * @return BOOL 是否成功
 */
BOOL LoadNpcapDlls()
{
    _TCHAR npcap_dir[512];
    UINT len;
    len = GetSystemDirectory(npcap_dir, 480);
    if (!len)
    {
        fprintf(stderr, "Error in GetSystemDirectory: %lx", GetLastError());
        return FALSE;
    }
    _tcscat_s(npcap_dir, 512, _T("\\Npcap"));
    if (SetDllDirectory(npcap_dir) == 0)
    {
        fprintf(stderr, "Error in SetDllDirectory: %lx", GetLastError());
        return FALSE;
    }
    return TRUE;
}
#endif

pcap_t *pcap;
char pcap_errbuf[PCAP_ERRBUF_SIZE];

/**
 * @brief 根据ip进行前缀匹配，选取最长前缀匹配的网卡
 * 
 * @param ip ip地址
 * @param if_name 出口参数，选取的网卡名
 * @param mask 出口参数，该网卡的掩码
 * @return int 成功为0，失败为-1
 */
int driver_find(uint8_t *ip, char *if_name, uint8_t *mask)
{
    pcap_if_t *alldevs;
    pcap_if_t *d;
    pcap_addr_t *a;
    size_t i;
    uint8_t match[PCAP_BUF_SIZE] = {0};
    size_t if_num = 0;
    uint32_t mask_all = PCAP_NETMASK_UNKNOWN;
    if (pcap_findalldevs(&alldevs, pcap_errbuf) == -1)
    {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", pcap_errbuf);
        return -1;
    }

    for (d = alldevs; d; d = d->next, if_num++)
        for (a = d->addresses; a; a = a->next)
            if (a->addr && a->addr->sa_family == AF_INET)
            {
                match[if_num] = ip_prefix_match(ip, (uint8_t *)&((struct sockaddr_in *)a->addr)->sin_addr.s_addr);
                if (match[if_num] < ip_prefix_match((uint8_t *)&mask_all, (uint8_t *)&((struct sockaddr_in *)(a->netmask))->sin_addr.s_addr))
                    match[if_num] = 0;
            }
    if (if_num == 0)
    {
        fprintf(stderr, "Error, no interface found.\n");
        return -1;
    }
    uint8_t max_match = 0;
    size_t max_if = 0;
    for (i = 0; i < if_num; i++)
        if (match[i] > max_match)
            max_if = i, max_match = match[i];
    if (max_match == 0)
    {
        fprintf(stderr, "Error, no interface found.\n");
        return -1;
    }

    for (d = alldevs, i = 0; i < max_if; d = d->next, i++)
        ;
    if (max_match == 32)
    {
        fprintf(stderr, "Error, interface %s have the same ip %s with me.\n", d->name, iptos(net_if_ip));
        return -1;
    }
    for (a = d->addresses; a; a = a->next)
        if (a->addr && a->addr->sa_family == AF_INET)
            *(uint32_t *)mask = ((struct sockaddr_in *)(a->netmask))->sin_addr.s_addr;

    strcpy(if_name, d->name);
    return 0;
}

/**
 * @brief 打开网卡
 * 
 * @return int 成功为0，失败为-1
 */
int driver_open()
{
#ifdef _WIN32
    /* Load Npcap and its functions. */
    if (!LoadNpcapDlls())
    {
        fprintf(stderr, "Couldn't load Npcap\n");
        return -1;
    }
#endif

    char if_name[PCAP_BUF_SIZE];
    uint32_t mask;
    if (driver_find(net_if_ip, if_name, (uint8_t *)&mask) < 0)
    {
        fprintf(stderr, "Error in driver find.\n");
        return -1;
    }
    printf("Using interface %s, my ip is %s.\n", if_name, iptos(net_if_ip));

    if ((pcap = pcap_open_live(if_name, 65536, 1, 10, pcap_errbuf)) == NULL) //混杂模式打开网卡
    {
        fprintf(stderr, "Error in pcap_open_live.\n%s.\n", pcap_errbuf);
        return -1;
    }
    if (pcap_setnonblock(pcap, 1, pcap_errbuf) < 0) //设置非阻塞模式
    {
        fprintf(stderr, "Error in pcap_setnonblock. %s.\n", pcap_errbuf);
        return -1;
    }
    char filter_exp[PCAP_BUF_SIZE];
    struct bpf_program fp;
    uint8_t mac_addr[6] = NET_IF_MAC;
    sprintf(filter_exp, //过滤数据包
            "(ether dst %02x:%02x:%02x:%02x:%02x:%02x or ether broadcast) and (not ether src %02x:%02x:%02x:%02x:%02x:%02x)",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5],
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    if (pcap_compile(pcap, &fp, filter_exp, 0, mask) < 0)
    {
        fprintf(stderr, "Error in pcap_compile.\n%s.\n", pcap_geterr(pcap));
        return -1;
    }
    if (pcap_setfilter(pcap, &fp) < 0)
    {
        fprintf(stderr, "Error in pcap_setfilter.\n%s.\n", pcap_geterr(pcap));
        return -1;
    }
    return 0;
}
/**
 * @brief 试图从网卡接收数据包
 * 
 * @param buf 收到的数据包
 * @return int 数据包的长度，未收到为0，错误为-1
 */
int driver_recv(buf_t *buf)
{
    struct pcap_pkthdr *pkt_hdr;
    const uint8_t *pkt_data;
    int ret = pcap_next_ex(pcap, &pkt_hdr, &pkt_data);
    if (ret == 0)
        return 0;
    else if (ret == 1)
    {
        memcpy(buf->data, pkt_data, pkt_hdr->len);
        buf->len = pkt_hdr->len;
        return pkt_hdr->len;
    }
    fprintf(stderr, "Error in driver_recv.\n%s.\n", pcap_geterr(pcap));
    return -1;
}
/**
 * @brief 使用网卡发送一个数据包
 * 
 * @param buf 要发送的数据包
 * @return int 成功为0，失败为-1
 */
int driver_send(buf_t *buf)
{
    if (pcap_sendpacket(pcap, buf->data, buf->len) == -1)
    {
        fprintf(stderr, "Error in driver_send.\n%s.\n", pcap_geterr(pcap));
        return -1;
    }

    return 0;
}
/**
 * @brief 关闭网卡
 * 
 */
void driver_close()
{
    pcap_close(pcap);
}
