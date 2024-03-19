#include "select.h"
#include "user_cmds.h"
#include <errno.h>

void impl_select(node *node)
{
  fd_set rfds;
  struct addrinfo hints, *res;
  int maxfd = 0, newfd = 0, fd = 0;
  int errcode_listen_tdp, errcode_pred_tcp, errcode_succ_tcp, errcode_chord_tcp;
  struct sockaddr addr;
  socklen_t addrlen;
  int counter = 0, nread = 0, n = 0;
  int lixo = 0;
  char buffer[500];
  char readInter[128];
  char modo[20];
  char* token = NULL;

  /* socket de escuta tcp (receber informação de nós fora do ring)*/
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    printf("Erro na criação da socket tcp");
    exit(0);
  }
  perror("fd");
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP socket
  hints.ai_flags = AI_PASSIVE;
  if ((errcode_listen_tdp = getaddrinfo(NULL, TCP_global, &hints, &res)) != 0)
  {
    printf("Erro getaddrinfo tcp");
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
  fprintf(stdout, "socket de escuta fd: %d\n", fd);
  while (1)
  { // ACTUALLY select
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(fd, &rfds);
    maxfd = max(fd, node->pred_fd, node->succ_fd, node->chord_fd);
    fprintf(stdout, "fd: %d, node->pred_fd: %d, node->succ_fd: %d, node->chord_fd: %d\n", fd, node->pred_fd, node->succ_fd, node->chord_fd);
    fprintf(stdout, "Maxfd: %d\n", maxfd);
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
    // fprintf(stdout, "fds no while: %i %i %i %i\n", fd, node->pred_fd, node->succ_fd, node->chord_fd);
    // fprintf(stdout, "maxfd: %i \n", maxfd);

    // printf("File descriptors inside rfds before calling select: \n");
    // int j = 0;
    // for (int j = 0; j < maxfd; j++) {
    //     if (FD_ISSET(j, &rfds)) {
    //         fprintf(stdout, "fd a ser ouvido: %d\n ", j);
    //     }
    // }

    counter = select(maxfd + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);
    perror("Erro ao abrir o arquivo");
    printf("Error opening file: %s\n", strerror(errno));
    fprintf(stdout, "counter select %d\n", counter);

    // The select function returns the number of file descriptors (the number of bits from readfds, writefds, and errorfds).
    if (counter == 0)
    {
      // probably não vamos usar
    }

    /* fd responsável pelo terminal****************************************/
    if (FD_ISSET(0, &rfds))
    {
      FD_CLR(0, &rfds);
      if (fgets(readInter, sizeof(readInter), stdin) != NULL)
      {
        sscanf(readInter, "%s", modo);
        while (node->ring_bool == 0)
        {
          if ((strcmp(modo, "direct") == 0) || (strcmp(modo, "dj") == 0) || (strcmp(modo, "join") == 0) || (strcmp(modo, "j") == 0) || (strcmp(modo, "exit") == 0) || (strcmp(modo, "x") == 0))
          {
            break;
          }
          else
          {
            fprintf(stdout, "Comando inválido. O nó ainda não faz parte de um ring. Por favor, escolha um comando válido. \n");
            Interface(node);
            fgets(readInter, sizeof(readInter), stdin);
            sscanf(readInter, "%s", modo);
          }
        }
      }
      //fprintf(stdout, "%s\n", modo);
      if (strcmp(modo, "leave") == 0 || strcmp(modo, "l") == 0)
      {
        fprintf(stdout, "Entrei no leave\n");
        if (node->self != node->pred) //2+ nós
        {
          //fprintf(stdout,"Ring: %s\n", node->ring);
          if(strcmp(node->ring,"\0") != 0){
            leave(node);
           // fprintf(stdout,"Entrei mal se fui djoin!\n");
          }
          close(node->pred_fd);
          close(node->succ_fd);
          if(node->chord_bool == 1){
            close(node->chord_fd);
          }
          clean_node(node);
          // ADICIONAR ALTERAR ROUTE
        }
        else if ((node->succ == node->pred) && (node->self == node->pred))
        { // leave:check if único node/////
        if(strcmp(node->ring,"\0") != 0){
            leave(node);
            fprintf(stdout,"Entrei mal!\n");
          }
          clean_node(node);
          // ADICIONAR ALTERAR ROUTE
        }
      }
      else if (strcmp(modo, "exit") == 0 || strcmp(modo, "x") == 0)
      {
        fprintf(stdout, "Entrei no exit\n");
        if (node->self != node->pred)
        {
          if(node->ring_bool == 1){
            leave(node);
          }
          clean_node(node);
          FD_ZERO(&rfds);
          freeaddrinfo(res);
          close(node->pred_fd);
          close(node->succ_fd);
          if(node->chord_bool == 1){
            close(node->chord_fd);
          }
          close(fd);
          close(2);
          close(1);
          close(0);
          free(node);
          exit(0);
        }
        else if ((node->succ == node->pred) && (node->self == node->pred))
        { /////EXIT:check if único node/////
          if(node->ring_bool == 1){
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
          exit(0);
        }
      }
      else
      {
        choose_terminal_cmd(node, readInter);
      }
      fprintf(stdout, "Chego aqui");
      Interface(node);
    }
    /**********************************************************************/

    /* fd responsável pelo canal de escuta ********************************/
    if (FD_ISSET(fd, &rfds))
    {
      memset(buffer, '\0', 500);
      fprintf(stdout, "Recebi uma mensagem de alguém\n");
      // fprintf(stdout,"foi de: %d\n", fd); //socket de escuta
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
        fprintf(stdout,"estou a receber no buffer: %s\n", buffer);
        choose_received_protocol(node, buffer, newfd);
        break; // necessário para parar o while se não eles mesmo depois de tudo continua aqui e não deixa ler mais coisas do terminal
      }
    }
    /**********************************************************************/

    /* fd responsável pelo canal do sucessor ******************************/
    if (FD_ISSET(node->succ_fd, &rfds))
    {
      memset(buffer, '\0', 500);
      FD_CLR(node->succ_fd, &rfds);
      while ((n = read(node->succ_fd, buffer, 500)) != 0)
      {
        printf("Recebi uma mensagem do meu succ\n");
        fprintf(stdout, "n pred:%d\n", n);
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        else
        {
          // identificar o protocolo recebido
          token = strtok(buffer,"\n");
          while (token != NULL)
          {
            choose_received_protocol(node, token, node->succ_fd);
            
            strcpy(token, strtok(NULL,"\n"));
          }

          fprintf(stdout, "Cheguei aqui");
          break; // necessário para parar o while se não eles mesmo depois de tudo continua aqui e não recebe mais coisas
        }
      }
      if (n == 0)
      {
        if (strcmp(node->self, node->succ_two) == 0)
        {
          // caso 2 nós, 1 vai sair e vai ficar só 1
          strcpy(node->succ, node->succ_two);
          strcpy(node->succ_IP, node->succ_two_IP);
          strcpy(node->succ_TCP, node->succ_two_TCP);
          strcpy(node->pred, node->succ_two);
          close(node->pred_fd);
          close(node->succ_fd);
          if(node->chord_bool == 1){
            close(node->chord_fd);
          }
          node->pred_fd = -1;
          node->succ_fd = -1;
          node->chord_fd = -1;
        }
        else if (strcmp(node->self, node->succ_two) != 0 && strcmp(node->pred, node->succ_two) == 0)
        {
          // passar de 3 para 2 nós
          strcpy(node->succ, node->succ_two);
          strcpy(node->succ_IP, node->succ_two_IP);
          strcpy(node->succ_TCP, node->succ_two_TCP);
          // esta ordem é importante (SUCC primeiro e depois PRED) para este caso
          close(node->succ_fd);
          lixo = SUCC("0", "0", node, 1);
          node->succ_fd = PRED(node->succ_IP, node->succ_TCP, node, 0);
        }
        else
        {
          fprintf(stdout, "O meu succ deu leave");
          strcpy(node->succ, node->succ_two);
          strcpy(node->succ_IP, node->succ_two_IP);
          strcpy(node->succ_TCP, node->succ_two_TCP);
          close(node->succ_fd);
          lixo = SUCC("0", "0", node, 1);
          node->succ_fd = PRED(node->succ_IP, node->succ_TCP, node, 0);
        }
      }
    }

    /**********************************************************************/


    /* fd responsável pelo canal do predecessor ***************************/
    if (FD_ISSET(node->pred_fd, &rfds))
    {
      FD_CLR(node->pred_fd, &rfds);
      memset(buffer, '\0', 500);
      // printf("Recebi uma mensagem do meu pred");
      // fprintf(stdout, "A fd é: %d \n", node->pred_fd);
      while ((n = read(node->pred_fd, buffer, 500)) != 0)
      {
        printf("Recebi uma mensagem do meu pred");
        fprintf(stdout, "n pred:%d\n", n);
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        else
        {
          // identificar o protocolo recebido
          token = strtok(buffer,"\n");
          while (token != NULL)
          {
            choose_received_protocol(node, token, node->pred_fd);
            
            strcpy(token, strtok(NULL,"\n"));
          }
          break; // necessário para parar o while se não eles mesmo depois de tudo continua aqui e não recebe mais coisas
        }
      }
      if (n == 0)
      {
        if(strcmp(node->self,node->succ_two) == 0){ //caso em que estão 2 nós e o outro sai
          node->pred_left_bool = 1;
          close(node->pred_fd);
          node->pred_fd = -1;
          fprintf(stdout, "O meu pred deu leave");
        } else
        {
          node->pred_left_bool = 1;
          close(node->pred_fd);
          node->pred_fd = -1;
          fprintf(stdout, "O meu pred deu leave");
        }
      }
    }
    /**********************************************************************/

    /* fd responsável pelo canal da corda que eu criei *********************************/
    if (FD_ISSET(node->chord_fd, &rfds))
    {
      FD_CLR(node->chord_fd, &rfds);
      memset(buffer, '\0', 500);
      while ((n = read(newfd, buffer, 500)) != 0)
      {
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        // identificar o protocolo recebido
        token = strtok(buffer,"\n");
          while (token != NULL)
          {
            choose_received_protocol(node, token, node->chord_fd);
            
            strcpy(token, strtok(NULL,"\n"));
          }
        break; // necessário para parar o while se não eles mesmo depois de tudo continua aqui e não recebe mais coisas
      }
      // close(newfd); // não pdoemos dar close, só no leave
      // break;
    }
    /**********************************************************************/
  }
  //}
}

