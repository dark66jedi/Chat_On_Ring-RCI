#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "udp_functions.h"
#include "tcp_functions.h"
#include "protocol_msgs.h"
#include "global_variables.h"
#include "aux_functions.h"
#include "select.h"

char regIP[30];
char regUDP[10];
char IP_global[30];
char TCP_global[10];
// node self_node;

int main(int argc, char *argv[])
{

  char readInter[128];
  node *n = malloc(sizeof(node));

  if (argc == 5 || argc == 3)
  {
    if (argc == 5)
    {
      strcpy(IP_global, argv[1]);
      strcpy(TCP_global, argv[2]);
      strcpy(regIP, argv[3]);
      strcpy(regUDP, argv[4]);
    }
    else
    {
      strcpy(IP_global, argv[1]);
      strcpy(TCP_global, argv[2]);
      strcpy(regIP, "193.136.138.142");
      strcpy(regUDP, "59000");
    }

    n->chord_bool = 0;
    n->ring_bool = 0;
    Interface(n);
    impl_select(n);
  }
  else
  {
    printf("Erro na inicialização do programa\n"); //ADICIONAR sugestões de correção
  }
  return 0;
}