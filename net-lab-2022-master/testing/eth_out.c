#include <stdio.h>
#include <string.h>
#include "driver.h"
#include "ethernet.h"

extern FILE *pcap_in;
extern FILE *pcap_out;
extern FILE *pcap_demo;
extern FILE *control_flow;
extern FILE *demo_log;
extern FILE *out_log;

int check_log();
int check_pcap();
char* print_mac(uint8_t *mac);
FILE* open_file(char * path, char * name, char * mode);

buf_t buf,buf2;
int main(int argc, char* argv[]){
        int ret;
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

        printf("\e[0;34mTest start\n");
        net_init();
        int i = 1;
        printf("\e[0;34mFeeding input %02d",i);
        while((ret = driver_recv(&buf)) > 0){
                printf("\b\b%02d",i);
                fprintf(control_flow,"\nRound %02d -----------------------------\n",i++);
                buf_copy(&buf2, &buf, 0);
                memset(buf.data,0,sizeof(ether_hdr_t));
                buf_remove_header(&buf, sizeof(ether_hdr_t));
                int proto = buf2.data[12];
                proto <<= 8;
                proto |= buf2.data[13];
                ethernet_out(&buf,buf2.data,proto);
        }
        if(ret < 0){
                fprintf(stderr,"\e[1;31m\nError occur on loading input,exiting\n");
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
        ret = check_log() ? 1 : 0;
        ret = check_pcap() ? 1 : ret;
        fclose(demo_log);
        fclose(out_log);
        return ret ? -1 : 0;
}