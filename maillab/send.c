#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"

#define MAX_SIZE 4095
#define swap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF))

typedef struct sockaddr_in sockaddr_in;

char buf[MAX_SIZE+1];

// Get message from server, and print it out
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

char* get_file_content(const char* file_name) {
    // Source of base64 encoding function
    FILE* src = fopen(file_name, "rb");
    // Destination of base64 encoding function
    FILE* dst = fopen("tmp", "wb+");
    if (src == NULL || dst == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    encode_file(src, dst);
    fclose(src);

    // Get the size of the file
    fseek(dst, 0, SEEK_END);
    int file_size = ftell(dst);
    char* file_content = (char*) malloc((file_size + 1) * sizeof(char));
    rewind(dst);
    
    // Read the file content
    fread(file_content, 1, file_size, dst);
    file_content[file_size] = '\0';

    fclose(dst);
    return file_content;
}

// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = ""; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = ""; // TODO: Specify the user
    const char* pass = ""; // TODO: Specify the password
    const char* from = ""; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;
    const char* from_encoded = encode_str(from);

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

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
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

    // Send EHLO command and print server response
    send(s_fd, "EHLO ", strlen("EHLO "), 0);
    send(s_fd, user, strlen(user), 0);
    send(s_fd, "\r\n", strlen("\r\n"), 0);

    // TODO: Print server response to EHLO command
    print_msg(&r_size, s_fd);

    // TODO: Authentication. Server response should be printed out.
    send(s_fd, "AUTH login\r\n", strlen("AUTH login\r\n"), 0);
    print_msg(&r_size, s_fd);

    send(s_fd, from_encoded, strlen(from_encoded), 0);
    send(s_fd, "\r\n", strlen("\r\n"), 0);
    print_msg(&r_size, s_fd);

    send(s_fd, pass, strlen(pass), 0);
    send(s_fd, "\r\n", strlen("\r\n"), 0);
    print_msg(&r_size, s_fd);


    // TODO: Send MAIL FROM command and print server response
    send(s_fd, "MAIL FROM:<", strlen("MAIL FROM:<"), 0);
    send(s_fd, from, strlen(from), 0);
    send(s_fd, ">\r\n", strlen(">\r\n"), 0);
    print_msg(&r_size, s_fd);
    
    // TODO: Send RCPT TO command and print server response
    send(s_fd, "RCPT TO:<", strlen("RCPT TO:<"), 0);
    send(s_fd, receiver, strlen(receiver), 0);
    send(s_fd, ">\r\n", strlen(">\r\n"), 0);
    print_msg(&r_size, s_fd);
    
    // TODO: Send DATA command and print server response
    send(s_fd, "DATA\r\n", strlen("DATA\r\n"), 0);
    print_msg(&r_size, s_fd);

    // Get file_name, message content and attachment content with base64 encoding
    char* file_name;
    char* msg_content_encoded = NULL;
    char* att_content_encoded = NULL;
    if (msg != NULL)
    {
        msg_content_encoded = get_file_content(msg);
    }
    if (att_path != NULL)
    {
        // Get file name
        // Notice that path in Windows is separated with '\', while in Linux or macOS is separated with '/'

        // Case 1: path separated with '/'
        char* file_name_ptr = strrchr(att_path, '/');

        // Case 2: path separated with '\'
        if (file_name_ptr == NULL)
        {
            file_name_ptr = strrchr(att_path, '\\');
        }

        // Result 1: path is a file name itself
        if (file_name_ptr == NULL)
        {
            file_name = (char*)malloc(sizeof(char) * strlen(att_path));
            strcpy(file_name, att_path);
        }
        // Result 2: path is a directory
        else {
            int file_name_len = -1;
            while (file_name_ptr[file_name_len] != '\0')
                ++file_name_len;
            file_name = (char*) malloc(sizeof(char) * (file_name_len + 1));
            strcpy(file_name, file_name_ptr + 1);
        }

        att_content_encoded = get_file_content(att_path);
    }


    // TODO: Send message data
    send(s_fd, "From: ", strlen("From: "), 0);
    send(s_fd, from, strlen(from), 0);
    send(s_fd, "\r\nTo: ", strlen("\r\nTo: "), 0);
    send(s_fd, receiver, strlen(receiver), 0);

    if (subject != NULL)
    {
        send(s_fd, "\r\nSubject: ", strlen("\r\nSubject: "), 0);
        send(s_fd, subject, strlen(subject), 0);
    }

    send(s_fd, "\r\nMIME-Version: 1.0", strlen("\r\nMIME-Version: 1.0"), 0);
    send(s_fd, "\r\nContent-Type: multipart/mixed; boundary=\"----=_boundary\"", strlen("\r\nContent-Type: multipart/mixed; boundary=\"----=_boundary\""), 0);
    send(s_fd, "\r\nContent-Transfer-Encoding: 8Bit", strlen("\r\nContent-Transfer-Encoding: 8Bit"), 0);

    send(s_fd, "\r\nThis is a multi-part message in MIME format.\r\n", strlen("\r\nThis is a multi-part message in MIME format.\r\n"), 0);
    send(s_fd, "\r\n------=_boundary", strlen("\r\n------=_boundary"), 0);

    if (msg_content_encoded != NULL)
    {
        send(s_fd, "\r\nContent-Type: text/plain", strlen("\r\nContent-Type: text/plain"), 0);
        send(s_fd, "\r\nContent-Transfer-Encoding: base64\r\n\r\n", strlen("\r\nContent-Transfer-Encoding: base64\r\n\r\n"), 0);
        send(s_fd, msg_content_encoded, strlen(msg_content_encoded), 0);
    }

    if (att_content_encoded != NULL)
    {
        send(s_fd, "\r\n------=_boundary", strlen("\r\n------=_boundary"), 0);
        send(s_fd, "\r\nContent-Type: application/octet-stream;charset=\"utf-8\";name=\"", strlen("\r\nContent-Type: application/octet-stream;charset=\"utf-8\";name=\""), 0);
        send(s_fd, file_name, strlen(file_name), 0);
        send(s_fd, "\"\r\n", strlen("\"\r\n"), 0);

        send(s_fd, "Content-Disposition:attachment; filename=\"", strlen("Content-Disposition:attachment; filename=\""), 0);
        send(s_fd, file_name, strlen(file_name), 0);
        send(s_fd, "\"\r\n", strlen("\"\r\n"), 0);

        send(s_fd, "Content-Transfer-Encoding:base64\r\n\r\n", strlen("Content-Transfer-Encoding:base64\r\n\r\n"), 0);

        send(s_fd, att_content_encoded, strlen(att_content_encoded), 0);
    }
    
    send(s_fd, "\r\n------=_boundary--\r\n", strlen("\r\n------=_boundary--\r\n"), 0);

    // TODO: Message ends with a single period
    send(s_fd, end_msg, strlen(end_msg), 0);

    print_msg(&r_size, s_fd);
    
    // TODO: Send QUIT command and print server response
    send(s_fd, "QUIT\r\n", strlen("QUIT\r\n"), 0);
    print_msg(&r_size, s_fd);

    free(msg_content_encoded);
    free(att_content_encoded);

    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}
