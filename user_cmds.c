#include "user_cmds.h"

void join(char ring[], node *n)
{
    char mensagemUDP[50];
    char mensagemTCP[50];
    char answer[500];
    char connectid[3], connectIP[50], connectTCP[10];
    int new = 0;

    NODES(ring, answer);
    char *after = DeleteMethod(answer);
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
    else // VAI SER PRECISO MAIS UM IF, ESTE SÓ FUNCIONA PARA 2 NÓS
    {
        // 2º nó está a entrar aqui
        /*
        1. Guarda lista de nós disponíveis -> a fazer para mais do que 2 nós
        2. Comparar ids (se iguall, dar i+1, voltar a comparar, etc, algo recursivo) ->a fazer para mais do que 2 nós
        3. id escolhido, comparar IPs e porto? -> a fazer para mais do que 2 nós
        4. escolher o primeiro nó na lista, para ser esse a que se liga -> a fazer para mais do que 2 nós
        5. implement select
        */
        sscanf(after, "%s %s %s", connectid, connectIP, connectTCP); // escolher o primeiro nó na lista, para ser esse a que se liga
        if (strcmp(connectid, n->self) == 0) //só está para 2 nós, caso em que já existe esse id
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
        }
        // ENTRY para o sucessor
        ENTRY(connectIP, connectTCP, n);
        // sucessor
        strcpy(n->succ, connectid);
        strcpy(n->succ_IP, connectIP);
        strcpy(n->succ_TCP, connectTCP);
        // registar servidor de nós
        REG(ring, answer, n);
        if (strcmp(answer, "OKREG") != 0)
        {
            printf("Erro no registo do nó no anel");
            exit(1);
        }
        n->ring_bool = 1;
    }
}

void direct_join(char ring[], node *n, char buffer[]){
    
    char modo[15], modo2[15];
    char id[3];
    char succid[3], succIP[30], succTCP[10];

    sscanf(buffer, "%s %s %s %s", id, succid, succIP, succTCP);
    printf("Scanned : %s, TCP: %s/n",succIP, succTCP);
    if(strcmp(id,succid)==0){
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
    } else{
        printf("Enviei IP: %s, TCP: %s/n",succIP, succTCP);
        ENTRY(succIP, succTCP, n);
        // sucessor
        strcpy(n->succ, succid);
        strcpy(n->succ_IP, succIP);
        strcpy(n->succ_TCP, succTCP); 
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

    // AINDA POR FAZER
    if(chord_bool == 1){
        printf("\tSeu segundo-sucessor:\n");
        printf("\t\u2022 id: %s \n", node->succ_two);
        printf("\t\u2022 IP: %s \n", node->succ_two_IP);
        printf("\t\u2022 Porto: %s \n\n", node->succ_two_TCP);
    }else{
        printf("\tEste nó não possui cordas\n");
    }

    printf("\n================================================\n\n");
}