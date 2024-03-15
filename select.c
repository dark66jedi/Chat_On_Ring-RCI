#include "select.h"
#include "user_cmds.h"

void impl_select(node *node)
{
  fd_set rfds;
  struct addrinfo hints, *res;
  int maxfd = 0, newfd, fd;
  int errcode_listen_tdp, errcode_pred_tcp, errcode_succ_tcp, errcode_chord_tcp;
  struct sockaddr addr;
  socklen_t addrlen;
  // char *ptr;
  int counter, nread, n;
  char buffer[128];
  char readInter[128];
  char modo[20];

  // int *id = 0;

  /* socket de escuta tcp (receber informação de nós fora do ring)*/
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    exit(1);
  if (fd == -1)
  {
    printf("Erro na criação da socket tcp");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP socket
  hints.ai_flags = AI_PASSIVE;
  if ((errcode_listen_tdp = getaddrinfo(NULL, TCP_global, &hints, &res)) != 0) // Duarte tinha IP_global, why
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

  // fprintf(stdout, "fds antes while: %i %i %i %i\n", fd, node->pred_fd, node->succ_fd, node->chord_fd);
  /********************************************************************/
  // while (i < 2){
  // FD_ZERO(&rfds);
  //  maxfd = max(fd, node->pred_fd, node->succ_fd, node->chord_fd);~

  while (1)
  { // ACTUALLY select
    FD_ZERO(&rfds);
    maxfd = max(fd, node->pred_fd, node->succ_fd, node->chord_fd);
    FD_SET(0, &rfds);
    FD_SET(fd, &rfds);
    if (node->pred_fd != 0)
      FD_SET(node->pred_fd, &rfds); // só se pode dar set se existir
    if (node->succ_fd != 0)
      FD_SET(node->succ_fd, &rfds);
    if (node->chord_fd != 0)
      FD_SET(node->chord_fd, &rfds);
    // fprintf(stdout, "fds no while: %i %i %i %i\n", fd, node->pred_fd, node->succ_fd, node->chord_fd);
    // fprintf(stdout, "maxfd: %i \n", maxfd);

    // Show Interface for each command
    // não pode estar aqui, se não de cada vez que há uma comunicação de outro nó, "imprime" a interface de novo

    counter = select(maxfd + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);

    // The select function returns the number of file descriptors (the number of bits from readfds, writefds, and errorfds).
    if (counter == 0)
    {
    }

    /* fd responsável pelo terminal****************************************/
    if (FD_ISSET(0, &rfds))
    {
      // coisas no terminal
      FD_CLR(0, &rfds);
      // Interface(node);
      if (fgets(readInter, sizeof(readInter), stdin) == NULL)
      {
        // fprintf(stdout, "Entrei fgets\n");
        exit(0);
      }
      sscanf(readInter, "%s", modo);
      // fprintf(stdout, "%s\n", modo);
      if (strcmp(modo, "leave") == 0 || strcmp(modo, "l") == 0)
      {
        fprintf(stdout, "Entrei no leave\n");
        if (node->self != node->pred)
        {
          leave(node);
          FD_ZERO(&rfds);
          freeaddrinfo(res);
          close(node->pred_fd);
          close(node->succ_fd);
          close(fd);
          free(node);
          return;
        }
        else if ((node->succ == node->pred) && (node->self == node->pred))
        { /////LEAVE:check if 1º node/////
          FD_ZERO(&rfds);
          freeaddrinfo(res);
          close(fd);
          free(node);
          return;
        }
      }
      else if (strcmp(modo, "exit") == 0 || strcmp(modo, "e") == 0)
      {
        // fprintf(stdout, "Entrei mal2\n");
      }
      else
      {
        // 3fprintf(stdout, "Entrei aqui,pfvr!\n");
        choose_terminal_cmd(node, readInter);
      }
      Interface(node);
    }
    /**********************************************************************/

    /* fd responsável pelo canal de escuta ********************************/
    if (FD_ISSET(fd, &rfds))
    {
      // comunicação de outros nós
      memset(buffer, '\0', 128);
      fprintf(stdout, "Recebi uma mensagem de alguém\n");
      // fprintf(stdout,"foi de: %d\n", fd); //socket de escuta
      // FD_CLR(fd, &rfds);
      addrlen = sizeof(addr);
      if ((newfd = accept(fd, &addr, &addrlen)) == -1)
      {
        printf("Erro no accept");
        exit(1);
      }
      while ((n = read(newfd, buffer, 128)) != 0) // break para parar este while? Sim
      {
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        // identificar o protocolo recebido
        // printf("Recebi de novo");
        // fprintf(stdout,"crei nova socket com numero: %d\n", newfd);
        choose_received_protocol(node, buffer, newfd);
        break; // necessário para parar o while se não eles mesmo depois de tudo continua aqui e não deixa ler mais coisas do terminal
      }
      // printf("n é logo 0? %d\n", n);
      // close(newfd); //acho que não podemos dar close
      // close(node->listen_fd);
      // break;
    }
    /**********************************************************************/

    /* fd responsável pelo canal do predecessor ***************************/
    if (FD_ISSET(node->pred_fd, &rfds))
    {
      memset(buffer, '\0', 128);
      // printf("Recebi uma mensagem do meu pred"); //-> imprime infinitamente, ver pq, TENHO DE FECHAR ALGO? Nao, certo?
      // fprintf(stdout, "A fd é: %d \n", node->pred_fd);
      //   comunicação de outros nós
      //  FD_CLR(node->pred_fd, &rfds);
      //  addrlen = sizeof(addr);
      //  Acho que não tenho de dar accept porque a ligação já foi estabelecida
      /*if ((newfd = accept(node->pred_fd, &addr, &addrlen)) == -1)
      {
        printf("Erro no accept");
        exit(1);
      }*/
      while ((n = read(node->pred_fd, buffer, 128)) != 0) // precisamos de parar este while?
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
          choose_received_protocol(node, buffer, node->pred_fd);
          break; // necessário para parar o while se não eles mesmo depois de tudo continua aqui e não recebe mais coisas
        }
      }
      if (n == 0)
      {
        node->pred_left_bool = 1;
        fprintf(stdout, "O meu pred deu leave");
        // na verdade, acho que só isto, porque ele precisa de receber o PRED para conseguir fazer algo
      }
      // close(newfd); // não pdoemos dar close, só no leave
      // break;
    }
    /**********************************************************************/

    /* fd responsável pelo canal do sucessor ******************************/
    if (FD_ISSET(node->succ_fd, &rfds))
    {
      memset(buffer, '\0', 128);
      // printf("Recebi uma mensagem do meu succ\n");// -> imprime infinitamente, ver pq
      //  fprintf(stdout, "A fd é: %d \n", node->succ_fd);
      //    comunicação de outros nós
      while ((n = read(node->succ_fd, buffer, 128)) != 0)
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
          choose_received_protocol(node, buffer, node->succ_fd);
          fprintf(stdout, "Cheguei aqui");
          break; // necessário para parar o while se não eles mesmo depois de tudo continua aqui e não recebe mais coisas
          // fprintf(stdout,"Oh não, não deu brek");
        }
      }
      if (n == 0)
      {
        fprintf(stdout, "O meu succ deu leave");
        strcpy(node->succ, node->succ_two);
        strcpy(node->succ_IP, node->succ_two_IP);
        strcpy(node->succ_TCP, node->succ_two_TCP);
        node->succ_fd = PRED(node->succ_IP, node->succ_TCP, node, 0);
        SUCC(node->pred_IP, node->pred_TCP, node, 1);
      }
    }
    // close(newfd); // não pdoemos dar close, só no leave
    // break;

    /**********************************************************************/

    /* fd responsável pelo canal da corda *********************************/
    if (FD_ISSET(node->chord_fd, &rfds))
    {
      // comunicação de outros nós
      // FD_CLR(node->chord_fd, &rfds);
      /*addrlen = sizeof(addr);
      if ((newfd = accept(node->chord_fd, &addr, &addrlen)) == -1)
      {
        printf("Erro no accept");
        exit(1);
      }*/
      memset(buffer, '\0', 128);
      while ((n = read(newfd, buffer, 128)) != 0)
      {
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        // identificar o protocolo recebido
        choose_received_protocol(node, buffer, node->chord_fd);
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
  char buffer[128];
  char modo[15], modo2[15];
  char ring[4];
  char id[3], dest[3];
  char succid[3], succIP[30], succTCP[10];
  char message[129];

  // printf("Entrei no terminal?");
  /*if (fgets(readInter, sizeof(readInter), stdin) == NULL)
  {
    exit(0);
  };*/
  // fprintf(stdout, "Resultado fgets: %s\n", readInter);
  sscanf(readInter, "%s", modo);

  //  join
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
        // printf("Buffer: %s", buffer);
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
    }
    // chord();
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
      // remove_chord();
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
      show_topology(node, 0);
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
      show_topology(node, 0);
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
    sscanf(readInter, "%s %s %s", modo, dest, message);
    // message();
  }
  else
  {
    printf("\n=== Comando não reconhecido! ===\n");
    return;
  }
}

