#include "aux_functions.h"

void Interface(node *node)
{
  // falta adicionar outras coisas
  fprintf(stdout, "\nLista de comandos:\n");
  if (node->ring_bool == 0)
  {
    fprintf(stdout, "\n \u2022 join (j) ring id - Entrada de um nó no anel ring com identificador id");
    fprintf(stdout, "\n \u2022 direct join (dj) ring id succid succIP succTCP - Entrada de um nó no anel ring com identificador id ao nó com parâmetros succid, succIP e succTCP");
  }
  else if (node->ring_bool == 1)
  {
    if (node->chord_bool == 0)
    {
      fprintf(stdout, "\n \u2022 chord (c) - Estabelicimento de uma corda com outro nó");
    }
    else if (node->chord_bool == 1)
    {
      fprintf(stdout, "\n \u2022 remove chord (rc) - Eliminação da corda");
    }
    fprintf(stdout, "\n \u2022 2ª interface"); //só para não aparecer uma mancha tão grande no código enquanto testo
    /*fprintf(stdout, "\n \u2022 show topology (st) - Mostra o estado do protocolo topológico");
    fprintf(stdout, "\n \u2022 show routing (sr) dest - Mostra a entrada da tabela de encaminhamento para o nó dest");
    fprintf(stdout, "\n \u2022 show path (sp) dest - Mostra o caminho mais curto para o nó dest");
    fprintf(stdout, "\n \u2022 show forwarding (sf) - Mostra a tabela de expedição do próprio nó");
    fprintf(stdout, "\n \u2022 message (m) dest message - Envio da mensagem message para o nó dest");
    fprintf(stdout, "\n \u2022 leave (l) - Saída do nó do anel");*/
  }

  fprintf(stdout, "\n \u2022 exit (x) - Fecho do programa");
  fprintf(stdout, "\n\nChoose the mode\n\n");
}

/*  Deletion method 0:  Find the first character that is not an "X" and
    return its address.*/
char *DeleteMethod(char *string)
{
  for (char *p = string;; ++p)
    if (*p == '\n')
      return p + 1;
}

char *DeleteMethod_space(char *string)
{
  for (char *p = string;; ++p)
    if (*p == ' ')
      return p + 1;
}

int max(int x, int y, int z, int j)
{
    int highest = x;

    if (y > highest) {
        highest = y;
    }
    if (z > highest) {
        highest = z;
    }
    if (j > highest) {
        highest = j;
    }
    return highest;
}