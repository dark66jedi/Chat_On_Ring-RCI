#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "protocol_msgs.h"
#include "udp_functions.h"
#include "tcp_functions.h"
#include "aux_functions.h"
#include "user_cmds.h"

void NODES(char ring[], char answer[])
{
    char msg_to_send[50] = "NODES ";

    strcat(msg_to_send, ring);
    strcpy(answer, UDP_Client(regIP, regUDP, msg_to_send));
}

void REG(char ring[], char answer[], node *node)
{
    char msg_to_send[100];

    sprintf(msg_to_send, "REG %s %s %s %s", ring, node->self, node->self_IP, node->self_TCP);
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

int ENTRY(char connectIP[], char connectTCP[], node *node)
{
    char msg_to_send[100];
    int fd = 0;
    
    sprintf(msg_to_send,"ENTRY %s %s %s\n",node->self, node->self_IP,node->self_TCP);

    fd = TCP_Client(connectIP, connectTCP, msg_to_send, 0, 0);
    return fd;
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

    fd = TCP_Client(connectIP, connectTCP, msg_to_send, 1, node->pred_fd);
}

void ENTRY_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd)
{
  int lixo = 0;

  if (fd == node->succ_fd)
  { // recebi um ENTRY NOT ME
    close(node->succ_fd);
    node->succ_fd = PRED(aux_IP, aux_TCP, node, 0);
    strcpy(node->node_left, node->succ);
    strcpy(node->succ_two, node->succ);
    strcpy(node->succ_two_IP, node->succ_IP);
    strcpy(node->succ_two_TCP, node->succ_TCP);
    strcpy(node->succ, aux_id);
    strcpy(node->succ_IP, aux_IP);
    strcpy(node->succ_TCP, aux_TCP);
    show_topology(node);
    lixo = SUCC("0", "0", node, 1);
    update_neighbour(node, node->node_left, node->succ); // essencialmente não está a estragar nada porque só apaga as entradas nas tabelas
  }
  else
  { // caso de novo nó
    if (strcmp(node->pred, node->self) == 0)
    { // caso em que ainda só são 2 nós na rede
      strcpy(node->pred, aux_id);
      node->pred_fd = fd;
      strcpy(node->succ, aux_id);
      strcpy(node->succ_IP, aux_IP);
      strcpy(node->succ_TCP, aux_TCP);
      lixo = SUCC(aux_IP, aux_TCP, node, 1);
      node->succ_fd = PRED(aux_IP, aux_TCP, node, 0);
    }
    else
    { //+2 nós
      ENTRY_not_me("0", "0", node, aux_id, aux_IP, aux_TCP);
      node->pred_fd = fd;
      strcpy(node->node_left, node->pred);
      strcpy(node->pred, aux_id);
      lixo = SUCC(aux_IP, aux_TCP, node, 1);
      update_neighbour(node, node->node_left, node->pred);
    }
  }
}

int PRED(char connectIP[], char connectTCP[], node *node, int sock_created)
{
    char msg_to_send[100];
    int fd = 0;
    sprintf(msg_to_send,"PRED %s\n",node->self);

    if(sock_created==0){
        fd = TCP_Client(connectIP, connectTCP, msg_to_send, sock_created, 0); 
    } else{
        fd = TCP_Client(connectIP, connectTCP, msg_to_send, sock_created, node->succ_fd);
    }
    return fd;
}

void PRED_Received(char aux_id[], node *node, int fd)
{
  int lixo = 0;
  int fd_index = 0;
  char answer[100];

  if (node->pred_left_bool == 1)
  {
    strcpy(node->pred, aux_id);
    node->pred_fd = fd;
    lixo = SUCC("0", "0", node, 1);
    node->pred_left_bool = 0;
  }
  else
  {
    strcpy(node->pred, aux_id);
    node->pred_fd = fd;
    REG(node->ring, answer, node);
    if (strcmp(answer, "OKREG") != 0)
    {
      printf("Erro no registo do nó no anel");
      exit(1);
    }
    node->ring_bool = 1;
  }

  // atualizar tabelas
  update_neighbour(node, node->node_left, node->succ);
  // enviar route
  ROUTE(node, node->pred, node->pred_fd);
  ROUTE(node, node->succ, node->succ_fd);
  if (node->chord_bool == 1)
  {
    ROUTE(node, node->chord, node->chord_fd);
  }
  for (size_t j = 0; j < MAX_N_NODES; j++)
  {
    if (node->chord_rec[j] != -1)
    {
      search_fd(node, node->chord_rec[j], &fd_index);
      ROUTE(node, node->table_indexes[fd_index], node->chord_rec[j]);
    }
  }
  ROUTE(node, node->pred, node->pred_fd);
  ROUTE(node, node->succ, node->succ_fd);
}

