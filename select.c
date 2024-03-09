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
  if ((errcode_listen_tdp = getaddrinfo(NULL, TCP_global, &hints, &res)) != 0) //Duarte tinha IP_global, why
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
  maxfd = fd;
  /********************************************************************/

  while (1)
  { // ACTUALLY select
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(fd, &rfds);
    if(node->pred_fd !=0) FD_SET(node->pred_fd, &rfds); //só se pode dar set se existir
    if(node->succ_fd !=0) FD_SET(node->succ_fd, &rfds);
    if(node->chord_fd !=0) FD_SET(node->chord_fd, &rfds);

    // Show Interface for each command
    Interface(node);

    counter = select(maxfd + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);

    // The select function returns the number of file descriptors (the number of bits from readfds, writefds, and errorfds).
    if (counter == 0)
    { // caso não se leia nada-> leave?
    }

    /* fd responsável pelo terminal****************************************/
    if (FD_ISSET(0, &rfds))
    {
      // coisas no terminal
      FD_CLR(0,&rfds);
      choose_terminal_cmd(node);
    }
    /**********************************************************************/

    /* fd responsável pelo canal de escuta ********************************/
    if (FD_ISSET(fd, &rfds))
    {
      // comunicação de outros nós
      FD_CLR(fd,&rfds);
      addrlen = sizeof(addr);
      if ((newfd = accept(fd, &addr, &addrlen)) == -1)
      {
        printf("Erro no accept");
        exit(1);
      }
      while ((n = read(newfd, buffer, 128)) != 0) //break para parar este while? Sim
      {
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        // identificar o protocolo recebido
        printf("Recebi de novo");
        choose_received_protocol(node, buffer, newfd);
        break; //necessário para parar o while se não eles mesmo depois de tudo continua aqui e não deixa ler mais coisas do terminal
      }
      //close(newfd); //acho que não podemos dar close 
      //close(node->listen_fd);
    }
    /**********************************************************************/

    /* fd responsável pelo canal do predecessor ***************************/
    if (FD_ISSET(node->pred_fd, &rfds))
    {
      // comunicação de outros nós
      FD_CLR(node->pred_fd,&rfds);
      addrlen = sizeof(addr);
      if ((newfd = accept(node->pred_fd, &addr, &addrlen)) == -1)
      {
        printf("Erro no accept");
        exit(1);
      }
      while ((n = read(newfd, buffer, 128)) != 0) //precisamos de parar este while?
      {
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        // identificar o protocolo recebido
        choose_received_protocol(node, buffer, newfd);
      }
      // close(newfd); // não pdoemos dar close, só no leave
    }
    /**********************************************************************/

    /* fd responsável pelo canal do sucessor ******************************/
    if (FD_ISSET(node->succ_fd, &rfds))
    {
      // comunicação de outros nós
      FD_CLR(node->succ_fd,&rfds);
      addrlen = sizeof(addr);
      if ((newfd = accept(node->succ_fd, &addr, &addrlen)) == -1)
      {
        printf("Erro no accept");
        exit(1);
      }
      while ((n = read(newfd, buffer, 128)) != 0)
      {
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        // identificar o protocolo recebido
        choose_received_protocol(node, buffer, newfd);
      }
     // close(newfd); // não pdoemos dar close, só no leave
    }
    /**********************************************************************/

    /* fd responsável pelo canal da corda *********************************/
    if (FD_ISSET(node->chord_fd, &rfds))
    {
      // comunicação de outros nós
      FD_CLR(node->chord_fd,&rfds);
      addrlen = sizeof(addr);
      if ((newfd = accept(node->chord_fd, &addr, &addrlen)) == -1)
      {
        printf("Erro no accept");
        exit(1);
      }
      while ((n = read(newfd, buffer, 128)) != 0)
      {
        if (n == -1)
        {
          printf("Erro no n");
          exit(1);
        }
        // identificar o protocolo recebido
        choose_received_protocol(node, buffer, newfd);
      }
      //close(newfd); // não pdoemos dar close, só no leave
    }
    /**********************************************************************/
  }
}

