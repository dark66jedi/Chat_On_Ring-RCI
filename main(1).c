#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORT "58001"
#define max(A, B) ((A) >= (B) ? (A) : (B))

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

void impl_select(char IP[], char TCP[], char regIP[], char regUDP[], node *n);

void Interface()
{
  // falta adicionar outras coisas
  fprintf(stdout, "\n Lista de comandos:");
  fprintf(stdout, "\n \u2022 join ring id - Entrada de um nó no anel ring com identificador id");
 // fprintf(stdout, "\n \u2022 direct join ring id succid succIP succTCP - Entrada de um nó no anel ring com identificador id ao nó com parâmetros succid, succIP e succTCP");
  fprintf(stdout, "\n \u2022 show topology - mostra o estado do protocolo topológico\n");
}

/*  Deletion method 0:  Find the first character that is not an "X" and
    return its address.*/
static char *DeleteMethod(char *string)
{
  for (char *p = string;; ++p)
    if (*p == '\n')
      return p + 1;
}

void TCPServer()
{
  // ainda não tenho a certeza se é assim mas suponho que precisemos quando outros nós estão a falar connosco
  //  é preciso incluir isto no select

  int fd, errcode, newfd;
  ssize_t n;
  socklen_t addrlen;
  struct addrinfo hints, *res;
  struct sockaddr_in addr;
  char buffer[128];

  fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
  if (fd == -1)
    exit(1); // error
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP socket
  hints.ai_flags = AI_PASSIVE;
  errcode = getaddrinfo(NULL, PORT, &hints, &res);
  if ((errcode) != 0) /*error*/
    exit(1);
  n = bind(fd, res->ai_addr, res->ai_addrlen);
  if (n == -1) /*error*/
    exit(1);
  if (listen(fd, 5) == -1) /*error*/
    exit(1);
  while (1)
  {
    addrlen = sizeof(addr);
    if ((newfd = accept(fd, (struct sockaddr *)&addr,
                        &addrlen)) == -1)
      /*error*/ exit(1);
    n = read(newfd, buffer, 128);
    if (n == -1) /*error*/
      exit(1);
    // write(1,"received: ",10);
    // write(1,buffer,n);
    n = write(newfd, buffer, n);
    if (n == -1) /*error*/
      exit(1);
    close(newfd);
  }
  freeaddrinfo(res);
  close(fd);
}

