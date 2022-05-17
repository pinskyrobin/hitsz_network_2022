#include "net.h"
#include <string.h>
#include <stdio.h>

extern FILE *control_flow;

extern FILE *ip_fin;
extern FILE *ip_fout;

char* print_ip(uint8_t *ip);
char* print_mac(uint8_t *mac);
void fprint_buf(FILE* f, buf_t* buf);

void ip_in(buf_t *buf, uint8_t *src_mac)
{
        fprintf(ip_fout,"ip_in:\n");
        fprintf(ip_fout,"\tmac:%s\n", print_mac(src_mac));
        fprint_buf(ip_fout, buf);
}

void ip_fragment_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol, int id, uint16_t offset, int mf)
{
        fprintf(ip_fout,"ip_fragment_out:\n");        
        fprintf(ip_fout,"\tip: %s\n", print_ip(ip));
        fprintf(ip_fout,"\tprotocol: %d\n",protocol);
        fprintf(ip_fout,"\tid: %d\n",id);
        fprintf(ip_fout,"\toffset: %d\n",offset);
        fprintf(ip_fout,"\tmf: %d\n",mf);
        fprint_buf(ip_fout, buf);
}

void ip_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol)
{
        fprintf(ip_fout,"\tip_out:\n");
        fprintf(ip_fout,"\tip: %s\n", print_ip(ip));
        fprintf(ip_fout,"\tprotocol: %d\n",protocol);
        fprint_buf(ip_fout, buf);
}

void ip_init()
{
    net_add_protocol(NET_PROTOCOL_IP, ip_in);
}