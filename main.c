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

#include "udp_functions.h"
#include "tcp_functions.h"
#include "protocol_msgs.h"
#include "global_variables.h"
#include "user_cmds.h"
#include "aux_functions.h"

char regIP[30];
char regUDP[10];
char IP[30];
char TCP[10];
// node self_node;

int main(int argc, char *argv[])
{

  char readInter[128];
  node *n = malloc(sizeof(node));
  char modo[15];
  char ring[4];
  char id[3];

  if (argc == 5 || argc == 3)
  {
    if (argc == 5)
    {
      strcpy(IP, argv[1]);
      strcpy(TCP, argv[2]);
      strcpy(regIP, argv[3]);
      strcpy(regUDP, argv[4]);
      printf("%s %s", IP, TCP);
    }
    else
    {
      strcpy(IP, argv[1]);
      strcpy(TCP, argv[2]);
      strcpy(regIP, "193.136.138.142");
      strcpy(regUDP, "59000");
      printf("%s %s", IP, TCP);
    }
    Interface();
    printf("Choose the mode\n");
    //scanf("%s %s %s", modo, ring, id);
    fgets(readInter, sizeof(readInter), stdin);
    sscanf(readInter, "%s", modo);
    //printf("%s",modo);
    //retirei o if e já não deu segmentation fault portanto não é antes disto
    //printf("%s(%d)\n", modo, strlen(modo));
    if (strcmp(modo, "join") == 0 || strcmp(modo, "j") == 0)
    {
      //printf("entrei if");
      sscanf(readInter, "%s %s %s", modo, ring, id);
      strcpy(n->self, id);
      //printf("%s",n->self);
      strcpy(n->self_IP, IP);
      strcpy(n->self_TCP, TCP);
      join(ring, n);
    }
  }
  else
  {
    printf("Erro na inicialização do programa\n"); //DICIONAR sugestões de correção
  }
  return 0;
}