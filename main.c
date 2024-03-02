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
#define max(A,B) ((A)>=(B)?(A):(B))

typedef struct node
{
  int self;       // ID
  char* self_IP;  // IP da máquina
  char* self_TCP; // porto
  int pred;
  char* pred_IP;
  char* pred_TCP;
  int succ;
  char* succ_IP;
  char* succ_TCP;
  int succ_two;
  char* succ_two_IP;
  char* succ_two_TCP;
} node;


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


//FUnção para enviar mensagens UDP
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
  n = recvfrom(fd, buffer, 128, 0,(struct sockaddr *)&addr, &addrlen);
  if (n == -1) /*error*/
    exit(1);
  // write(1, buffer, n);
  freeaddrinfo(res);
  close(fd);
  return buffer;
}

void join(char regIP[], char regUDP[], char ring[], node *n)
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
  if(strcmp(after,"\0")==0){ //caso em que não há ainda nós no anel

  } else{
    sscanf(after, "%s %s %s", connectid, connectIP, connectTCP);
  }
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
  //printf("%s", mensagemTCP);
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

void impl_select(char IP[], char TCP[], char regIP[], char regUDP[]){
  fd_set rfds;
  struct addrinfo hints,*res;
  struct sockaddr addr;
  int maxfd, tcpfd,udpfd,errcode_udp, errcode_tcp;
  int counter, nread;
  char buffer [128];
  socklen_t addrlen;
  char readInter[128];
  char modo[15], ring[4];
  int *id = 0;

  //socket udp (receber informação do tejo)
  udpfd = socket(AF_INET,SOCK_DGRAM,0);
  if(udpfd==-1){
    printf("Erro na criação da socket udp");
    exit(1);
  }
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_DGRAM;//UDP socket
  hints.ai_flags=AI_PASSIVE;

  if((errcode_udp=getaddrinfo(NULL,"58001",&hints,&res))!=0){//trocar "58001" pela variável que tiver o porto
    printf("Erro getaddrinfo udp");
    exit(1); 
  } 
  if(bind(udpfd,res->ai_addr,res->ai_addrlen)==-1){
    printf("Erro bind udp");
    exit(1);
  }

  //socket tcp (receber informação de outros nós)
  if((tcpfd=socket(AF_INET,SOCK_STREAM,0))==-1)exit(1);
  if(tcpfd==-1){
    printf("Erro na criação da socket tcp");
    exit(1);
  }

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket
  hints.ai_flags=AI_PASSIVE;
  if((errcode_tcp=getaddrinfo(NULL,"58001",&hints,&res))!=0){ //atualizar porto
    printf("Erro getaddrinfo tcp");
    exit(1);
  }

  if(bind(tcpfd,res->ai_addr,res->ai_addrlen)==-1){
    printf("Erro bind tcp");
    exit(1);
  }

  if(listen(tcpfd,5)==-1){
    printf("Erro listen tcp");
    exit(1);
  }

  maxfd = max(tcpfd, udpfd); //se depois tivermos mais, atualizar
  while(1){ //ACTUALLY select
      FD_ZERO(&rfds);
      FD_SET(0,&rfds);
      FD_SET(tcpfd,&rfds);
      FD_SET(udpfd,&rfds);
      printf("3");
      counter = select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
      //The select function returns the number of file descriptors (the number of bits from readfds, writefds, and errorfds).
      if (counter == 0){ //caso não se leia nada-> leave?

      }
      if(FD_ISSET(0,&rfds)){
        //coisas no terminal
        printf("Entrámos aqui?");
        //addrlen=sizeof(addr);
        //printf("Okau?");
        //memset(readInter,'\0',128); //memset(void *str, int c, size_t n) copies the character c to the first n characters of the string pointed to, by the argument str.
        if(fgets(readInter,sizeof (readInter), stdin)==NULL){
          printf("AQUI");
        };
        //printf("2");
        //sscanf(readInter, "%s", modo);
        //printf("3");
        /*if(strcmp(modo,"join")==0 || strcmp(modo,"j")==0){
          printf("Entrámso aqui?");
          sscanf(readInter, "%s %s %d", modo, ring, id);
          //join(regIP, regUDP, ring, n);
        }*/
      }
      if(FD_ISSET(tcpfd,&rfds)){
        //comunicação de outros nós
      printf("Funcionalidade por desenvolver: TCP");
      exit(1);
      }
      if(FD_ISSET(udpfd,&rfds)){ //confirmar se faz sentido, porque em principio, o tejo nunca fala connpscp sem que nós perguntemos algo
        //comunicação do servidor
        printf("Funcionalidade por desenvolver: UDP");
        addrlen = sizeof(addr);
        memset(buffer,'\0',128);
        if((nread=recvfrom(udpfd,buffer,128,0,&addr,&addrlen))==-1){ //Fazer algo com o buffer, mas não preciso de fazer o sendto que está nos slides, se não me engano
          printf("Erro na leitura UDP");
          exit(1);
        }
        //aqui, depois, acho que só precisamos de fazer algo se a responder for o NODESLIST
        //no OFREG e o OKUNREG acho que não se faz nada.
      }
    }
}

int main(int argc, char *argv[])
{

  char IP[50], TCP[50];
  char regIP[50], regUDP[50];
  char readInter[128];
  node *n;
  char modo[15], ring[4];
  int *id = 0;

  if (argc == 5 || argc == 3)
  {
    if(argc == 5){
      strcpy(IP, argv[1]);
      strcpy(TCP, argv[2]);
      strcpy(regIP, argv[3]);
      strcpy(regUDP, argv[4]);
    } else{
      strcpy(IP, argv[1]);
      strcpy(TCP, argv[2]);
      strcpy(regIP, "193.136.138.142");
      strcpy(regUDP, "59000");
    }
    Interface();
    fgets(readInter,sizeof (readInter), stdin);
    printf("1");
    sscanf(readInter, "%s", modo);
    printf("2");
    if(strcmp(modo,"join")==0 || strcmp(modo,"j")==0){
      printf("Entrámso aqui?");
      sscanf(readInter, "%s %s %d", modo, ring, id);
      n->self = id;
      strcpy(n->self_IP,IP);
      strcpy(n->self_TCP,TCP);
      join(regIP, regUDP, ring, n);
    }
    //impl_select(IP, TCP, regIP, regUDP);
  }
  else
  {
    fprintf(stderr, "Erro\n"); /*ADICIONAR sugestões de correção*/
  }
  return 0;
}