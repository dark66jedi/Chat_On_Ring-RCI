#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "udp_functions.h"
#include "tcp_functions.h"

int main(int argc, char *argv[]){
    char udp_port[5] = "58000";

    printf("UDP Port: %s\n", udp_port);
    UDP_Server(udp_port);
    

    return 0;
}