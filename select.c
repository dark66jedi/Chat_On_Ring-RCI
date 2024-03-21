#include "select.h"
#include "user_cmds.h"
#include <errno.h>

void impl_select(node *node)
{
  fd_set rfds;
  struct addrinfo hints, *res;
  int maxfd = 0, newfd = 0, fd = 0;
  int errcode_listen_tcp, errcode_pred_tcp, errcode_succ_tcp, errcode_chord_tcp;
  struct sockaddr addr;
  socklen_t addrlen;
  int counter = 0, nread = 0, n = 0;
  int lixo = 0;
  char buffer[500];
  char readInter[128];
  char modo[20];
  int i = 0;
  int fd_index = 0;

  /* socket de escuta tcp (receber informação de nós fora do ring)*/
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    printf("Erro na criação da socket tcp\n");
    perror("fd");
    exit(0);
  }
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP socket
  hints.ai_flags = AI_PASSIVE;
  if ((errcode_listen_tcp = getaddrinfo(NULL, TCP_global, &hints, &res)) != 0)
  {
    printf("Erro getaddrinfo tcp");
    perror("errcode_listen_tcp");
    exit(1);
  }
  if (bind(fd, res->ai_addr, res->ai_addrlen) == -1)
  {
    printf("Erro bind tcp escuta");
    exit(1);
  }
  if (listen(fd, 5) == -1)
  {
    printf("Erro listen tcp");
    exit(1);
  }
  while (1)
  { // ACTUALLY select
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(fd, &rfds);
    maxfd = max(fd, node->pred_fd, node->succ_fd, node->chord_fd, node->chord_rec);
    if (node->pred_fd > 0)
    {
      FD_SET(node->pred_fd, &rfds); // só se pode dar set se existir
    }
    if (node->succ_fd > 0)
    {
      FD_SET(node->succ_fd, &rfds);
    }
    if (node->chord_fd > 0)
    {
      FD_SET(node->chord_fd, &rfds);
    }
    for (size_t i = 0; i < MAX_N_NODES; i++)
    {
      if (node->chord_rec[i] > 0)
      {
        FD_SET(node->chord_rec[i], &rfds);
      }
    }

    counter = select(maxfd + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);
    if (counter == -1)
    {
      perror("Erro ao abrir o arquivo");
    }

    /* fd responsável pelo terminal****************************************/
    if (FD_ISSET(0, &rfds))
    {
      FD_CLR(0, &rfds);
      if (fgets(readInter, sizeof(readInter), stdin) != NULL)
      {
        sscanf(readInter, "%s", modo);
        fprintf(stdout, "ring bool: %d", node->ring_bool);
        while (node->ring_bool == 0)
        {
          if ((strcmp(modo, "direct") == 0) || (strcmp(modo, "dj") == 0) || (strcmp(modo, "join") == 0) || (strcmp(modo, "j") == 0) || (strcmp(modo, "exit") == 0) || (strcmp(modo, "x") == 0))
          {
            break;
          }
          else
          {
            fprintf(stdout, "Comando inválido. O nó ainda não faz parte de um anel. Por favor, escolha um comando válido. \n");
            Interface(node);
            fgets(readInter, sizeof(readInter), stdin);
            sscanf(readInter, "%s", modo);
          }
        }
      }
      if (strcmp(modo, "leave") == 0 || strcmp(modo, "l") == 0)
      {
        if (strcmp(node->self, node->pred) != 0) // 2+ nós
        {
          {
            if (strcmp(node->ring, "\0") != 0)
              leave(node);
          }
          close(node->pred_fd);
          close(node->succ_fd);
          if (node->chord_bool == 1)
          {
            close(node->chord_fd);
          }
          fprintf(stdout, "O nó %s saiu do anel.\n", node->self);
          clean_node(node);
          FD_ZERO(&rfds);
          freeaddrinfo(res);
          close(fd);
          return;
        }
        else if (strcmp(node->succ, node->pred) == 0 && strcmp(node->self, node->pred) == 0)
        { // leave:check if único node/////
          if (strcmp(node->ring, "\0") != 0)
          {
            leave(node);
          }
          fprintf(stdout, "O nó %s saiu do anel.", node->self);
          clean_node(node);
          FD_ZERO(&rfds);
          freeaddrinfo(res);
          close(fd);
          return;
        }
      }
      else if (strcmp(modo, "exit") == 0 || strcmp(modo, "x") == 0)
      {
        if (node->self != node->pred)
        {
          if (strcmp(node->ring, "\0") != 0)
          {
            leave(node);
          }
          clean_node(node);
          FD_ZERO(&rfds);
          freeaddrinfo(res);
          close(node->pred_fd);
          close(node->succ_fd);
          if (node->chord_bool == 1)
          {
            close(node->chord_fd);
          }
          close(fd);
          close(2);
          close(1);
          close(0);
          free(node);
          fprintf(stdout, "Saída da aplicação com sucesso.");
          exit(0);
        }
        else if ((node->succ == node->pred) && (node->self == node->pred))
        { /////EXIT:check if único node/////
          if (strcmp(node->ring, "\0") != 0)
          {
            leave(node);
          }
          clean_node(node);
          FD_ZERO(&rfds);
          freeaddrinfo(res);
          close(fd);
          close(2);
          close(1);
          close(0);
          free(node);
          fprintf(stdout, "Saída da aplicação com sucesso.");
          exit(0);
        }
      }
      else
      {
        choose_terminal_cmd(node, readInter);
      };
      Interface(node);
    }
    /**********************************************************************/

    /* fd responsável pelo canal do sucessor ******************************/
    if (FD_ISSET(node->succ_fd, &rfds))
    {
      memset(buffer, '\0', 500);
      FD_CLR(node->succ_fd, &rfds);
      while ((n = read(node->succ_fd, buffer, 500)) != 0)
      {
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        else
        {
          // identificar o protocolo recebido
          choose_received_protocol(node, buffer, node->succ_fd);
          break;
        }
      }
      if (n == 0)
      {
        if (strcmp(node->self, node->succ_two) == 0)
        {
          // caso 2 nós, 1 vai sair e vai ficar só 1
          strcpy(node->node_left, node->succ);
          strcpy(node->succ, node->succ_two);
          strcpy(node->succ_IP, node->succ_two_IP);
          strcpy(node->succ_TCP, node->succ_two_TCP);
          strcpy(node->pred, node->succ_two);
          close(node->pred_fd);
          close(node->succ_fd);
          if (node->chord_bool == 1)
          {
            close(node->chord_fd);
          }
          node->pred_fd = -1;
          node->succ_fd = -1;
          node->chord_fd = -1;
          // atualizar tabelas
          update_neighbour(node, node->node_left, node->succ);
        }
        else if (strcmp(node->self, node->succ_two) != 0 && strcmp(node->pred, node->succ_two) == 0)
        {
          // passar de 3 para 2 nós
          strcpy(node->node_left, node->succ);
          strcpy(node->succ, node->succ_two);
          strcpy(node->succ_IP, node->succ_two_IP);
          strcpy(node->succ_TCP, node->succ_two_TCP);
          // esta ordem é importante (SUCC primeiro e depois PRED) para este caso
          close(node->succ_fd);
          lixo = SUCC("0", "0", node, 1);
          node->succ_fd = PRED(node->succ_IP, node->succ_TCP, node, 0);

          // atualizar tabelas
          update_neighbour(node, node->node_left, node->succ);
          // enviar route
          ROUTE(node, node->pred, node->pred_fd);
          ROUTE(node, node->succ, node->succ_fd);
        }
        else
        {
          strcpy(node->node_left, node->succ);
          strcpy(node->succ, node->succ_two);
          strcpy(node->succ_IP, node->succ_two_IP);
          strcpy(node->succ_TCP, node->succ_two_TCP);
          close(node->succ_fd);
          lixo = SUCC("0", "0", node, 1);
          node->succ_fd = PRED(node->succ_IP, node->succ_TCP, node, 0);

          // atualizar tabelas
          update_neighbour(node, node->node_left, node->succ);
          // enviar route
          ROUTE(node, node->pred, node->pred_fd);
          ROUTE(node, node->succ, node->succ_fd);
          if (node->chord_bool == 1)
          {
            ROUTE(node, node->chord, node->chord_fd);
          }
          for (size_t j = 0; j < MAX_N_NODES; j++)
          {
            if (node->chord_rec[j] != -1)
            {
              search_fd(node, node->chord_rec[j], &fd_index);
              ROUTE(node, node->table_indexes[fd_index], node->chord_rec[j]);
            }
          }
        }
      }
    }

    /**********************************************************************/

    /* fd responsável pelo canal do predecessor ***************************/
    if (FD_ISSET(node->pred_fd, &rfds))
    {
      FD_CLR(node->pred_fd, &rfds);
      memset(buffer, '\0', 500);
      while ((n = read(node->pred_fd, buffer, 500)) != 0)
      {
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        else
        {
          // identificar o protocolo recebido
          choose_received_protocol(node, buffer, node->pred_fd);
          break;
        }
      }
      if (n == 0)
      {
        if (strcmp(node->self, node->succ_two) == 0)
        { // caso em que estão 2 nós e o outro sai
          strcpy(node->node_left, node->pred);
          node->pred_left_bool = 1;
          close(node->pred_fd);
          node->pred_fd = -1;
          update_neighbour(node,node->node_left, node->pred);
        }
        else //possivelmente a pagar
        {
          strcpy(node->node_left, node->pred);
          node->pred_left_bool = 1;
          close(node->pred_fd);
          node->pred_fd = -1;
          update_neighbour(node,node->node_left, node->pred);
        }
      }
    }
    /**********************************************************************/

    /* fd responsável pelas cordas que se ligaram a mim *********************************/
    for (i = 0; i < MAX_N_NODES; i++)
    {
      if (FD_ISSET(node->chord_rec[i], &rfds))
      {
        FD_CLR(node->chord_rec[i], &rfds);
        memset(buffer, '\0', 500);
        while ((n = read(node->chord_rec[i], buffer, 500)) != 0)
        {
          if (n == -1)
          {
            printf("Erro no n");
            exit(1);
          }
          else
          {
            choose_received_protocol(node, buffer, node->chord_fd);
          }
          break;
        }
        if (n == 0)
        {
          node->chord_rec[i] = -1;
          // atualizar tabelas
          search_fd(node, node->chord_rec[i], &fd_index);
          update_neighbour(node, node->table_indexes[fd_index], NULL);
          // enviar route
          ROUTE(node, node->pred, node->pred_fd);
          ROUTE(node, node->succ, node->succ_fd);
          if (node->chord_bool == 1)
          {
            ROUTE(node, node->chord, node->chord_fd);
          }
          for (size_t j = 0; j < MAX_N_NODES; j++)
          {
            if (node->chord_rec[j] != -1)
            {
              search_fd(node, node->chord_rec[j], &fd_index);
              ROUTE(node, node->table_indexes[fd_index], node->chord_rec[j]);
            }
          }
        }
      }
    }
      /**********************************************************************/

      /* fd responsável pelo canal da corda que eu criei *********************************/
      if (FD_ISSET(node->chord_fd, &rfds))
      {
        FD_CLR(node->chord_fd, &rfds);
        memset(buffer, '\0', 500);
        while ((n = read(node->chord_fd, buffer, 500)) != 0)
        {
          if (n == -1)
          {
            printf("Erro no n");
            exit(1);
          }
          // identificar o protocolo recebido
          choose_received_protocol(node, buffer, node->chord_fd);
          break;
        }
        if (n == 0)
        {
          node->chord_bool == 0;
          // atualizar tabelas
          update_neighbour(node, node->chord, NULL);
          // enviar route
          ROUTE(node, node->pred, node->pred_fd);
          ROUTE(node, node->succ, node->succ_fd);
          for (size_t j = 0; j < MAX_N_NODES; j++)
          {
            if (node->chord_rec[j] != -1)
            {
              search_fd(node, node->chord_rec[j], &fd_index);
              ROUTE(node, node->table_indexes[fd_index], node->chord_rec[j]);
            }
          }
        }
        // break;
      }
      /**********************************************************************/

      /* fd responsável pelo canal de escuta ********************************/
      if (FD_ISSET(fd, &rfds))
      {
        memset(buffer, '\0', 500);
        FD_CLR(fd, &rfds);
        addrlen = sizeof(addr);
        if ((newfd = accept(fd, &addr, &addrlen)) == -1)
        {
          printf("Erro no accept");
          exit(1);
        }
        while ((n = read(newfd, buffer, 500)) != 0)
        {
          if (n == -1)
          {
            printf("Erro no n");
            exit(1);
          }
          // identificar o protocolo recebido
          choose_received_protocol(node, buffer, newfd);
          break;
        }
      }
      /**********************************************************************/
    }
  }

  void choose_terminal_cmd(node * node, char readInter[])
  {
    char buffer[500];
    char modo[15], modo2[15];
    char ring[4];
    char id[3], dest[3];
    char succid[3], succIP[30], succTCP[10];
    char mensagem[128];

    sscanf(readInter, "%s", modo);

    //  Comando join
    if (strcmp(modo, "join") == 0 || strcmp(modo, "j") == 0)
    {
      if (node->ring_bool == 1)
      {
        printf("\n=== Nó já pertence a um anel! ===\n");
        return;
      }
      sscanf(readInter, "%s %s %s", modo, ring, id);
      strcpy(node->ring, ring);
      strcpy(node->self, id);
      strcpy(node->self_IP, IP_global);
      strcpy(node->self_TCP, TCP_global);
      join(ring, node);
    }

    // Comando direct join
    else if (strcmp(modo, "direct") == 0 || strcmp(modo, "dj") == 0)
    {
      sscanf(readInter, "%s %s", modo, modo2);
      if ((strcmp(modo2, "join") == 0) || (strcmp(modo, "dj") == 0))
      {
        if (node->ring_bool == 1)
        {
          printf("\n=== Nó já pertence a um anel! ===\n");
          return;
        }
        if (strcmp(modo, "dj") == 0)
        {
          sscanf(readInter, "%s %s %s %s %s", modo, id, succid, succIP, succTCP);
          sprintf(buffer, "%s %s %s %s", id, succid, succIP, succTCP);
        }
        if (strcmp(modo2, "join") == 0)
        {
          sscanf(readInter, "%s %s %s %s %s %s", modo, modo2, id, succid, succIP, succTCP);
          sprintf(buffer, "%s %s %s %s", id, succid, succIP, succTCP);
        }
        strcpy(node->self, id);
        strcpy(node->self_IP, IP_global);
        strcpy(node->self_TCP, TCP_global);
        direct_join(ring, node, buffer);
      }
      else
      {
        printf("\n=== Comando não reconhecido! ===\n");
        return;
      }
    }

    // Comando chord
    else if (strcmp(modo, "chord") == 0 || strcmp(modo, "c") == 0)
    {
      if (node->chord_bool == 1)
      {
        printf("\n=== Nó já tem uma corda! ===\n");
        return;
      }
      else if (strcmp(node->ring, "\0") == 0)
      {
        fprintf(stdout, "O nó não tem conhecimento do anel a qual faz parte. Não poderá criar cordas");
      }
      else
      {
        chord(node);
      }
    }

    // Comando remove chord
    else if ((strcmp(modo, "remove") == 0) || (strcmp(modo, "rc") == 0))
    {
      sscanf(readInter, "%s %s", modo, modo2);
      if ((strcmp(modo2, "chord") == 0) || (strcmp(modo, "rc") == 0))
      {
        if (node->chord_bool == 0)
        {
          printf("\n=== Nó não tem uma corda! ===\n");
          return;
        }
        remove_chord(node);
      }
      else
      {
        printf("\n=== Comando não reconhecido! ===\n");
        return;
      }
    }

    // Comando show topology
    else if ((strcmp(modo, "show") == 0) || (strcmp(modo, "st") == 0))
    {
      sscanf(readInter, "%s %s", modo, modo2);
      if ((strcmp(modo2, "topology") == 0) || (strcmp(modo, "st") == 0))
      {
        show_topology(node);
      }
      else
      {
        printf("\n=== Comando não reconhecido! ===\n");
        return;
      }
    }

    // Comando show routing
    else if ((strcmp(modo, "show") == 0) || (strcmp(modo, "sr") == 0))
    {
      sscanf(readInter, "%s %s %s", modo, modo2, dest);
      if ((strcmp(modo2, "path") == 0))
      {
        sscanf(readInter, "%s %s %s", modo, modo2, dest);
        show_routing(node, dest);
      }
      else if (strcmp(modo, "sr") == 0)
      {
        sscanf(readInter, "%s %s", modo, dest);
        show_routing(node, dest);
      }
      else
      {
        printf("\n=== Comando não reconhecido! ===\n");
        return;
      }
    }

    // Comando show path
    else if ((strcmp(modo, "show") == 0) || (strcmp(modo, "sp") == 0))
    {
      sscanf(readInter, "%s %s %s", modo, modo2, dest);
      if ((strcmp(modo2, "path") == 0))
      {
        sscanf(readInter, "%s %s %s", modo, modo2, dest);
        show_path(node, dest);
      }
      else if (strcmp(modo, "sp") == 0)
      {
        sscanf(readInter, "%s %s", modo, dest);
        show_path(node, dest);
      }
      else
      {
        printf("\n=== Comando não reconhecido! ===\n");
        return;
      }
    }

    // Comando show forwarding
    else if ((strcmp(modo, "show") == 0) || (strcmp(modo, "sf") == 0))
    {
      sscanf(readInter, "%s %s", modo, modo2);
      if ((strcmp(modo2, "forwarding") == 0) || (strcmp(modo, "sf") == 0))
      {
        show_forwarding(node);
      }
      else
      {
        printf("\n=== Comando não reconhecido! ===\n");
        return;
      }
    }

    // Comando message
    else if (strcmp(modo, "message") == 0 || strcmp(modo, "m") == 0)
    {
      if (node->ring_bool == 0)
      {
        printf("\n=== Nó não pertence a um anel! ===\n");
        return;
      }
      sscanf(readInter, "%s %s %s", modo, dest, mensagem);
      message(node, dest, mensagem);
    }
    else
    {
      printf("\n=== Comando não reconhecido! ===\n");
      return;
    }
  }

  void choose_received_protocol(node *node, char buffer[], int fd)
  {
    char protocol[128];
    char aux_id[3];
    char aux_IP[50];
    char aux_TCP[10];
    char lixo[30];
    char origin_id[3], dest_id[3], path[50];
    char i[3], n[3], message[128];
    char *after = buffer;
    char *aux_after = buffer;

    while ((aux_after = find_next_protocol(after)) != NULL) // guardar mensagens a serem recebidas
    {
      sscanf(after, "%s", protocol);
      if (strcmp(protocol, "ENTRY") == 0)
      {
        sscanf(after, "%s %s %s %s", protocol, aux_id, aux_IP, aux_TCP);
        ENTRY_Received(aux_id, aux_IP, aux_TCP, node, fd);
      }
      else if (strcmp(protocol, "SUCC") == 0)
      {
        sscanf(after, "%s %s %s %s", protocol, aux_id, aux_IP, aux_TCP);
        SUCC_Received(aux_id, aux_IP, aux_TCP, node, fd);
      }
      else if (strcmp(protocol, "PRED") == 0)
      {
        sscanf(after, "%s %s", protocol, aux_id);
        PRED_Received(aux_id, node, fd);
      }
      else if (strcmp(protocol, "CHORD") == 0)
      {
        sscanf(after, "%s %s", protocol, aux_id);
        CHORD_Received(aux_id, node, fd);
      }
      else if (strcmp(protocol, "ROUTE") == 0)
      {
        sscanf(after, "%s %s %s %s", protocol, origin_id, dest_id, path);
        ROUTE_Received(node, origin_id, dest_id, path);
      }
      else if (strcmp(protocol, "CHAT") == 0)
      {
        sscanf(buffer, "%s %s %s %s", protocol, i, n, message);
        CHAT_Received(node, i, n, message);
      }
      after = aux_after;
    }
  }

  void PRED_Received(char aux_id[], node *node, int fd)
  {
    int lixo = 0;
    int fd_index = 0;
    if (node->pred_left_bool == 1)
    {
      strcpy(node->pred, aux_id);
      node->pred_fd = fd;
      lixo = SUCC("0", "0", node, 1);
      node->pred_left_bool = 0;
    }
    else
    {
      strcpy(node->pred, aux_id);
      node->pred_fd = fd;
    }

    // atualizar tabelas
    update_neighbour(node, node->node_left, node->succ);
    // enviar route
    ROUTE(node, node->pred, node->pred_fd);
    ROUTE(node, node->succ, node->succ_fd);
    if (node->chord_bool == 1)
    {
      ROUTE(node, node->chord, node->chord_fd);
    }
    for (size_t j = 0; j < MAX_N_NODES; j++)
    {
      if (node->chord_rec[j] != -1)
      {
        search_fd(node, node->chord_rec[j], &fd_index);
        ROUTE(node, node->table_indexes[fd_index], node->chord_rec[j]);
      }
    }
    ROUTE(node, node->pred, node->pred_fd);
    ROUTE(node, node->succ, node->succ_fd);
  }

  void ENTRY_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd)
  {
    int lixo = 0;

    if (fd == node->succ_fd)
    { //recebi um ENTRY NOT ME
      close(node->succ_fd);
      node->succ_fd = PRED(aux_IP, aux_TCP, node, 0);
      strcpy(node->node_left,node->succ);
      strcpy(node->succ_two, node->succ);
      strcpy(node->succ_two_IP, node->succ_IP);
      strcpy(node->succ_two_TCP, node->succ_TCP);
      strcpy(node->succ, aux_id);
      strcpy(node->succ_IP, aux_IP);
      strcpy(node->succ_TCP, aux_TCP);
      show_topology(node);
      lixo = SUCC("0", "0", node, 1);
      update_neighbour(node, node->node_left, node->succ);     // essencialmente não está a estragar nada porque só apaga as entradas nas tabelas
    }
    else
    { // caso de novo nó
      if (strcmp(node->pred, node->self) == 0)
      { // caso em que ainda só são 2 nós na rede
        strcpy(node->pred, aux_id);
        node->pred_fd = fd;
        strcpy(node->succ, aux_id);
        strcpy(node->succ_IP, aux_IP);
        strcpy(node->succ_TCP, aux_TCP);
        lixo = SUCC(aux_IP, aux_TCP, node, 1);
        node->succ_fd = PRED(aux_IP, aux_TCP, node, 0);
      }
      else
      { //+2 nós
        ENTRY_not_me("0", "0", node, aux_id, aux_IP, aux_TCP);
        node->pred_fd = fd;
        strcpy(node->node_left,node->pred);
        strcpy(node->pred, aux_id);
        lixo = SUCC(aux_IP, aux_TCP, node, 1);
        update_neighbour(node, node->node_left, node->pred);
      }
    }
  }

  void SUCC_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd)
  {
    strcpy(node->succ_two, aux_id);
    strcpy(node->succ_two_IP, aux_IP);
    strcpy(node->succ_two_TCP, aux_TCP);
  }

  void CHORD_Received(char aux_id[], node *node, int fd)
  {
    int int_id = 0;
    int i = 0;

    search_index(node, aux_id, &int_id);
    node->chord_rec[int_id] = fd;
    update_neighbour(node, NULL, aux_id);
    ROUTE(node, node->pred, node->pred_fd);
    ROUTE(node, node->succ, node->succ_fd);

    for (i = 0; i < MAX_N_NODES; i++)
    {
      if (node->chord_rec[i] != -1)
      {
        ROUTE(node, aux_id, node->chord_rec[i]);
      }
    }
    return;
  }

  void CHAT_Received(node * node, char origin[], char dest[], char message[])
  {
    if (strcmp(node->self, dest) == 0)
    {
      fprintf(stdout, "\nRecebi uma mensagem:\n %s\n", message);
    }
    else
    {
      CHAT(node, origin, dest, message);
    }
  }
  