#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "global_variables.h"
#include "protocol_msgs.h"
#include "udp_functions.h"


// extern char *regIP;
// extern char *regUDP;

char* NODES(char ring[])
{
    // char msg_to_send[50] = "NODES ";

    // strcat(msg_to_send, ring);
    // return UDP_Client(regIP, regUDP, msg_to_send);

    printf("=====================\n");
    printf("regIP: %s\n", regIP);
    printf("regUDP: %s\n", regUDP);
    printf("ring: %s\n", ring);
}