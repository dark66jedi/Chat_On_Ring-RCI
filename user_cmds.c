#include "user_cmds.h"

typedef struct nodeslist
{
    char id[3];
    char IP[50];
    char TCP[30];
} nodeslist;

void leave(node *n){
    char answer[500];

    UNREG(n->ring, answer, n);
    if (strcmp(answer, "OKUNREG") != 0)
    {
        printf("Erro na saída do nó do servidor de nós");
        exit(1);
    }
    return;
    //close(node->pred_fd_send);
    //close(node->succ_fd_send);
}

void join(char ring[], node *n)
{
    char mensagemUDP[50];
    char mensagemTCP[50];
    char answer[500];
    char connectid[3], connectIP[50], connectTCP[10];
    nodeslist list[16];
    char aux_id[3], aux_IP[50], aux_TCP[30];
    int new = 0;
    int i = 0, j =0;
    int fd = 0;

    NODES(ring, answer);
    char *after = DeleteMethod(answer);
   // printf("after: %s\n", after);
    if (strcmp(after, "\0") == 0) // caso em que não há ainda nós no anel
    {
        REG(ring, answer, n);
        if (strcmp(answer, "OKREG") != 0)
        {
            printf("Erro no registo do nó no anel");
            exit(1);
        }
        strcpy(n->succ, n->self);
        strcpy(n->succ_IP, n->self_IP);
        strcpy(n->succ_TCP, n->self_TCP);
        strcpy(n->pred, n->self);
        strcpy(n->pred_IP, n->self_IP);
        strcpy(n->pred_TCP, n->self_TCP);
        strcpy(n->succ_two, n->self);
        strcpy(n->succ_two_IP, n->self_IP);
        strcpy(n->succ_two_TCP, n->self_TCP);
        n->ring_bool = 1;
    }
    else 
    {
        // Falta comparar IPs e porto? 
        sscanf(after, "%s %s %s", connectid, connectIP, connectTCP); // escolher o primeiro nó na lista, para ser esse a que se liga
        fprintf(stdout,"a quem em estou a conectar? %s %s %s \n", connectid, connectIP, connectTCP);
        while (*after != '\0') // guardar nodes list para comparar
        {
            sscanf(after, "%s %s %s", aux_id, aux_IP, aux_TCP);
            strcpy(list[i].id, aux_id);
            strcpy(list[i].IP, aux_IP);
            strcpy(list[i].TCP, aux_TCP);
            after = DeleteMethod(after);
            i++;
            if (*after == '\0') //acho que não está aqui a fazer nada
            {
                break;
            }
        }
        //j= i-1;
        //printf("j:%d", j);
        /*for(i=i-1;i>=0;--i){
            printf("\n");
            printf("i=%i\n",i);
            printf("id: %s\n",list[i].id);
            printf("IP: %s\n",list[i].IP);
            printf("TCP: %s\n",list[i].TCP);
            printf("\n");
        } */

        for (j = i-1; j >= 0; j--) //caso em que já existe esse id
        {
            if (strcmp(list[j].id, n->self) == 0)
            {
                new = atoi(n->self);
                new = new + 1;
                if (new < 10)
                {
                    sprintf(n->self, "0%d", new);
                }
                else
                {
                    sprintf(n->self, "%d", new);
                }
                j = i;
            }
        }

        // ENTRY para o sucessor
        fd = ENTRY(connectIP, connectTCP, n);
        fprintf(stdout,"A fd que criei é: %d\n", fd);
        // sucessor
        strcpy(n->succ, connectid);
        strcpy(n->succ_IP, connectIP);
        strcpy(n->succ_TCP, connectTCP);
        n->succ_fd = fd; 
        // registar servidor de nós
        REG(ring, answer, n); //só devia fazer isto se o resto der certo?
        if (strcmp(answer, "OKREG") != 0)
        {
            printf("Erro no registo do nó no anel");
            exit(1);
        }
        n->ring_bool = 1;
    }
}

void direct_join(char ring[], node *n, char buffer[])
{

    char modo[15], modo2[15];
    char id[3];
    char succid[3], succIP[30], succTCP[10];
    int fd = 0;

    sscanf(buffer, "%s %s %s %s", id, succid, succIP, succTCP);
    // printf("Scanned : %s, TCP: %s/n",succIP, succTCP);
    if (strcmp(id, succid) == 0)
    {
        strcpy(n->succ, n->self);
        strcpy(n->succ_IP, n->self_IP);
        strcpy(n->succ_TCP, n->self_TCP);
        strcpy(n->pred, n->self);
        strcpy(n->pred_IP, n->self_IP);
        strcpy(n->pred_TCP, n->self_TCP);
        strcpy(n->succ_two, n->self);
        strcpy(n->succ_two_IP, n->self_IP);
        strcpy(n->succ_two_TCP, n->self_TCP);
        n->ring_bool = 1;
    }
    else
    {
        // printf("Enviei IP: %s, TCP: %s/n",succIP, succTCP);
        fd = ENTRY(succIP, succTCP, n);
        // sucessor
        strcpy(n->succ, succid);
        strcpy(n->succ_IP, succIP);
        strcpy(n->succ_TCP, succTCP);
        n->succ_fd = fd;
        n->ring_bool = 1;
    }
}

void show_topology(node *node, int chord_bool)
{
    printf("\n================================================\n\n");

    printf("Estado do protocolo topológico:\n\n");
    printf("\tPróprio nó:\n");
    printf("\t\u2022 id: %s \n", node->self);
    printf("\t\u2022 IP: %s \n", node->self_IP);
    printf("\t\u2022 Porto: %s \n\n", node->self_TCP);

    printf("\tSeu sucessor:\n");
    printf("\t\u2022 id: %s \n", node->succ);
    printf("\t\u2022 IP: %s \n", node->succ_IP);
    printf("\t\u2022 Porto: %s \n\n", node->succ_TCP);

    printf("\tSeu segundo-sucessor:\n");
    printf("\t\u2022 id: %s \n", node->succ_two);
    printf("\t\u2022 IP: %s \n", node->succ_two_IP);
    printf("\t\u2022 Porto: %s \n\n", node->succ_two_TCP);

    printf("\tSeu predecessor:\n");
    printf("\t\u2022 id: %s \n", node->pred);
    printf("\t\u2022 IP: %s \n", node->pred_IP);
    printf("\t\u2022 Porto: %s \n\n", node->pred_TCP);

    // AINDA POR FAZER
    if (chord_bool == 1)
    {
        printf("\tSeu segundo-sucessor:\n");
        printf("\t\u2022 id: %s \n", node->succ_two);
        printf("\t\u2022 IP: %s \n", node->succ_two_IP);
        printf("\t\u2022 Porto: %s \n\n", node->succ_two_TCP);
    }
    else
    {
        printf("\tEste nó não possui cordas\n");
    }

    printf("\n================================================\n\n");
}