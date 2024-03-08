#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "protocol_msgs.h"
#include "udp_functions.h"
#include "tcp_functions.h"

void NODES(char ring[], char answer[])
{
    char msg_to_send[50] = "NODES ";

    strcat(msg_to_send, ring);
    strcpy(answer, UDP_Client(regIP, regUDP, msg_to_send));
}

void REG(char ring[], char answer[], node *node)
{
    char msg_to_send[50] = "REG ";

    strcat(msg_to_send, ring);
    strcat(msg_to_send, " ");
    strcat(msg_to_send, node->self); // id
    strcat(msg_to_send, " ");
    strcat(msg_to_send, node->self_IP);
    strcat(msg_to_send, " ");
    strcat(msg_to_send, node->self_TCP);
    strcpy(answer, UDP_Client(regIP, regUDP, msg_to_send));
}

void UNREG(char ring[], char answer[], node *node)
{
    char msg_to_send[50] = "UNREG ";

    strcat(msg_to_send, ring);
    strcat(msg_to_send, " ");
    strcat(msg_to_send, node->self); // id
    strcpy(answer, UDP_Client(regIP, regUDP, msg_to_send));
}

void ENTRY_not_me(char connectIP[], char connectTCP[], node *node, char aux_id[], char aux_IP[], char aux_TCP[])
{
    char msg_to_send[50] = "ENTRY ";
    
    strcat(msg_to_send, aux_id);
    strcat(msg_to_send, " ");
    strcat(msg_to_send, aux_IP);
    strcat(msg_to_send, " ");
    strcat(msg_to_send, aux_TCP);
    strcat(msg_to_send, "\n");
    printf("\nmessage: %s\n", msg_to_send);

    TCP_Client(connectIP, connectTCP, msg_to_send);
}

void ENTRY(char connectIP[], char connectTCP[], node *node)
{
    char msg_to_send[50] = "ENTRY ";
    
    strcat(msg_to_send, node->self);
    strcat(msg_to_send, " ");
    strcat(msg_to_send, node->self_IP);
    strcat(msg_to_send, " ");
    strcat(msg_to_send, node->self_TCP);
    strcat(msg_to_send, "\n");
    //printf("\nmessage: %s\n", msg_to_send);

    TCP_Client(connectIP, connectTCP, msg_to_send);
}

void receive_ENTRY()
{
    printf("\n============| receive_ENTRY |============\n\n");
}

void PRED(char connectIP[], char connectTCP[], node *node)
{
    char msg_to_send[50] = "PRED ";
    
    strcat(msg_to_send, node->self);
    strcat(msg_to_send, "\n");

    TCP_Client(connectIP, connectTCP, msg_to_send);
}

void SUCC(char connectIP[], char connectTCP[], node *node)
{
    char msg_to_send[50] = "SUCC ";
    
    strcat(msg_to_send, node->succ);
    strcat(msg_to_send, " ");
    strcat(msg_to_send, node->succ_IP);
    strcat(msg_to_send, " ");
    strcat(msg_to_send, node->succ_TCP);
    strcat(msg_to_send, "\n");

    TCP_Client(connectIP, connectTCP, msg_to_send);
}

void CHORD(char connectIP[], char connectTCP[], node *node)
{
    char msg_to_send[50] = "CHORD ";
    
    strcat(msg_to_send, node->self);
    strcat(msg_to_send, "\n");

    TCP_Client(connectIP, connectTCP, msg_to_send);
}