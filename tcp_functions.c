#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "tcp_functions.h"

void TCP_Client(char* port)
{
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[128];

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        exit(1); // error
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo("tejo.tecnico.ulisboa.pt", port, &hints, &res);
    if (errcode != 0) /*error*/
        exit(1);

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/
        exit(1);
    n = write(fd, "Hello!\n", 7);
    if (n == -1) /*error*/
        exit(1);
    n = read(fd, buffer, 128);
    if (n == -1) /*error*/
        exit(1);
    write(1, "echo: ", 6);
    write(1, buffer, n);
    freeaddrinfo(res);
    close(fd);
}

void TCP_Server(char* port)
{
    int fd, newfd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[128];

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        exit(1); // error
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    // TCP socket
    // IPv4
    // TCP socket
    errcode = getaddrinfo(NULL, port, &hints, &res);
    if ((errcode) != 0) /*error*/
        exit(1);
    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/
        exit(1);
    if (listen(fd, 5) == -1) /*error*/
        exit(1);

    while (1)
    {
        addrlen = sizeof(addr);
        if ((newfd = accept(fd, (struct sockaddr *)&addr, &addrlen)) == -1) /*error*/
            exit(1);
        n = read(newfd, buffer, 128);
        if (n == -1) /*error*/
            exit(1);
        write(1, "received: ", 10);
        write(1, buffer, n);
        n = write(newfd, buffer, n);
        if (n == -1) /*error*/
            exit(1);
        close(newfd);
    }
    freeaddrinfo(res);
    close(fd);
}
