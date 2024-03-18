#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#ifndef _TCP_H
#define _TCP_H

int TCP_Client(char connectIP[], char connectTCP[], char mensagem[], int sock_created, int fd);

#endif