int SUCC(char connectIP[], char connectTCP[], node *node, int sock_created)
{
    char msg_to_send[100];
    int fd = 0;

    sprintf(msg_to_send,"SUCC %s %s %s\n",node->succ, node->succ_IP,node->succ_TCP);

    fd = TCP_Client(connectIP, connectTCP, msg_to_send, 1, node->pred_fd);
    return fd;
}

void SUCC_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd)
{
  strcpy(node->succ_two, aux_id);
  strcpy(node->succ_two_IP, aux_IP);
  strcpy(node->succ_two_TCP, aux_TCP);
}

int CHORD(char connectIP[], char connectTCP[], node *node)
{
    char msg_to_send[100];
    int fd = 0;
    
    sprintf(msg_to_send,"CHORD %s\n",node->self);

    fd = TCP_Client(connectIP, connectTCP, msg_to_send, 0, 0); 
    return fd;
}

void CHORD_Received(char aux_id[], node *node, int fd)
{
  int int_id = 0;
  int i = 0;

  search_index(node, aux_id, &int_id);
  node->chord_rec[int_id] = fd;
  update_neighbour(node, NULL, aux_id);
  ROUTE(node, node->pred, node->pred_fd);
  ROUTE(node, node->succ, node->succ_fd);

  for (i = 0; i < MAX_N_NODES; i++)
  {
    if (node->chord_rec[i] != -1)
    {
      ROUTE(node, aux_id, node->chord_rec[i]);
    }
  }
  return;
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
            fprintf(stdout, "mensagem ROUTE: %s\n", msg_to_send);
            fprintf(stdout, "fd ROUTE: %d\n", fd);
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
    int teste;
    char new_node[3] = "";
    char new_path[50] = "";
    int im_in_path;

    check_for_new_node(node, path);
    check_for_self_in_path(node, path, &im_in_path);

    search_index(node, origin_id, &origin_id_index);
    search_index(node, dest_id, &dest_id_index);    

    if(strcmp(node->self, dest_id) != 0)
    {
        sprintf(new_path, "%s-%s", node->self, path);

        if (strlen(new_path) != strlen(node->routing_table[dest_id_index][origin_id_index]) && im_in_path == 0)
        {
            strcpy(node->routing_table[dest_id_index][origin_id_index], new_path);
            update_tables(node);
            ROUTE(node, node->pred, node->pred_fd);
            ROUTE(node, node->succ, node->succ_fd);
        }
    }
    else
    {
        sprintf(new_path, "%s", node->self);

        if (strlen(new_path) != strlen(node->routing_table[dest_id_index][origin_id_index]))
        {
            strcpy(node->routing_table[dest_id_index][origin_id_index], new_path);
            update_tables(node);
            ROUTE(node, node->pred, node->pred_fd);
            ROUTE(node, node->succ, node->succ_fd);
        }
    } 
    update_tables(node);
}

void CHAT(node *node, char origin[], char dest[], char message[]){
    //Está feita para o caso de enviar sempre para o succ, depois é preciso alterar
    char msg_to_send[128];
    int fd = 0;
    int dest_idx, next_node_idx;
    sprintf(msg_to_send,"CHAT %s %s %s\n", origin, dest, message);

    search_index(node, dest, &dest_idx);
    
    // QUERO CONFIRMAR ISTO AMANHÃ CONTIGO MARTA!!!
    if (strcmp(node->expedition_table[dest_idx], node->pred) == 0)
    {
        fd = TCP_Client("0", "0", msg_to_send, 1, node->pred_fd);
    }
    else if (strcmp(node->expedition_table[dest_idx], node->succ) == 0)
    {
        fd = TCP_Client("0", "0", msg_to_send, 1, node->succ_fd);
    }
    else if (strcmp(node->expedition_table[dest_idx], node->chord) == 0)
    {
        fd = TCP_Client("0", "0", msg_to_send, 1, node->chord_fd);
    }
    else
    {
        search_index(node, node->expedition_table[dest_idx], &next_node_idx);
        fd = TCP_Client("0", "0", msg_to_send, 1, node->chord_rec[next_node_idx]);
    }
}

void CHAT_Received(node *node, char origin[], char dest[], char message[])
{
  if (strcmp(node->self, dest) == 0)
  {
    fprintf(stdout, "\nRecebi uma mensagem: %s\n", message);
  }
  else
  {
    fprintf(stdout, "\nRecebi uma mensagem e vou redirecionar: %s\n", message);
    CHAT(node, origin, dest, message);
  }
}
