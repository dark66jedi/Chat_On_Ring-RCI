#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "protocol_msgs.h"
#include "udp_functions.h"
#include "tcp_functions.h"
#include "aux_functions.h"

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
    int fd = 0;
    
    strcat(msg_to_send, aux_id);
    strcat(msg_to_send, " ");
    strcat(msg_to_send, aux_IP);
    strcat(msg_to_send, " ");
    strcat(msg_to_send, aux_TCP);
    strcat(msg_to_send, "\n");
    fprintf(stdout,"Enviei ENTRY not me: %s", msg_to_send);
    //printf("pred_fd: %d\n", node->pred_fd);

    fd = TCP_Client(connectIP, connectTCP, msg_to_send, 1, node->pred_fd);
}

int ENTRY(char connectIP[], char connectTCP[], node *node)
{
    char msg_to_send[100];
    int fd = 0;
    
    sprintf(msg_to_send,"ENTRY %s %s %s\n",node->self, node->self_IP,node->self_TCP);
    fprintf(stdout, "Enviei ENTRY: %s", msg_to_send);

    fd = TCP_Client(connectIP, connectTCP, msg_to_send, 0, 0);
    return fd;
}

int PRED(char connectIP[], char connectTCP[], node *node, int sock_created)
{
    char msg_to_send[100];
    int fd = 0;
    sprintf(msg_to_send,"PRED %s\n",node->self);
    //fprintf(stdout, "Enviei para: %s %s", connectIP, connectTCP);
    fprintf(stdout, "ESTOU A ENVIAR PRED: \n");

    /*if(strcmp(node->succ_two, node->self) == 0){ //caso de dois nós
       // fprintf(stdout, "Estou a enviar o pred certo\n");
       // fprintf(stdout, "fd a enviar:%d\n", node->pred_fd_send);
        fd = TCP_Client(connectIP, connectTCP, msg_to_send, 1, node->pred_fd_send);
    } else{
        fd = TCP_Client(connectIP, connectTCP, msg_to_send, 0, 0);
    }*/
    if(sock_created==0){
        fd = TCP_Client(connectIP, connectTCP, msg_to_send, sock_created, 0); //não existe ainda, logo vai criar
    } else{
        fd = TCP_Client(connectIP, connectTCP, msg_to_send, sock_created, node->succ_fd);
        //fprintf(stdout, "fd: %i\n", fd);
    }
    return fd;
}

int SUCC(char connectIP[], char connectTCP[], node *node, int sock_created)
{
    char msg_to_send[100];
    int fd = 0;

    sprintf(msg_to_send,"SUCC %s %s %s\n",node->succ, node->succ_IP,node->succ_TCP);
    fprintf(stdout, "Enviei SUCC: %s\n", msg_to_send);
   // fprintf(stdout, "node->pred_fd_send %i\n", node->pred_fd_send);
//    fprintf(stdout, "node->self %s\n", node->self);

    fd = TCP_Client(connectIP, connectTCP, msg_to_send, 1, node->pred_fd);

   /* if(strcmp(node->succ_two, node->self) == 0){ //caso de dois nós
      // fprintf(stdout, "Estou a enviar o succ certo\n");
       // fprintf(stdout, "fd a enviar:%d\n", node->pred_fd_send);
        fd = TCP_Client(connectIP, connectTCP, msg_to_send, 1, node->pred_fd);
    } else{
        fd = TCP_Client(connectIP, connectTCP, msg_to_send, 1, node->pred_fd);
        fprintf(stdout, "Estou a enviar certo! fd a enviar:%d\n", node->pred_fd);
    }*/
    /*if(sock_created==0){
        fd = TCP_Client(connectIP, connectTCP, msg_to_send, sock_created, 0); //não existe ainda, logo vai criar
    } else{
        fd = TCP_Client(connectIP, connectTCP, msg_to_send, sock_created, node->succ_fd_send);
        fprintf(stdout, "fd: %i\n", fd);
    }*/

    return fd;
}

void CHORD(char connectIP[], char connectTCP[], node *node)
{
    char msg_to_send[100];
    int fd = 0;
    
    sprintf(msg_to_send,"CHORD %s\n",node->self);

    fd = TCP_Client(connectIP, connectTCP, msg_to_send, 0, 0); //ver ainda os 0s
}

void ROUTE(node *node, char target_node[], int fd)
{
    char msg_to_send[100] = "";

    for (size_t i = 0; i < MAX_N_NODES; i++)
    {
        if(strcmp(node->short_paths_table[i], "-") == 0) continue;
        else
        {
            sprintf(msg_to_send, "ROUTE %s %s %s\n", node->self, node->table_indexes[i], node->short_paths_table[i]);
            TCP_Client(NULL, NULL, msg_to_send, 1, fd);
        }
        
    }
}

void ROUTE_Received(node *node, char origin_id[], char dest_id[], char path[])
{
    int origin_id_index, dest_id_index;

    search_index(node, origin_id, origin_id_index);
    search_index(node, dest_id, dest_id_index);

    invert_order(path);

    strcpy(node->expedition_table, path);
}

void initialize_tables(node *node)
{
    int i, k;
    for (i = 0; i < MAX_N_NODES; i++)
    {
        for (k = 0; k < MAX_N_NODES; k++)
        {
            strcpy(node->routing_table[i][k], "-");
        }
    }
}