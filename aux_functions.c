#include "aux_functions.h"

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
char *DeleteMethod(char *string)
{
  for (char *p = string;; ++p)
    if (*p == '\n')
      return p + 1;
}