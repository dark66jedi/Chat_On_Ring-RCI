#include "tcp_functions.h"

int TCP_Client(char connectIP[], char connectTCP[], char mensagem[], int sock_created, int fd)
// Enviar mensagens
{
  int errcode;
  ssize_t n;
  struct addrinfo hints, *res;
  ssize_t nbytes, nleft, nwritten;
  char *ptr, buffer[128];
  fprintf(stdout, "Entrei no TCP\n");
  if (sock_created != 1)
  {
    fprintf(stdout, "soube entrar no if\n");
    memset(buffer, '\0', sizeof(buffer));
    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == -1)
    {
      printf("Erro fd");
      exit(1);
    }
  fprintf(stdout, "ainda não deu segmentation 1\n");
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    // printf("dentro do client: %s %s", connectIP, connectTCP);
    errcode = getaddrinfo(connectIP, connectTCP, &hints, &res);
    if (errcode != 0)
    {
      printf("Erro errcode"); // ERRO AQUI
      //return 9; -> utilizei para poder fazer st e ver o que não é feito
      exit(1);
    }
    fprintf(stdout, "ainda não deu segmentation 2\n");

    // printf("\nconnectIP: %s", connectIP);
    // printf("\nconnectTCP: %s\n", connectTCP);
    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1)
    {
      printf("Erro no connect"); // pode ser por outra comunicação já estar a ser feito nessa socket?
      exit(1);
    }
    fprintf(stdout, "ainda não deu segmentation 13n");
  }

  //fprintf(stdout, "A fd para onde está a ser enviado está a ser: %d", fd);

  ptr = strcpy(buffer, mensagem);
  nbytes = strlen(mensagem);
  nleft = nbytes; //Confirmar isto para não enviar o \0 -> Não, quando envias uma mensagem, não é para enviares um '\0' no final (ou seja, write(fd, "PRED 00\n", 8) está certo e write(fd, "PRED 00\n", 9) está errado)
  fprintf(stdout, "pre write\n");
  while (nleft > 0)
  {
    //fprintf(stdout, "Entro num loop");
    nwritten = write(fd, ptr, nleft);
    if (nwritten <= 0)
    {
      printf("Erro no envio da mensagem TCP");
      exit(1);
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  // printf("mensagem tcp enviada: %s\n", mensagem);
  //fprintf(stdout, "Sai do TCP");
  return fd;
}