void choose_terminal_cmd(node *node, char readInter[])
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
      printf("\n=== Nó já pertence a um anel! ===\n"); // Not sure se é suposto fazer isto
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
        fprintf(stdout, "Buffer: %s\n", buffer);
      }
      if (strcmp(modo2, "join") == 0)
      {
        sscanf(readInter, "%s %s %s %s %s %s", modo, modo2, id, succid, succIP, succTCP);
        sprintf(buffer, "%s %s %s %s", id, succid, succIP, succTCP);
        // printf("Buffer: %s", buffer);
      }
      strcpy(node->self, id);
      strcpy(node->self_IP, IP_global);
      strcpy(node->self_TCP, TCP_global);
      // printf("minhas informações %s %s %s:\n", node->self,node->self_IP, node->self_TCP);
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
    } else{
      chord(node, readInter);
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
    if ((strcmp(modo2, "routing") == 0) || (strcmp(modo, "sr") == 0))
    {
      // show_routing();
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
    if ((strcmp(modo2, "path") == 0) || (strcmp(modo, "sp") == 0))
    {
      // show_path();
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
    message(node,dest,mensagem);
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

  fprintf(stdout, "Recebi algo: %s\n", buffer);
  // fprintf(stdout,"tamanho %zu\n", strlen("\n"));
  sscanf(buffer, "%s", protocol);
  printf("\n\n»»»»»»»»»\nprotocol: %s\n««««««««««««\n\n", protocol);
  if (strcmp(protocol, "ENTRY") == 0)
  {
    sscanf(buffer, "%s %s %s %s", protocol, aux_id, aux_IP, aux_TCP);
    ENTRY_Received(aux_id, aux_IP, aux_TCP, node, fd);
  }
  else if (strcmp(protocol, "SUCC") == 0)
  {
    sscanf(buffer, "%s %s %s %s", protocol, aux_id, aux_IP, aux_TCP);
    SUCC_Received(aux_id, aux_IP, aux_TCP, node, fd);
  }
  else if (strcmp(protocol, "PRED") == 0)
  {
    sscanf(buffer, "%s %s", protocol, aux_id);
    PRED_Received(aux_id, node, fd);
  }
  else if (strcmp(protocol, "CHORD") == 0)
  {
    sscanf(buffer, "%s %s", protocol, aux_id);
    CHORD_Received(aux_id, node, fd);
  }
  else if (strcmp(protocol, "ROUTE") == 0)
  {
    sscanf(buffer, "%s %s %s %s", protocol, origin_id, dest_id, path);
    ROUTE_Received(node, origin_id, dest_id, path);
  }
  else if (strcmp(protocol, "CHAT") == 0)
  {
    sscanf(buffer, "%s %s %s %s", protocol, i, n, message);
    CHAT_Received(node, i, n, message);
  }

  /*char *after = buffer;
  // fprintf(stdout, "After: %s", after);
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
      sscanf(after, "%s %s", protocol, origin_id, dest_id, path);
      ROUTE_Received(node, origin_id, dest_id, path);
    }
    // after = find_next_protocol(after);
    // fprintf(stdout,"After no final de uma ronda: %s\n", after);
    // fprintf(stdout,"Aux no final de uma ronda: %s\n", aux_after);
    after = aux_after;
  }*/
  // fprintf(stdout,"After assim que saio do while: %s\n", after);
}

void PRED_Received(char aux_id[], node *node, int fd)
{
  int lixo = 0;
  if (node->pred_left_bool == 1)
  {
    fprintf(stdout, "Recebi o PRED left pred\n");
    strcpy(node->pred, aux_id);
    node->pred_fd = fd;
    lixo = SUCC("0", "0", node, 1);
    // codigo
    node->pred_left_bool = 0;
  }
  else
  {
    fprintf(stdout, "Recebi o PRED\n");
    strcpy(node->pred, aux_id);
    node->pred_fd = fd;
  }
  ROUTE(node, node->pred, node->pred_fd);
  ROUTE(node, node->succ, node->succ_fd);
}

void ENTRY_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd)
{
  int lixo = 0;

  if (fd == node->succ_fd)
  {
    fprintf(stdout, "Recebi um entry not me\n");
    // sscanf(buffer, "%s %s %s %s", protocol, aux_id, aux_IP, aux_TCP);
    fprintf(stdout, "meu pred fd:%d\n", node->pred_fd);
    close(node->succ_fd); 
    node->succ_fd = PRED(aux_IP, aux_TCP, node, 0);
    strcpy(node->succ_two, node->succ);
    strcpy(node->succ_two_IP, node->succ_IP);
    strcpy(node->succ_two_TCP, node->succ_TCP);
    strcpy(node->succ, aux_id);
    strcpy(node->succ_IP, aux_IP);
    strcpy(node->succ_TCP, aux_TCP);
    show_topology(node);
    lixo = SUCC("0", "0", node, 1);
  }
  else
  { // caso de novo nó
    fprintf(stdout, "Recebi um entry\n");
    // fprintf(stdout, "Veio do fd: %i\n", fd);
    if (strcmp(node->pred, node->self) == 0)
    { // caso em que ainda só são 2 nós na rede
      strcpy(node->pred, aux_id);
      node->pred_fd = fd;
      // fprintf(stdout, "pred_fd quando recebo entry1: %d\n", node->pred_fd);
      strcpy(node->succ, aux_id);
      strcpy(node->succ_IP, aux_IP);
      strcpy(node->succ_TCP, aux_TCP);
      node->succ_fd = PRED(aux_IP, aux_TCP, node, 0);
      lixo = SUCC(aux_IP, aux_TCP, node, 1);
    }
    else
    { //+2 nós
      // fprintf(stdout, "Entrei no +2 nós");
      ENTRY_not_me("0", "0", node, aux_id, aux_IP, aux_TCP);
      node->pred_fd = fd;
      fprintf(stdout, "pred fd:%d\n", node->pred_fd);
      strcpy(node->pred, aux_id);
      lixo = SUCC(aux_IP, aux_TCP, node, 1);
      // fprintf(stdout, "meu pred:%s", node->pred);
    }
  }
}

void SUCC_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd)
{
  fprintf(stdout, "Recebi o SUCC\n");
  // fprintf(stdout, "Veio do fd: %i\n", fd);
  strcpy(node->succ_two, aux_id);
  strcpy(node->succ_two_IP, aux_IP);
  strcpy(node->succ_two_TCP, aux_TCP);
  // fprintf(stdout,"2º sucessor: %s %s %s", node->succ_two, node->succ_two_IP, node->succ_two_TCP);
  // fprintf(stdout, "fd que guardo pos SUCC: %d\n", node->succ_fd);
}

void CHORD_Received(char aux_id[], node *node, int fd)
{
  // int int_id = 0;
  // node->n_chords++;
  // int_id = atoi(aux_id);
  // node->chord_rec[i][0] = int_id;
  // node->chord_rec[i][1] = fd;
  // return;
}

void CHAT_Received(node *node, char i[], char n[], char message[]){
    if(strcmp(node->self,n) == 0){
      //Mensagem chegou ao destino
    }
    else{
      CHAT(node,i,n,message);
    }
}