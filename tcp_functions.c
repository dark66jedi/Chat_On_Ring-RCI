#include "tcp_functions.h"

void TCP_Client(char connectIP[], char connectTCP[], char mensagem[], int sock_created, int fd)
// Enviar mensagens
{
  int errcode;
  ssize_t n;
  struct addrinfo hints, *res;
  ssize_t nbytes, nleft, nwritten;
  char *ptr, buffer[128 + 1];
  if (sock_created != 1)
  {
    //fprintf(stdout, "soube entrar no if");
    memset(buffer, '\0', sizeof(buffer));
    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == -1)
    {
      printf("Erro fd");
      exit(1);
    }
  }
  //fprintf(stdout,"socket no tcp client: %d", fd);
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP socket
  // printf("dentro do client: %s %s", connectIP, connectTCP);
  errcode = getaddrinfo(connectIP, connectTCP, &hints, &res);
  if (errcode != 0)
  {
    printf("Erro errcode"); // ERRO AQUI
    exit(1);
  }

    printf("\nconnectIP: %s", connectIP);
   //printf("\nconnectTCP: %s\n", connectTCP);
  n = connect(fd, res->ai_addr, res->ai_addrlen);
  if (n == -1)
  {
    printf("Erro no connect"); //pode ser por outra comunicação já estar a ser feito nessa socket?
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
  //printf("mensagem tcp enviada: %s\n", mensagem);
  // freeaddrinfo(res); // não posso fazer já, tenho de manter a comunicação aberta
  // close(fd);         // não posso fazer já, tenho de mandter a comunicação aberta
  return;
}

void TCP_Server()
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
  errcode = getaddrinfo(NULL, "58001", &hints, &res); // O 58001 NÃO PODE FICAR ASSIM
  if ((errcode) != 0)                                 /*error*/
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
    // close(newfd);
  }
  return;
  // freeaddrinfo(res);
  // close(fd);
}
