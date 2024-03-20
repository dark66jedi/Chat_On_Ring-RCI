#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifndef GLOBAL_VARIABLES_H_
#define GLOBAL_VARIABLES_H_

#define EXTERN extern

#ifdef DEFINE_GLOBAL_VARIABLES
    #define EXTERN     /*nothing*/
#else
    #define EXTERN     extern
#endif

EXTERN char regIP[];
EXTERN char regUDP[];
EXTERN char IP_global[];
EXTERN char TCP_global[];

#define MAX_N_NODES 30

typedef struct node
{
  char ring[4];
  char self[3];      // ID
  char self_IP[50];  // IP da máquina
  char self_TCP[10]; // porto
  char pred[3];
  char succ[3];
  char succ_IP[50];
  char succ_TCP[10];
  char succ_two[3];
  char succ_two_IP[50];
  char succ_two_TCP[10];
  char chord[3];
  char chord_IP[50];
  char chord_TCP[10];
  int pred_left_bool; //fica 1 quando o seu pred deu leave
  char node_left[3];
  int ring_bool;
  int chord_bool;                   // boleano, se ainda não foi criada uma corda está a 0 senão está a 1
  
  /* file descriptors necessários para o select*/
 // int listen_fd; não é especifico de cada nó, não me faz sentido que exista
  int pred_fd;
  int succ_fd;
  int chord_fd; //corda que EU criei
  int chord_rec[30]; // fd
  /*********************************************/

  int filling_table_index;
  char routing_table[30][30][50];   // tabela de encaminhamento do próprio nó [destino][vizinhos][caminhos]
  char short_paths_table[30][50];   // tabela de caminhos mais curtos do próprio nó
  char expedition_table[30][3];     // tabela de expedição do próprio nó
  char table_indexes[30][3];       // vetor que identifica o indice atribuido a cada id de um nó (ex.: o id do nó pode ser 50 mas o indice é 2)
} node;

// EXTERN node self_node;

#endif