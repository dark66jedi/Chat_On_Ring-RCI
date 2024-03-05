#include "select.h" 

void impl_select(char IP[], char TCP[], char regIP[], char regUDP[], node *no)
{
  fd_set rfds;
  struct addrinfo hints, *res;
  int maxfd, newfd, tcpfd, udpfd, errcode_udp, errcode_tcp;
  struct sockaddr addr;
  socklen_t addrlen;
  //char *ptr;
  char buffer[128];
  int counter, nread, n;
  char readInter[128];
  char protocolo[128];
  char modo[15], modo2[15];
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

  if ((errcode_udp = getaddrinfo(NULL, no->self_TCP, &hints, &res)) != 0)
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
  if ((errcode_tcp = getaddrinfo(NULL, no->self_TCP, &hints, &res)) != 0)
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
    counter = select(maxfd + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);
    // The select function returns the number of file descriptors (the number of bits from readfds, writefds, and errorfds).
    if (counter == 0)
    { // caso não se leia nada-> leave?
    }
    if (FD_ISSET(0, &rfds))
    {
      // coisas no terminal
      fgets(readInter, sizeof(readInter), stdin);
      sscanf(readInter, "%s", modo);
      if ((strcmp(modo, "show") == 0) || (strcmp(modo, "st") == 0))
      {
        sscanf(readInter, "%s %s", modo, modo2);
        if ((strcmp(modo, "topology") == 0) || (strcmp(modo, "st") == 0)){          
          show_topology(no, 0);
        }
      }
      // printf("Entrámos aqui? Okay se fiz printf");
      // addrlen=sizeof(addr);
      // memset(readInter,'\0',128); //memset(void *str, int c, size_t n) copies the character c to the first n characters of the string pointed to, by the argument str.
      if (fgets(readInter, sizeof(readInter), stdin) == NULL)
      {
        exit(0);
      };
      // sscanf(readInter, "%s", modo);
      /*if(strcmp(modo,"join")==0 || strcmp(modo,"j")==0){
        printf("Entrámso aqui?");
        sscanf(readInter, "%s %s %d", modo, ring, id);
        //join(regIP, regUDP, ring, n);
      }*/
    }
    if (FD_ISSET(tcpfd, &rfds))
    {
      // comunicação de outros nós
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
        //ptr = &buffer[0];
        sscanf(buffer, "%s", protocolo);
        if (strcmp(protocolo, "ENTRY") == 0)
        { // APENAS A FUNCIONAR PARA 2 NÓS: não faz a coisa de ir avisar o seu sucesor nem pred nem nada
          sscanf(buffer, "%s %s %s %s", protocolo, no->pred, no->pred_IP, no->pred_TCP);
          sscanf(buffer, "%s %s %s %s", protocolo, no->succ, no->succ_IP, no->succ_TCP);
          strcpy(no->succ_two, no->self);
          strcpy(no->succ_two_IP, no->self_IP);
          strcpy(no->succ_two_TCP, no->self_TCP);
        }
      }
      close(newfd);
      close(tcpfd);
    }
    if (FD_ISSET(udpfd, &rfds))
    { // confirmar se faz sentido, porque em principio, o tejo nunca fala connosco sem que nós perguntemos algo
      // comunicação do servidor
      //printf("Funcionalidade por desenvolver: UDP");
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