void TCPClient(char connectIP[], char connectTCP[], char mensagem[])
// Enviar mensagens
{
  int fd, errcode;
  ssize_t n;
  struct addrinfo hints, *res;
  ssize_t nbytes, nleft, nwritten;
  char *ptr, buffer[128 + 1];

  memset(buffer, '\0', sizeof(buffer));
  fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
  if (fd == -1)
  {
    printf("Erro fd");
    exit(1);
  }
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP socket
  errcode = getaddrinfo(connectIP, connectTCP, &hints, &res);
  if (errcode != 0)
  {
    printf("Erro errcode");
    exit(1);
  }
  n = connect(fd, res->ai_addr, res->ai_addrlen);
  if (n == -1)
  {
    printf("Erro no connect");
    exit(1);
  }

  ptr = strcpy(buffer, mensagem);
  nbytes = strlen(buffer);
  nleft = nbytes;
  while (nleft > 0)
  {
    nwritten = write(fd, ptr, nleft);
    if (nwritten <= 0)
    {
      printf("Erro no envio da mensagem TCP");
      exit(1);
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  freeaddrinfo(res); // não posso fazer já, tenho de manter a comunicação aberta
  close(fd);         // não posso fazer já, tenho de mandter a comunicação aberta
  return;
}

// Função para enviar mensagens UDP
char *UDPClient(char regIP[], char regUDP[], char mensagem[])
{
  int fd, errcode;
  ssize_t n;
  socklen_t addrlen;
  struct addrinfo hints, *res;
  struct sockaddr_in addr;
  static char buffer[128];

  fd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
  if (fd == -1)                        /*error*/
    exit(1);
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;      // IPv4
  hints.ai_socktype = SOCK_DGRAM; // UDP socket
  errcode = getaddrinfo(regIP, regUDP, &hints, &res);
  memset(buffer, '\0', 128);
  if (errcode != 0) /*error*/
    exit(1);
  n = sendto(fd, mensagem, strlen(mensagem), 0, res->ai_addr, res->ai_addrlen);
  if (n == -1) /*error*/
    exit(1);
  addrlen = sizeof(addr);
  n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
  if (n == -1) /*error*/
    exit(1);
  // write(1, buffer, n);
  freeaddrinfo(res);
  close(fd);
  return buffer;
}

void join(char regIP[], char regUDP[], char ring[], node *n)
{
  // 2º nó está a entrar no join
  char mensagemUDP[50];
  char mensagemTCP[50];
  char answer[500];
  char connectid[3], connectIP[50], connectTCP[10];
  int new=0;
  // int i = 0;

  //printf("entrei join");
  strcpy(mensagemUDP, "NODES ");
  strcat(mensagemUDP, ring);
  strcpy(answer, UDPClient(regIP, regUDP, mensagemUDP)); // receber NODESList
  char *after = DeleteMethod(answer);
  if (strcmp(after, "\0") == 0)
  { // caso em que não há ainda nós no anel
    strcpy(mensagemUDP, "REG ");
    strcat(mensagemUDP, ring);
    strcat(mensagemUDP, " ");
    strcat(mensagemUDP, n->self); // id
    strcat(mensagemUDP, " ");
    strcat(mensagemUDP, n->self_IP);
    strcat(mensagemUDP, " ");
    strcat(mensagemUDP, n->self_TCP);
    memset(answer, '\0', sizeof(answer));
    if (strcmp(UDPClient(regIP, regUDP, mensagemUDP), "OKREG") != 0)
    {
      printf("Erro no registo do nó no anel");
      exit(1);
    }
    impl_select(n->self_IP, n->self_TCP, regIP, regUDP, n);
  }
  else // VAI SER PRECISO MAIS UM IF, ESTE SÓ FUNCIONA PARA 2 NÓS
  {
    // 2º nó está a entrar aqui
    /*
    1. Guarda lista de nós disponíveis -> a fazer para mais do que 2 nós
    2. Comparar ids (se iguall, dar i+1, voltar a comparar, etc, algo recursivo) ->a fazer para mais do que 2 nós
    3. id escolhido, comparar IPs e porto? -> a fazer para mais do que 2 nós
    4. escolher o primeiro nó na lista, para ser esse a que se liga -> a fazer para mais do que 2 nós
    5. implement select
    */
    sscanf(after, "%s %s %s", connectid, connectIP, connectTCP); // escolher o primeiro nó na lista, para ser esse a que se liga
    if (strcmp(connectid,n->self)==0){
      new = atoi (n->self);
      new = new+1;
      if (new<10){
      sprintf(n->self,"0%d",new);
      }
    }
    //printf("%s", n->self);
    strcpy(mensagemTCP, "ENTRY ");
    strcat(mensagemTCP, n->self);
    strcat(mensagemTCP, " ");
    strcat(mensagemTCP, n->self_IP);
    strcat(mensagemTCP, " ");
    strcat(mensagemTCP, n->self_TCP);
    strcat(mensagemTCP, "\n");
    //printf("%s", mensagemTCP);
    TCPClient(connectIP, connectTCP, mensagemTCP); // mensagem a ser enviada está correta
    // sucessor
    strcpy(n->succ, connectid);
    strcpy(n->succ_IP, connectIP);
    strcpy(n->succ_TCP, connectTCP);
    // predecessor
    strcpy(n->pred, connectid);
    strcpy(n->pred_IP, connectIP);
    strcpy(n->pred_TCP, connectTCP);
    // segundo sucessor
    strcpy(n->succ_two, n->self);
    strcpy(n->succ_two_IP, n->self_IP);
    strcpy(n->succ_two_TCP, n->self_TCP);
    // registar servidor de nós
    strcpy(mensagemUDP, "REG ");
    strcat(mensagemUDP, ring);
    strcat(mensagemUDP, " ");
    strcat(mensagemUDP, n->self); // id
    strcat(mensagemUDP, " ");
    strcat(mensagemUDP, n->self_IP);
    strcat(mensagemUDP, " ");
    strcat(mensagemUDP, n->self_TCP);
    memset(answer, '\0', sizeof(answer));
    if (strcmp(UDPClient(regIP, regUDP, mensagemUDP), "OKREG") != 0)
    {
      printf("Erro no registo do nó no anel");
      exit(1);
    }
    impl_select(n->self_IP, n->self_TCP, regIP, regUDP, n);
  }

  /*
  1.CONECÇÃO UDP NODES feito
 2. Receber NODeslist e escolher um (até pode ser o primeiro) Feito
 3.ENTRY TCP
 4. SUCC TCP
 5. SUCC TCP
 6. PRED
 7. reg
 8. OKAYREG
  */
}

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
          printf("Estado do protocolo topológico:\n");
          printf("Próprio anel:\n");
          printf("\u2022 id: %s \n",no->self);
          printf("\u2022 IP: %s \n",no->self_IP);
          printf("\u2022 Porto: %s \n\n",no->self_TCP);

          printf("Seu sucessor:\n");
          printf("\u2022 id: %s \n",no->succ);
          printf("\u2022 IP: %s \n",no->succ_IP);
          printf("\u2022 Porto: %s \n\n",no->succ_TCP);

          printf("Seu segundo-sucessor:\n");
          printf("\u2022 id: %s \n",no->succ_two);
          printf("\u2022 IP: %s \n",no->succ_two_IP);
          printf("\u2022 Porto: %s \n\n",no->succ_two_TCP);
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

int main(int argc, char *argv[])
{

  char IP[50], TCP[50];
  char regIP[50], regUDP[50];
  char readInter[128];
  node *n = malloc(sizeof(node));
  //= {"\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0"
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
    fgets(readInter, sizeof(readInter)+1, stdin);
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
      join(regIP, regUDP, ring, n);
    }
  }
  else
  {
    printf("Erro na inicialização do programa\n"); //DICIONAR sugestões de correção
  }
  return 0;
}