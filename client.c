#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "udp_functions.h"
#include "tcp_functions.h"

int main(int argc, char *argv[]){
    char udp_port[5] = "58000";
    char* message;

    message = (char*)malloc((strlen(argv[1] + 1))*sizeof(char));
    if(message == NULL)
        exit(1);

    strcpy(message, argv[1]);
    strcat(message, "\n");

    printf("UDP Port: %s\n", udp_port);
    printf("Message: %s", message);
    UDP_Client(udp_port, message);

    free(message);

    return 0;
}