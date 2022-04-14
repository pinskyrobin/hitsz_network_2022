#include "net.h"
#include "udp.h"
#include "driver.h"

#ifdef UDP
void handler(uint8_t *data, size_t len, uint8_t *src_ip, uint16_t src_port)
{
    printf("recv udp packet from %s:%u len=%zu\n", iptos(src_ip), src_port, len);
    for (int i = 0; i < len; i++)
        putchar(data[i]);
    putchar('\n');
    udp_send(data, len, 60000, src_ip, 60000); //发送udp包
}
#endif

int main(int argc, char const *argv[])
{

    if (net_init() == -1) //初始化协议栈
    {
        printf("net init failed.");
        return -1;
    }
#ifdef UDP
    udp_open(60000, handler); //注册端口的udp监听回调
#endif
    while (1)
    {
        net_poll(); //一次主循环
    }

    return 0;
}
