#include <stdio.h>
#include <string.h>
#include "net.h"
#include "ethernet.h"
#include "arp.h"
#include "driver.h"

extern FILE *pcap_in;
extern FILE *pcap_out;
extern FILE *pcap_demo;
extern FILE *ip_fout;
extern FILE *control_flow;
extern FILE *demo_log;
extern FILE *out_log;
extern FILE *arp_log_f;

char* print_ip(uint8_t *ip);
char* print_mac(uint8_t *mac);

uint8_t my_mac[] = NET_IF_MAC;
uint8_t boardcast_mac[] = {0xff,0xff,0xff,0xff,0xff,0xff};

int check_log();
int check_pcap();
FILE* open_file(char * path, char * name, char * mode);
void log_tab_buf();

buf_t buf;
int main(int argc, char* argv[]){
        int ret;
        printf("\e[0;34mTest begin.\n");
        pcap_in = open_file(argv[1], "in.pcap","r");
        pcap_out = open_file(argv[1], "out.pcap","w");
        control_flow = open_file(argv[1], "log","w");
        if(pcap_in == 0 || pcap_out == 0 || control_flow == 0){
                if(pcap_in) fclose(pcap_in); else printf("\e[1;31mFailed to open in.pcap\n");
                if(pcap_out)fclose(pcap_out); else printf("\e[1;31mFailed to open out.pcap\n");
                if(control_flow) fclose(control_flow); else printf("\e[1;31mFailed to open log\n");
                printf("\e[0m");
                return -1;
        }
        arp_log_f = control_flow;
        ip_fout = control_flow;

        printf("\e[0;34mTest start\n");
        net_init();
        log_tab_buf();
        int i = 1;
        printf("\e[0;34mFeeding input %02d",i);
        while((ret = driver_recv(&buf)) > 0){
                printf("\b\b%02d",i);
                fprintf(control_flow,"\nRound %02d -----------------------------\n",i++);
                if(memcmp(buf.data,my_mac,6) && memcmp(buf.data,boardcast_mac,6)){
                        buf_t buf2;
                        buf_copy(&buf2, &buf, 0);
                        memset(buf2.data,0,sizeof(ether_hdr_t));
                        buf_remove_header(&buf2, sizeof(ether_hdr_t));
                        uint8_t * ip = buf.data + 30;
                        // net_protocol_t pro = buf.data[13] ? NET_PROTOCOL_ARP : NET_PROTOCOL_IP;
                        arp_out(&buf2, ip);
                }else{
                        ethernet_in(&buf);
                }
                log_tab_buf();
        }
        if(ret < 0){
                fprintf(stderr,"\e[1;31m\nError occur on receive,exiting\n");
        }
        driver_close();
        printf("\e[0;34m\nSample input all processed, checking output\n");

        fclose(control_flow);

        demo_log = open_file(argv[1], "demo_log","r");
        out_log = open_file(argv[1], "log","r");
        pcap_out = open_file(argv[1], "out.pcap","r");
        pcap_demo = open_file(argv[1], "demo_out.pcap","r");
        if(demo_log == 0 || out_log == 0 || pcap_out == 0 || pcap_demo == 0){
                if(demo_log) fclose(demo_log); else printf("\e[1;31mFailed to open demo_log\n");
                if(out_log) fclose(out_log); else printf("\e[1;31mFailed to open log\n");
                if(pcap_demo) fclose(pcap_demo); else printf("\e[1;31mFailed to open demo_out.pcap\n");
                if(pcap_out) fclose(pcap_out); else printf("\e[1;31mFailed to open out.pcap\n");
                printf("\e[0m");
                return -1;
        }
        check_log();
        ret = check_pcap() ? 1 : 0;
        printf("\e[1;33mFor this test, log is only a reference. \
Your implementation is OK if your pcap file is the same to the demo pcap file.\n\e[0m");
        fclose(demo_log);
        fclose(out_log);
        return ret ? -1 : 0;
}