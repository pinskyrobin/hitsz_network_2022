#ifndef DRIVER_H
#define DRIVER_H

#include "net.h"

#ifndef PCAP_BUF_SIZE
#define PCAP_BUF_SIZE 1024
#endif
int driver_open();
int driver_recv(buf_t *buf);
int driver_send(buf_t *buf);
void driver_close();
#endif