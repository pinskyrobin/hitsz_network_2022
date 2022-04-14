#include <stdio.h>
#include "driver.h"
#include "ethernet.h"

extern FILE *pcap_in;
extern FILE *pcap_out;
extern FILE *control_flow;
extern FILE *ip_fout;
extern FILE *arp_fout;
extern FILE *demo_log;
extern FILE *out_log;

int check_log();
FILE* open_file(char * path, char * name, char * mode);

buf_t buf;
int main(int argc, char* argv[]){
        int ret;
        pcap_in = open_file(argv[1], "in.pcap", "r");
        pcap_out = open_file(argv[1], "out.pcap","w");
        ip_fout = open_file(argv[1], "log","w");
        if(pcap_in == 0 || pcap_out == 0 || ip_fout == 0){
                if(pcap_in) fclose(pcap_in); else printf("\e[1;31mFailed to open in.pcap\n");
                if(pcap_out)fclose(pcap_out); else printf("\e[1;31mFailed to open out.pcap\n");
                if(ip_fout) fclose(ip_fout); else printf("\e[1;31mFailed to open log\n");
                printf("\e[0m");
                return -1;
        }
        arp_fout = ip_fout;
        control_flow = ip_fout;

        printf("\e[0;34mTest start\n");
        net_init();
        int i = 1;
        printf("\e[0;34mFeeding input %02d",i);
        while((ret = driver_recv(&buf)) > 0){
                printf("\b\b%02d",i);
                fprintf(control_flow,"\nRound %02d -----------------------------\n",i++);
                ethernet_in(&buf);
        }
        if(ret < 0){
                fprintf(stderr,"\e[1;31m\nError occur on loading input,exiting\n");
        }
        driver_close();
        printf("\e[0;34m\nSample input all processed, checking output\n");

        fclose(ip_fout);

        demo_log = open_file(argv[1], "demo_log","r");
        out_log = open_file(argv[1], "log","r");
        if(demo_log == 0 || out_log == 0){
                if(demo_log) fclose(demo_log); else printf("\e[1;31mFailed to open demo_log\n");
                if(out_log) fclose(out_log); else printf("\e[1;31mFailed to open log\n");
                printf("\e[0m");
                return -1;
        }
        ret = check_log();
        fclose(demo_log);
        fclose(out_log);
        return (ret == 0) ? 0 : -1;
}