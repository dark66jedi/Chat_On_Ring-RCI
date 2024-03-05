#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "global_variables.h"

#ifndef _UDP_H
#define _UDP_H

char *UDP_Client(char regIP[], char regUDP[], char mensagem[]);
void UDP_Server(char* port);

#endif