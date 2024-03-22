#include "udp_functions.h"

char *UDP_Client(char regIP[], char regUDP[], char mensagem[])
{
    fd_set rfds;
    int fd = 0, maxfd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    struct timeval timeout;
    static char buffer[128];
    int counter = 0;

    fd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (fd == -1)                        /*error*/
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    errcode = getaddrinfo(regIP, regUDP, &hints, &res);
    memset(buffer, '\0', 128);
    if (errcode != 0) /*error*/
        exit(1);

    while (1)
    { // ACTUALLY select
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        maxfd = fd;

        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        n = sendto(fd, mensagem, strlen(mensagem), 0, res->ai_addr, res->ai_addrlen);
        if (n == -1) /*error*/
            exit(1);

        counter = select(maxfd + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, &timeout);

        if (counter == 0)
        {
            printf("\n\nDEU CONTINUE\n\n");
            continue;
        }
        
        if (FD_ISSET(fd, &rfds))
        {
            addrlen = sizeof(addr);
            n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
            if (n == -1) /*error*/
                exit(1);
            break;
        }
    }
    // write(1, buffer, n);
    freeaddrinfo(res);
    close(fd);
    return buffer;
}

void UDP_Server(char *port)
{
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[128];

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) /*error*/
        exit(1);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    hints.ai_flags = AI_PASSIVE;
    errcode = getaddrinfo(NULL, port, &hints, &res);
    if (errcode != 0) /*error*/
        exit(1);
    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/
        exit(1);

    while (1)
    {
        addrlen = sizeof(addr);
        n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
        if (n == -1) /*error*/
            exit(1);
        write(1, "received: ", 10);
        write(1, buffer, n);
        n = sendto(fd, buffer, n, 0, (struct sockaddr *)&addr, addrlen);
        if (n == -1) /*error*/
            exit(1);
    }
    freeaddrinfo(res);
    close(fd);
}