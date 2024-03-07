#include "user_cmds.h"

void join(char ring[], node *n)
{
    // 2º nó está a entrar no join
    char mensagemUDP[50];
    char mensagemTCP[50];
    char answer[500];
    char connectid[3], connectIP[50], connectTCP[10];
    int new = 0;
    // int i = 0;

    NODES(ring, answer);
    char *after = DeleteMethod(answer);
    if (strcmp(after, "\0") == 0)
    { // caso em que não há ainda nós no anel
        REG(ring, answer, n);
        if (strcmp(answer, "OKREG") != 0)
        {
            printf("Erro no registo do nó no anel");
            exit(1);
        }
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
        if (strcmp(connectid, n->self) == 0)
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
    }

    /*
    1.CONECÇÃO UDP NODES feito
   2. Receber NODeslist e escolher um (até pode ser o primeiro) Feito
   3.ENTRY TCP
   4. SUCC TCP
   5. SUCC TCP
   6. PRED
   7. reg
   8. OKAYREG
    */
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