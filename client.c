#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "udp_functions.h"
#include "tcp_functions.h"
#include "protocol_msgs.h"

#define DEFINE_GLOBAL_VARIABLES
#include "global_variables.h"

int main(int argc, char *argv[]){
    char ring[3];
    char *answer;
    // char udp_port[5] = "59000";
    // char* message;

    // message = (char*)malloc((strlen(argv[1] + 1))*sizeof(char));
    // if(message == NULL)
    //     exit(1);

    strcpy(regIP, argv[1]);     // IP
    strcpy(regUDP, argv[2]);    // Port
    strcpy(ring, argv[3]);

    printf("regIP: %s\n", regIP);
    printf("regUDP: %s\n", regUDP);
    printf("ring: %s\n", ring);

    NODES(ring);
    // printf("%s", answer);

    // free(message);

    return 0;
}