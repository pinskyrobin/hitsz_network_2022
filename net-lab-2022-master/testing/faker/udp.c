#include "udp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern FILE *udp_fin;
extern FILE *udp_fout;
char* print_ip(uint8_t *ip);
void fprint_buf(FILE* f, buf_t* buf);

void udp_out(buf_t *buf, uint16_t src_port, uint8_t *dest_ip, uint16_t dest_port)
{
        fprintf(udp_fout,"udp_out:\n");
        fprintf(udp_fout,"\tsrc_port: %d\n", src_port);
        fprintf(udp_fout,"\tdest_ip: %s\n", print_ip(dest_ip));
        fprintf(udp_fout,"\tdest_port: %d\n", dest_port);
        fprint_buf(udp_fout, buf);
}

void udp_init()
{
//     map_init(&udp_table, sizeof(uint16_t), sizeof(udp_handler_t), 0, 0, NULL);
    net_add_protocol(NET_PROTOCOL_UDP, udp_in);
}

int udp_open(uint16_t port, udp_handler_t handler)
{
        fprintf(udp_fout,"udp_open: port:%d\n",port);
        return 0;
}

void udp_close(uint16_t port)
{
        fprintf(udp_fout,"udp_close: port:%d\n",port);
}


void udp_send(uint8_t *data, uint16_t len, uint16_t src_port, uint8_t *dest_ip, uint16_t dest_port)
{
        fprintf(udp_fout,"udp_send:\n\tlen:%d\n",len);
        fprintf(udp_fout,"\tsrc_port:%d\n",src_port);
        fprintf(udp_fout,"\tdest_ip:%s\n",print_ip(dest_ip));
        fprintf(udp_fout,"\tdest_port:%d\n",dest_port);
        fprintf(udp_fout,"\tdata:");
        if(data){
                for(int i = 0; i < len; i++){
                        fprintf(udp_fout," %02x",data[i]);
                }
                fprintf(udp_fout,"\n");
        }else{
                fprintf(udp_fout," (null)\n");
        }
}

void udp_in(buf_t *buf, uint8_t *src_ip)
{
        fprintf(udp_fout,"udp_in:\n\tsrc_ip:%s\n",print_ip(src_ip));
        fprint_buf(udp_fout, buf);
}