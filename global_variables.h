#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define max(A, B) ((A) >= (B) ? (A) : (B))

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
EXTERN char IP[];
EXTERN char TCP[];

typedef struct node
{
  char self[3];      // ID
  char self_IP[50];  // IP da máquina
  char self_TCP[10]; // porto
  char pred[3];
  char pred_IP[50];
  char pred_TCP[10];
  char succ[3];
  char succ_IP[50];
  char succ_TCP[10];
  char succ_two[3];
  char succ_two_IP[50];
  char succ_two_TCP[10];
} node;

// EXTERN node self_node;

#endif