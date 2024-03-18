#include "aux_functions.h"

void Interface(node *node)
{
  // falta adicionar outras coisas
  fprintf(stdout, "\nLista de comandos:\n");
  if (node->ring_bool == 0)
  {
    fprintf(stdout, "\n \u2022 join (j) ring id - Entrada de um nó no anel ring com identificador id");
    fprintf(stdout, "\n \u2022 direct join (dj) id succid succid succIP succTCP - Entrada de um nó no anel ring com identificador id ao nó com parâmetros succid, succIP e succTCP");
  }
  else if (node->ring_bool == 1)
  {
    fprintf(stdout, "\n \u2022 2ª interface");
    if (node->chord_bool == 0)
    {
      fprintf(stdout, "\n \u2022 chord (c) - Estabelicimento de uma corda com outro nó");
    }
    else if (node->chord_bool == 1)
    {
      fprintf(stdout, "\n \u2022 remove chord (rc) - Eliminação da corda");
    }
   //só para não aparecer uma mancha tão grande no código enquanto testo
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

char *find_next_protocol(char *buffer) {
    char *newline_ptr = strchr(buffer, '\n');
    if (newline_ptr != NULL) {
        *newline_ptr = '\0';  // Replace newline with null terminator
        return newline_ptr + 1;  // Return pointer to the next message
    } else {
        return NULL;  // No more messages found
    }
}

void clean_node(node *node){
  node->ring[0] = '\0';
  node->self[0] = '\0' ;      
  node->self_IP[0] = '\0';  
  node->self_TCP[0] = '\0'; 
  node->pred[0] = '\0';
  node->succ[0] = '\0';
  node->succ_IP[0] = '\0';
  node->succ_TCP[0] = '\0';
  node->succ_two[0] = '\0';
  node->succ_two_IP[0] = '\0';
  node->succ_two_TCP[0] = '\0';
  node->chord[0] = '\0';
  node->chord_IP[0] = '\0';
  node->chord_TCP[0] = '\0';
  node->pred_left_bool = 0;
  node->ring_bool = 0;
  node->chord_bool = 0;                  
  node->n_chords = 0;                    
  node->pred_fd = -1;
  node->succ_fd = -1;
  node->chord_fd = -1;
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

void search_index(node *node, char node_id[], int index)
{
  index = -1;
  for (size_t i = 0; i < MAX_N_NODES; i++)
  {
    if(strcmp(node_id, node->table_indexes[i]) == 0)
    {
      index = i;
      break;
    }
  }
}

void check_for_new_node(node *node, char path[])
{
  char node_to_check[3] = "";
  int i = 0, path_i = 0;
  int index;
  char result[100];

  while(path_i < strlen(path))
  {
    i = 0;
    node_to_check[i] = path[path_i];
    i++;
    path_i++;
    node_to_check[i] = path[path_i];
    i++;
    path_i++;

    search_index(node, node_to_check, index);

    if (index == -1)                        // novo nó no anel
    {
        strcpy(node->table_indexes[node->filling_table_index], node_to_check);

        node->filling_table_index = node->filling_table_index + 1;
    }

    if (path_i>(strlen(path)-3)) break;
  
    path_i++;
  }
  strcpy(path,result);
}

void initialize_tables(node *node)
{
    int i, k;
    char buffer[50];

    for (i = 0; i < MAX_N_NODES; i++)
    {
        for (k = 0; k < MAX_N_NODES; k++)
        {
            strcpy(node->routing_table[i][k], "-");
        }
    }

    // registra-se a ele próprio no table_indexes
    strcpy(node->table_indexes[node->filling_table_index], node->self);
    node->filling_table_index = node->filling_table_index + 1;

    if ((strcmp(node->pred, node->self) != 0))
    {
        strcpy(node->table_indexes[node->filling_table_index], node->pred);
        sprintf(buffer, "%s-%s", node->self, node->pred);
        strcpy(node->routing_table[node->filling_table_index][node->filling_table_index], buffer);
        node->filling_table_index = node->filling_table_index + 1;
    }
    if ((strcmp(node->succ, node->self) != 0) && (strcmp(node->pred, node->succ) != 0))
    {
        strcpy(node->table_indexes[node->filling_table_index], node->succ);
        sprintf(buffer, "%s-%s", node->self, node->succ);
        strcpy(node->routing_table[node->filling_table_index][node->filling_table_index], buffer);
        node->filling_table_index = node->filling_table_index + 1;
    }
    
}

void update_neighbour(node *node, char old_neighbour[], char new_neighbour[])
{
  int old_neighbour_index, new_neighbour_index;

  if(old_neighbour != NULL)
  {
    search_index(node, old_neighbour, old_neighbour_index);

    for (size_t i = 0; i < MAX_N_NODES; i++)
    {
      strcpy(node->routing_table[i][old_neighbour_index], "-");
    }
  }
  
  strcpy(node->table_indexes[node->filling_table_index], new_neighbour);
  node->filling_table_index = node->filling_table_index + 1;
}

void update_tables(node *node)
{
  // first update short paths table with routing table
  char short_path[50] = "-";

  for (size_t i = 0; i < MAX_N_NODES; i++)
  {
    for (size_t k = 0; k < MAX_N_NODES; k++)
    {
      if(strcmp(node->routing_table[i][k], "-") == 0) continue;
      else
      {
        if (strlen(short_path) > strlen(node->routing_table[i][k]) || strcmp(short_path, "-") == 0)
        {
          strcpy(short_path, node->routing_table[i][k]);
        }
      }
    }
    strcpy(node->short_paths_table[i], short_path);
  }

  // then update expedition table with short paths table

  for (size_t i = 0; i < MAX_N_NODES; i++)
  {
    if(strcmp(node->expedition_table[i], "-") == 0) continue;
    else
    {
      node->expedition_table[i][0] = node->short_paths_table[i][3];
      node->expedition_table[i][1] = node->short_paths_table[i][4];
      node->expedition_table[i][2] = '\0';
    }
  }  
}

