#include "select.h"
#include "user_cmds.h"

void impl_select(node *node)
{
  fd_set rfds;
  struct addrinfo hints, *res;
  int maxfd, newfd, tcpfd, udpfd, errcode_udp, errcode_tcp;
  struct sockaddr addr;
  socklen_t addrlen;
  // char *ptr;
  int counter, nread, n;
  char buffer[128];

  // int *id = 0;

  // socket udp (receber informação do tejo)
  udpfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (udpfd == -1)
  {
    printf("Erro na criação da socket udp");
    exit(1);
  }
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;      // IPv4
  hints.ai_socktype = SOCK_DGRAM; // UDP socket
  hints.ai_flags = AI_PASSIVE;

  if ((errcode_udp = getaddrinfo(IP_global, TCP_global, &hints, &res)) != 0)
  {
    printf("Erro getaddrinfo udp");
    exit(1);
  }
  if (bind(udpfd, res->ai_addr, res->ai_addrlen) == -1)
  {
    printf("Erro bind udp");
    exit(1);
  }

  // socket tcp (receber informação de outros nós)
  if ((tcpfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    exit(1);
  if (tcpfd == -1)
  {
    printf("Erro na criação da socket tcp");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP socket
  hints.ai_flags = AI_PASSIVE;
  if ((errcode_tcp = getaddrinfo(IP_global, TCP_global, &hints, &res)) != 0)
  {
    printf("Erro getaddrinfo tcp");
    exit(1);
  }

  if (bind(tcpfd, res->ai_addr, res->ai_addrlen) == -1)
  {
    printf("Erro bind tcp");
    exit(1);
  }

  if (listen(tcpfd, 5) == -1)
  {
    printf("Erro listen tcp");
    exit(1);
  }

  maxfd = max(tcpfd, udpfd); // se depois tivermos mais, atualizar
  while (1)
  { // ACTUALLY select
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(tcpfd, &rfds);
    FD_SET(udpfd, &rfds);

    // Show Interface for each command
    Interface(node);

    counter = select(maxfd + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);

    // The select function returns the number of file descriptors (the number of bits from readfds, writefds, and errorfds).
    if (counter == 0)
    { // caso não se leia nada-> leave?
    }
    if (FD_ISSET(0, &rfds))
    {
      // coisas no terminal
      FD_CLR(0,&rfds);
      choose_terminal_cmd(node);
    }
    if (FD_ISSET(tcpfd, &rfds))
    {
      // comunicação de outros nós
      FD_CLR(tcpfd,&rfds);
      addrlen = sizeof(addr);
      if ((newfd = accept(tcpfd, &addr, &addrlen)) == -1)
      {
        printf("Erro no accept");
        exit(1);
      }
      while ((n = read(newfd, buffer, 128)) != 0)
      {
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        // identificar o protocolo recebido
        choose_received_protocol(node, buffer);
      }
      close(newfd);
      close(tcpfd);
    }
    if (FD_ISSET(udpfd, &rfds))
    { // confirmar se faz sentido, porque em principio, o tejo nunca fala connosco sem que nós perguntemos algo
      // comunicação do servidor
      // printf("Funcionalidade por desenvolver: UDP");
      FD_CLR(udpfd,&rfds);
      addrlen = sizeof(addr);
      memset(buffer, '\0', 128);
      if ((nread = recvfrom(udpfd, buffer, 128, 0, &addr, &addrlen)) == -1)
      { // Fazer algo com o buffer, mas não preciso de fazer o sendto que está nos slides, se não me engano
        printf("Erro na leitura UDP");
        exit(1);
      }
      // aqui, depois, acho que só precisamos de fazer algo se a responder for o NODESLIST
      // no OFREG e o OKUNREG acho que não se faz nada.
    }
  }
}

void choose_terminal_cmd(node *node)
{
  char readInter[128];
  char modo[15], modo2[15];
  char ring[4];
  char id[3], dest[3];
  char succid[3], succIP[30], succTCP[10];
  char message[129];

  if (fgets(readInter, sizeof(readInter), stdin) == NULL)
  {
    exit(0);
  };
  sscanf(readInter, "%s", modo);

  // Comando join
  if (strcmp(modo, "join") == 0 || strcmp(modo, "j") == 0)
  {
    if (node->ring_bool == 1)
    {
      printf("\n=== Nó já pertence a um anel! ===\n");
      return;
    }
    sscanf(readInter, "%s %s %s", modo, ring, id);
    strcpy(node->self, id);
    strcpy(node->self_IP, IP_global);
    strcpy(node->self_TCP, TCP_global);
    join(ring, node);
  }

  // Comando direct join
  else if (strcmp(modo, "direct") == 0 || strcmp(modo, "dj") == 0)
  {
    sscanf(readInter, "%s %s %s %s %s %s", modo, modo2, id, succid, succIP, succTCP);
    if ((strcmp(modo2, "join") == 0) || (strcmp(modo, "dj") == 0))
    {
      if (node->ring_bool == 1)
      {
        printf("\n=== Nó já pertence a um anel! ===\n");
        return;
      }
      // direct_join();
    }
    else
    {
      printf("\n=== Comando não reconhecido! ===\n");
      return;
    }
  }

  // Comando chord
  else if (strcmp(modo, "chord") == 0 || strcmp(modo, "c") == 0)
  {
    if (node->chord_bool == 1)
    {
      printf("\n=== Nó já tem uma corda! ===\n");
      return;
    }
    // chord();
  }

  // Comando remove chord
  else if ((strcmp(modo, "remove") == 0) || (strcmp(modo, "rc") == 0))
  {
    sscanf(readInter, "%s %s", modo, modo2);
    if ((strcmp(modo2, "chord") == 0) || (strcmp(modo, "rc") == 0))
    {
      if (node->chord_bool == 0)
      {
        printf("\n=== Nó não tem uma corda! ===\n");
        return;
      }
      // remove_chord();
    }
    else
    {
      printf("\n=== Comando não reconhecido! ===\n");
      return;
    }
  }

  // Comando show topology
  else if ((strcmp(modo, "show") == 0) || (strcmp(modo, "st") == 0))
  {
    sscanf(readInter, "%s %s", modo, modo2);
    if ((strcmp(modo2, "topology") == 0) || (strcmp(modo, "st") == 0))
    {
      show_topology(node, 0);
    }
    else
    {
      printf("\n=== Comando não reconhecido! ===\n");
      return;
    }
  }

  // Comando show routing
  else if ((strcmp(modo, "show") == 0) || (strcmp(modo, "sr") == 0))
  {
    sscanf(readInter, "%s %s %s", modo, modo2, dest);
    if ((strcmp(modo2, "routing") == 0) || (strcmp(modo, "sr") == 0))
    {
      // show_routing();
    }
    else
    {
      printf("\n=== Comando não reconhecido! ===\n");
      return;
    }
  }

  // Comando show path
  else if ((strcmp(modo, "show") == 0) || (strcmp(modo, "sp") == 0))
  {
    sscanf(readInter, "%s %s %s", modo, modo2, dest);
    if ((strcmp(modo2, "path") == 0) || (strcmp(modo, "sp") == 0))
    {
      // show_path();
    }
    else
    {
      printf("\n=== Comando não reconhecido! ===\n");
      return;
    }
  }

  // Comando show forwarding
  else if ((strcmp(modo, "show") == 0) || (strcmp(modo, "sf") == 0))
  {
    sscanf(readInter, "%s %s", modo, modo2);
    if ((strcmp(modo2, "forwarding") == 0) || (strcmp(modo, "sf") == 0))
    {
      show_topology(node, 0);
    }
    else
    {
      printf("\n=== Comando não reconhecido! ===\n");
      return;
    }
  }

  // Comando message
  else if (strcmp(modo, "message") == 0 || strcmp(modo, "m") == 0)
  {
    if (node->ring_bool == 0)
    {
      printf("\n=== Nó não pertence a um anel! ===\n");
      return;
    }
    sscanf(readInter, "%s %s %s", modo, dest, message);
    // message();
  }

  // Comando leave
  else if (strcmp(modo, "leave") == 0 || strcmp(modo, "l") == 0)
  {
    if (node->ring_bool == 0)
    {
      printf("\n=== Nó não pertence a um anel! ===\n");
      return;
    }
    // leave();
  }

  // Comando exit
  else if (strcmp(modo, "exit") == 0 || strcmp(modo, "x") == 0)
  {
    // exit_COR();
  }

  else
  {
    printf("\n=== Comando não reconhecido! ===\n");
    return;
  }
}

void choose_received_protocol(node *node, char buffer[])
{
  char protocol[128];

  sscanf(buffer, "%s", protocol);
  if (strcmp(protocol, "ENTRY") == 0)
  {
    receive_ENTRY();
    // sscanf(buffer, "%s %s %s %s", protocol, node->pred, node->pred_IP, node->pred_TCP);
  }
}