void choose_terminal_cmd(node *node)
{
  char readInter[128], buffer[128];
  char modo[15], modo2[15];
  char ring[4];
  char id[3], dest[3];
  char succid[3], succIP[30], succTCP[10];
  char message[129];

  printf("Entrei no terminal?");
  if (fgets(readInter, sizeof(readInter), stdin) == NULL)
  {
    exit(0);
  };
  sscanf(readInter, "%s", modo);

  //  join
  if (strcmp(modo, "join") == 0 || strcmp(modo, "j") == 0)
  {
    if (node->ring_bool == 1)
    {
      printf("\n=== Nó já pertence a um anel! ===\n"); //Not sure se é suposto fazer isto
      return;
    }
    sscanf(readInter, "%s %s %s", modo, ring, id);
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
      if(strcmp(modo, "dj") == 0){
        sscanf(readInter, "%s %s %s %s %s", modo, id, succid, succIP, succTCP);
        sprintf(buffer, "%s %s %s %s", id, succid, succIP, succTCP);
        //printf("Buffer: %s", buffer);
      }
      if(strcmp(modo2, "join") == 0){
        sscanf(readInter, "%s %s %s %s %s %s", modo, modo2, id, succid, succIP, succTCP);
        sprintf(buffer, "%s %s %s %s", id, succid, succIP, succTCP);
        //printf("Buffer: %s", buffer);
      }
      strcpy(node->self, id);
      strcpy(node->self_IP, IP_global);
      strcpy(node->self_TCP, TCP_global);
      //printf("minhas informações %s %s %s:\n", node->self,node->self_IP, node->self_TCP);
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

  // Comando leave
  else if (strcmp(modo, "leave") == 0 || strcmp(modo, "l") == 0)
  {
    if (node->ring_bool == 0)
    {
      printf("\n=== Nó não pertence a um anel! ===\n");
      return;
    }
    // leave();
  }

  // Comando exit
  else if (strcmp(modo, "exit") == 0 || strcmp(modo, "x") == 0)
  {
    // exit_COR();
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
  
  sscanf(buffer, "%s", protocol);
  if (strcmp(protocol, "ENTRY") == 0)
  {
    //receive_ENTRY(); //porquê esta função e não fazer tudo aqui? É que se não estou só a enviar mais coisas para lá
    if(fd = node->succ_fd){
      printf("Recebi um entry");
      sscanf(buffer, "%s %s %s %s", protocol, aux_id, aux_IP, aux_TCP);
      PRED(aux_IP, aux_TCP, node); 
      strcpy(node->succ_two, node->succ);
      strcpy(node->succ_two_IP, node->succ_IP);
      strcpy(node->succ_two_TCP, node->succ_TCP);
      strcpy(node->succ, aux_id);
      strcpy(node->succ_IP, aux_IP);
      strcpy(node->succ_TCP, aux_TCP);
      node->succ_fd =fd; //fd é o fd do que recebemos
      SUCC(node->pred_IP, node->pred_TCP, node);
      //Falta fechar adjacencia com o agora node->succ_two (pode ser aqui no final?)
    } else{ //caso de novo nó
      sscanf(buffer, "%s %s %s %s", protocol, aux_id, aux_IP, aux_TCP);
      if(strcmp(node->pred,node->self) == 0){
        strcpy(node->pred,aux_id);
        strcpy(node->pred_IP,aux_IP);
        strcpy(node->pred_TCP,aux_TCP);
        node->pred_fd = fd;
        strcpy(node->succ,aux_id);
        strcpy(node->succ_IP,aux_IP);
        strcpy(node->succ_TCP,aux_TCP);
        SUCC(aux_IP, aux_TCP, node);
        PRED(aux_IP, aux_TCP, node); //aqui faz logo a conexão
      } else{
        ENTRY_not_me(node->pred_IP, node->pred_TCP, node, aux_id, aux_IP, aux_TCP);
        strcpy(node->pred,aux_id);
        strcpy(node->pred_IP,aux_IP);
        strcpy(node->pred_TCP,aux_TCP);
        node->pred_fd = fd;
      }      
    }
    // sscanf(buffer, "%s %s %s %s", protocol, node->pred, node->pred_IP, node->pred_TCP);
  }
  if (strcmp(protocol, "SUCC") == 0){
    printf("Recebi o SUCC");
    sscanf(buffer, "%s %s %s %s", protocol, aux_id, aux_IP, aux_TCP);
    strcpy(node->succ_two, aux_id);
    strcpy(node->succ_two_IP, aux_IP);
    strcpy(node->succ_two_TCP, aux_TCP);
    printf("2º sucessor: %s %s %s", node->succ_two, node->succ_two_IP, node->succ_two_TCP);
  }
  if (strcmp(protocol, "PRED") == 0){
    printf("Recebi o PRED");
    sscanf(buffer, "%s %s", protocol, aux_id);
    strcpy(node->pred, aux_id);
    node->pred_fd = fd;
    //COMO TENHO O IP E PORTO DELE?
  }
}