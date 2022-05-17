#include "icmp.h"
#include <string.h>
#include <stdio.h>

extern FILE *icmp_fin;
extern FILE *icmp_fout;
char* print_ip(uint8_t *ip);
void fprint_buf(FILE* f, buf_t* buf);

// static void icmp_resp(buf_t *req_buf, uint8_t *src_ip)
// {
//         fprintf(icmp_fout,"icmp_resp:\t");
//         fprintf(icmp_fout,"ip: %s\n",src_ip ? print_ip(src_ip) : "null");
//         fprint_buf(icmp_fout, req_buf);
// }

void icmp_in(buf_t *buf, uint8_t *src_ip)
{
        fprintf(icmp_fout,"icmp_in:\n");
        fprintf(icmp_fout,"\tip: %s\n",print_ip(src_ip));
        fprint_buf(icmp_fout, buf);
}


void icmp_unreachable(buf_t *recv_buf, uint8_t *src_ip, icmp_code_t code)
{
        fprintf(icmp_fout,"icmp_unreachable:\n");
        fprintf(icmp_fout,"\tip: %s\n",src_ip ? print_ip(src_ip) : "null");
        fprintf(icmp_fout,"\tcode: %d\n",code);
        fprint_buf(icmp_fout, recv_buf);
}

void icmp_init(){
    net_add_protocol(NET_PROTOCOL_ICMP, icmp_in);
}