// fd -> quem está a falar comigo
void choose_received_protocol(node *node, char buffer[], int fd)
{
  char protocol[128];
  // char buffer2[128];
  char aux_id[3];
  char aux_IP[50];
  char aux_TCP[10];
  char lixo[30];
  char origin_id[3], dest_id[3], path[50];

  fprintf(stdout, "Recebi algo: %s\n", buffer);
  // fprintf(stdout,"tamanho %zu\n", strlen("\n"));
  char *after = buffer;
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
    /*if(strcmp(after, NULL)==0){
      fprintf(stdout,"Está null");
    }*/
    // fprintf(stdout,"After no final de uma ronda: %s\n", after);
    // fprintf(stdout,"Aux no final de uma ronda: %s\n", aux_after);
    after = aux_after;
  }
  // fprintf(stdout,"After assim que saio do while: %s\n", after);
}

void PRED_Received(char aux_id[], node *node, int fd)
{
  if (node->pred_left_bool == 1)
  {
    fprintf(stdout, "Recebi o PRED left pred\n");
    strcpy(node->pred, aux_id);
    node->pred_fd = fd;
    SUCC("0", "0", node, 1);
    // codigo
    node->pred_left_bool = 0;
  }
  else
  {
    fprintf(stdout, "Recebi o PRED\n");
    strcpy(node->pred, aux_id);
    node->pred_fd = fd;
  }
}

