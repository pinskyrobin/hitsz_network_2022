#include "net.h"
#include <string.h>
#include <stdio.h>

extern FILE *control_flow;

extern FILE *arp_fin;
extern FILE *arp_fout;

char* print_ip(uint8_t *ip);
char* print_mac(uint8_t *mac);
void fprint_buf(FILE* f, buf_t* buf);

map_t arp_table;
map_t arp_buf;

// void arp_update(uint8_t *ip, uint8_t *mac, arp_state_t state)
// {
//         fprintf(arp_fout,"arp update:\t");
//         fprintf(arp_fout,"ip:%s\t",print_ip(ip));
//         fprintf(arp_fout,"mac:%s\t",print_mac(mac));
//         fprintf(arp_fout,"state:%d\n",state);
// }

void arp_in(buf_t *buf, uint8_t *src_mac)
{
        fprintf(arp_fout,"arp_in:\n");
        fprintf(arp_fout,"\tmac:%s\n", print_mac(src_mac));
        fprint_buf(arp_fout,buf);
}

void arp_out(buf_t *buf, uint8_t *ip)
{
        fprintf(arp_fout,"arp_out:\n");
        fprintf(arp_fout,"\tip:%s\n",print_ip(ip));
        fprint_buf(arp_fout,buf);
}

void arp_init()
{
    map_init(&arp_table, NET_IP_LEN, NET_MAC_LEN, 0, ARP_TIMEOUT_SEC, NULL);
    map_init(&arp_buf, NET_IP_LEN, sizeof(buf_t), 0, ARP_MIN_INTERVAL, buf_copy);
    net_add_protocol(NET_PROTOCOL_ARP, arp_in);
}