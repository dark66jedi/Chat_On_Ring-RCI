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

void Interface()
{
  // falta adicionar outras coisas
  fprintf(stdout, "\n Lista de comandos:");
  fprintf(stdout, "\n \u2022 join ring id - Entrada de um nó no anel ring com identificador id");
  fprintf(stdout, "\n \u2022 direct join ring id succid succIP succTCP - Entrada de um nó no anel ring com identificador id ao nó com parâmetros succid, succIP e succTCP");
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
    if((newfd=accept(fd,(struct sockaddr*)&addr,
    &addrlen))==-1)
    /*error*/ exit(1);
    n=read(newfd,buffer,128);
    if(n==-1)/*error*/exit(1);
    //write(1,"received: ",10);
    //write(1,buffer,n);
    n=write(newfd,buffer,n);
    if(n==-1)/*error*/exit(1);
    close(newfd);
  }
  freeaddrinfo(res);
  close(fd);
}
void TCPClient(char connectIP[], char connectTCP[], char mensagem[])
// NOT SURE SE FUNCIONA YET porque ainda não temos nenhum nó na rede então não consigo abrir dois terminais e confirmar
// Na verdade, temos um bacano no nosso ring, acho
//Enviar mensagens
{
  int fd, errcode;
  ssize_t n;
  struct addrinfo hints, *res;
  char buffer[128];

  fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
  if (fd == -1)
    exit(1); // error
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP socket
  errcode = getaddrinfo(connectIP, connectTCP, &hints, &res); 
  if (errcode != 0) /*error*/
    exit(1);
  n = connect(fd, res->ai_addr, res->ai_addrlen);
  if (n == -1) /*error*/
    exit(1);
  n = write(fd, mensagem, strlen(mensagem));
  if (n == -1) /*error*/
    exit(1);
  n = read(fd, buffer, 128); //NAO SEI O QUE É QUE ISTO FAZ, TCP 
  if (n == -1) /*error*/
    exit(1);
  write(1, "echo: ", 6);
  write(1, buffer, n);
  freeaddrinfo(res);
  close(fd);
}

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
  if (errcode != 0) /*error*/
    exit(1);
  n = sendto(fd, mensagem, strlen(mensagem), 0, res->ai_addr, res->ai_addrlen);
  if (n == -1) /*error*/
    exit(1);
  addrlen = sizeof(addr);
  n = recvfrom(fd, buffer, 128, 0,
               (struct sockaddr *)&addr, &addrlen);
  if (n == -1) /*error*/
    exit(1);
  // write(1, buffer, n);
  freeaddrinfo(res);
  close(fd);
  return buffer;
}

void join(char regIP[], char regUDP[], char ring[], char id[])
{
  char mensagemUDP[30];
  char mensagemTCP[50];
  char answer[128];
  char connectid[3], connectIP[50], connectTCP[10];
  strcpy(mensagemUDP, "NODES ");
  strcat(mensagemUDP, ring);
  strcpy(answer, UDPClient(regIP, regUDP, mensagemUDP));
  // printf("%s", answer);
  char *after = DeleteMethod(answer);
  // printf("%s", after);
  sscanf(after, "%s %s %s", connectid, connectIP, connectTCP); //não estou a verificar o caso de não haver nenhum nó, IMPORTANTE FAZER porque é o primeiro passo que vamos ter de fazer
  // printf("%s %s %s", connectid, connectIP, connectTCP);
  strcpy(mensagemTCP,"ENTRY ");
  strcat(mensagemTCP, connectid);
  strcat(mensagemTCP, " ");
  strcat(mensagemTCP, connectIP);
  strcat(mensagemTCP, " ");
  strcat(mensagemTCP, connectTCP);
  strcat(mensagemTCP, "<LF>");
  printf("%s", mensagemTCP);
  TCPClient(connectIP, connectTCP, mensagemTCP);
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

int main(int argc, char *argv[])
{

  char IP[50], TCP[50];
  char regIP[50], regUDP[50];
  char modo[20], ring[4], id[3];
  char readInter[50];

  if (argc == 5)
  {
    strcpy(IP, argv[1]);
    strcpy(TCP, argv[2]);
    strcpy(regIP, argv[3]);
    strcpy(regUDP, argv[4]);
    Interface();
    fgets(readInter, 50, stdin);                   /*mostrar os comendos disponíveis*/
    sscanf(readInter, "%s %s %s", modo, ring, id); // não pdoemos usar assim para os outros modos, assumi assim só para fazer o join
    if (strcmp(modo, "join") == 0 || strcmp(modo, "j") == 0)
    {
      join(regIP, regUDP, ring, id);
    }
    else
    {
      printf("Funcionalidade por desenvolver");
    }
  }
  else
  {
    fprintf(stderr, "Erro\n"); /*ADICIONAR sugestões de correção*/
  }
  return 0;
}