void ENTRY_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd)
{
  int lixo = 0;
  int fd_me = 0;
  if (fd == node->succ_fd)
  {
    fprintf(stdout, "Recebi um entry not me\n");
    // sscanf(buffer, "%s %s %s %s", protocol, aux_id, aux_IP, aux_TCP);
    fprintf(stdout, "meu pred fd:%d\n", node->pred_fd);
    node->succ_fd = PRED(aux_IP, aux_TCP, node, 0);
    strcpy(node->succ_two, node->succ);
    strcpy(node->succ_two_IP, node->succ_IP);
    strcpy(node->succ_two_TCP, node->succ_TCP);
    strcpy(node->succ, aux_id);
    strcpy(node->succ_IP, aux_IP);
    strcpy(node->succ_TCP, aux_TCP);
    show_topology(node, 0);
    fprintf(stdout, "meu pred fd:%d\n", node->pred_fd);
    SUCC(node->pred_IP, node->pred_TCP, node, 1);
    // Falta fechar adjacencia com o agora node->succ_two (pode ser aqui no final?)
  }
  else
  { // caso de novo nó
    fprintf(stdout, "Recebi um entry\n");
    // fprintf(stdout, "Veio do fd: %i\n", fd);
    // sscanf(buffer, "%s %s %s %s", protocol, aux_id, aux_IP, aux_TCP);
    if (strcmp(node->pred, node->self) == 0)
    { // caso em que ainda só são 2 nós na rede
      // printf("E aqui, entrei?");
      strcpy(node->pred, aux_id);
      strcpy(node->pred_IP, aux_IP);
      strcpy(node->pred_TCP, aux_TCP);
      node->pred_fd = fd;
      // fprintf(stdout, "pred_fd quando recebo entry1: %d\n", node->pred_fd);
      strcpy(node->succ, aux_id);
      strcpy(node->succ_IP, aux_IP);
      strcpy(node->succ_TCP, aux_TCP);
      node->succ_fd = fd;
      SUCC(aux_IP, aux_TCP, node, 1);
      lixo = PRED(aux_IP, aux_TCP, node, 1);
      // fprintf(stdout, "compraração: %s = %s\n", node->succ_two, node->self);
      // fprintf(stdout, "lixo = %d\n", lixo);
    }
    else
    { //+2 nós
      // fprintf(stdout, "Entrei no +2 nós");

      ENTRY_not_me(node->pred_IP, node->pred_TCP, node, aux_id, aux_IP, aux_TCP);
      node->pred_fd = fd;
      fprintf(stdout, "pred fd:%d\n", node->pred_fd);
      strcpy(node->pred, aux_id);
      strcpy(node->pred_IP, aux_IP);
      strcpy(node->pred_TCP, aux_TCP);
      SUCC(aux_IP, aux_TCP, node, 1);
      // node->pred_fd_send = fd_me;
      // fprintf(stdout, "meu pred:%s", node->pred);
    }
  }
  // sscanf(buffer, "%s %s %s %s", protocol, node->pred, node->pred_IP, node->pred_TCP);
}

void SUCC_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd)
{
  fprintf(stdout, "Recebi o SUCC\n");
  // fprintf(stdout, "Veio do fd: %i\n", fd);
  // sscanf(buffer, "%s %s %s %s", protocol, aux_id, aux_IP, aux_TCP);
  strcpy(node->succ_two, aux_id);
  strcpy(node->succ_two_IP, aux_IP);
  strcpy(node->succ_two_TCP, aux_TCP);
  // fprintf(stdout,"2º sucessor: %s %s %s", node->succ_two, node->succ_two_IP, node->succ_two_TCP);
  // fprintf(stdout, "fd que guardo pos SUCC: %d\n", node->succ_fd);
}

void CHORD_Received(char aux_id[], node *node, int fd)
{
}