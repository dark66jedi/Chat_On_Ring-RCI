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
      return -1;
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    errcode = getaddrinfo(connectIP, connectTCP, &hints, &res);
    if (errcode != 0)
    {
      printf("Erro errcode"); 
      close(fd);
      return -1;
    }

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1)
    {
      printf("Erro no connect\n"); 
      close(fd);
      return -1;
    }
  }

  ptr = strcpy(buffer, mensagem);
  nbytes = strlen(mensagem);
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
  return fd;
}
