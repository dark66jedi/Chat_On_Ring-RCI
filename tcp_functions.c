#include "tcp_functions.h"

int TCP_Client(char connectIP[], char connectTCP[], char mensagem[], int sock_created, int fd)
// Enviar mensagens
{
  int errcode;
  ssize_t n;
  struct addrinfo hints, *res;
  ssize_t nbytes, nleft, nwritten;
  char *ptr, buffer[128];
  if (sock_created != 1)
  {
    memset(buffer, '\0', sizeof(buffer));
    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == -1)
    {
      printf("Erro na criação da ligação TCP");
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
      printf("Erro no connect\n"); 
      exit(1);
    }
  }

  ptr = strcpy(buffer, mensagem);
  nbytes = strlen(mensagem);
  nleft = nbytes; //Confirmar isto para não enviar o \0 -> Não, quando envias uma mensagem, não é para enviares um '\0' no final (ou seja, write(fd, "PRED 00\n", 8) está certo e write(fd, "PRED 00\n", 9) está errado)
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
  return fd;
}
