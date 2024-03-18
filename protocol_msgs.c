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
    return fd;
}

int CHORD(char connectIP[], char connectTCP[], node *node)
{
    char msg_to_send[100];
    int fd = 0;
    
    sprintf(msg_to_send,"CHORD %s\n",node->self);
    fprintf(stdout,"mensagem CHORD:\n",msg_to_send);
    fprintf(stdout,"TCP:\n",connectIP);
    fprintf(stdout,"Porto:\n",connectTCP);

    fd = TCP_Client(connectIP, connectTCP, msg_to_send, 0, 0); 
    return fd;
}

void ROUTE(node *node, char target_node[], int fd)
{
    char msg_to_send[100] = "";

    for (size_t i = 1; i < MAX_N_NODES; i++)
    {
        if(strcmp(node->short_paths_table[i], "-") == 0) continue;
        else
        {
            sprintf(msg_to_send, "ROUTE %s %s %s\n", node->self, node->table_indexes[i], node->short_paths_table[i]);
            TCP_Client(NULL, NULL, msg_to_send, 1, fd);
        }
        
    }

    // ROUTE do próprio nó -> "ROUTE 30 30 30"
    // Tem que ser a última msg de ROUTE enviada por bloco de ROUTEs enviado
    sprintf(msg_to_send, "ROUTE %s %s %s\n", node->self, node->table_indexes[0], node->short_paths_table[0]);
    TCP_Client(NULL, NULL, msg_to_send, 1, fd);
}

void ROUTE_Received(node *node, char origin_id[], char dest_id[], char path[])
{
    int origin_id_index, dest_id_index;
    char new_node[3] = "";
    char new_path[50] = "";
    

    check_for_new_node(node, path, new_node);

    search_index(node, origin_id, origin_id_index);
    search_index(node, dest_id, dest_id_index);

    if(strcmp(node->self, dest_id) != 0)
    {
        sprintf(new_path, "%s-%s", node->self, path);

        if (strlen(new_path) != strlen(node->routing_table[dest_id_index][origin_id_index]))
        {
            strcpy(node->routing_table[dest_id_index][origin_id_index], path);
            update_tables(node);
        }
    }
    else
    {
        sprintf(new_path, "%s-%s", node->self, origin_id);

        if (strlen(new_path) != strlen(node->routing_table[dest_id_index][origin_id_index]))
        {
            strcpy(node->routing_table[dest_id_index][origin_id_index], path);
            update_tables(node);
        }
    }
    
    
}

void CHAT(node *node, char i[], char n[], char message[]){
    //Está feita para o caso de enviar sempre para o succ, depois é preciso alterar
    char msg_to_send[100];
    int fd = 0;
    sprintf(msg_to_send,"CHAT %s %s %s\n",i, n, message);

    fd = TCP_Client("0", "0", msg_to_send, 1, node->succ_fd); 
}
