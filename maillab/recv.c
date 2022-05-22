#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535
#define swap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF))

typedef struct sockaddr_in sockaddr_in;

char buf[MAX_SIZE+1];

void print_msg(int *r_size, int s_fd)
{
    if ((*r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    buf[*r_size] = '\0';
    int i;
    for (i = 0; i < *r_size; i++)
        printf("%c", buf[i]);
    printf("\n");
    memset(buf, 0, MAX_SIZE + 1);
    *r_size = 0;
}

void recv_mail()
{
    const char* host_name = ""; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = ""; // TODO: Specify the user
    const char* pass = ""; // TODO: Specify the password
    char dest_ip[16];
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;
    int curr;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server
    if ((s_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    sockaddr_in *servaddr = (sockaddr_in *)malloc(sizeof(sockaddr_in));
    servaddr->sin_family = AF_INET;
    servaddr->sin_port = swap16(port);
    servaddr->sin_addr.s_addr = inet_addr(dest_ip);
    bzero(servaddr->sin_zero, 8);

    if (connect(s_fd, (struct sockaddr *)servaddr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // Print welcome message
    print_msg(&r_size, s_fd);

    // TODO: Send user and password and print server response
    send(s_fd, "USER ", strlen("USER "), 0);
    send(s_fd, user, strlen(user), 0);
    send(s_fd, "\r\n", strlen("\r\n"), 0);

    print_msg(&r_size, s_fd);

    send(s_fd, "PASS ", strlen("PASS "), 0);
    send(s_fd, pass, strlen(pass), 0);
    send(s_fd, "\r\n", strlen("\r\n"), 0);

    print_msg(&r_size, s_fd);

    // TODO: Send STAT command and print server response
    send(s_fd, "STAT\r\n", strlen("STAT\r\n"), 0);
    print_msg(&r_size, s_fd);

    // TODO: Send LIST command and print server response
    send(s_fd, "LIST\r\n", strlen("LIST\r\n"), 0);
    print_msg(&r_size, s_fd);

    // TODO: Retrieve the first mail and print its content
    send(s_fd, "RETR 1\r\n", strlen("RETR 1\r\n"), 0);
    print_msg(&r_size, s_fd);

    // TODO: Send QUIT command and print server response
    send(s_fd, "QUIT\r\n", strlen("QUIT\r\n"), 0);
    print_msg(&r_size, s_fd);

    close(s_fd);
}

int main(int argc, char* argv[])
{
    recv_mail();
    exit(0);
}
