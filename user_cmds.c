#include "user_cmds.h"

typedef struct nodeslist
{
    char id[3];
    char IP[50];
    char TCP[30];
} nodeslist;

void leave(node *n)
{
    char answer[500];

    UNREG(n->ring, answer, n);
    if (strcmp(answer, "OKUNREG") != 0)
    {
        printf("Erro na saída do nó do servidor de nós");
        exit(1);
    }
    return;
}

void join(char ring[], node *n)
{
    char answer[500];
    char connectid[3], connectIP[50], connectTCP[10];
    nodeslist list[16];
    char aux_id[3], aux_IP[50], aux_TCP[30];
    int new = 0;
    int i = 0, j = 0;
    int fd = 0;

    NODES(ring, answer);
    char *after = DeleteMethod(answer);
    if (strcmp(after, "\0") == 0) // caso em que não há ainda nós no anel
    {
        strcpy(n->succ, n->self);
        strcpy(n->succ_IP, n->self_IP);
        strcpy(n->succ_TCP, n->self_TCP);
        strcpy(n->pred, n->self);
        strcpy(n->succ_two, n->self);
        strcpy(n->succ_two_IP, n->self_IP);
        strcpy(n->succ_two_TCP, n->self_TCP);
        REG(n->ring, answer, n);
        if (strcmp(answer, "OKREG") != 0)
        {
            printf("Erro no registo do nó no anel");
            exit(1);
        }
        n->ring_bool = 1;
    }
    else
    {
        // Falta comparar IPs e porto?
        sscanf(after, "%s %s %s", connectid, connectIP, connectTCP); // escolher o primeiro nó na lista, para ser esse a que se liga
        while (*after != '\0') // guardar nodes list para comparar
        {
            sscanf(after, "%s %s %s", aux_id, aux_IP, aux_TCP);
            strcpy(list[i].id, aux_id);
            strcpy(list[i].IP, aux_IP);
            strcpy(list[i].TCP, aux_TCP);
            after = DeleteMethod(after);
            i++;
            if (*after == '\0') 
            {
                break;
            }
        }

        for (j = i - 1; j >= 0; j--) // caso em que já existe esse id
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
        i = 1; // para o caso de o 1º nó ter saído abruptamente;
        fd = ENTRY(connectIP, connectTCP, n);
        while (fd == -1)
        {
            strcpy(connectid, list[i].id);
            strcpy(connectIP, list[i].IP);
            strcpy(connectTCP, list[i].TCP);
            i++;
            fd = ENTRY(connectIP, connectTCP, n);
        }
        // sucessor
        strcpy(n->succ, connectid);
        strcpy(n->succ_IP, connectIP);
        strcpy(n->succ_TCP, connectTCP);
        n->succ_fd = fd;
        n->ring_bool = 1;
    }
    initialize_tables(n);
}

void direct_join(char ring[], node *n, char buffer[])
{
    char id[3];
    char succid[3], succIP[30], succTCP[10];
    int fd = 0;

    sscanf(buffer, "%s %s %s %s", id, succid, succIP, succTCP);
    if (strcmp(id, succid) == 0)
    {
        strcpy(n->succ, n->self);
        strcpy(n->succ_IP, n->self_IP);
        strcpy(n->succ_TCP, n->self_TCP);
        strcpy(n->pred, n->self);
        strcpy(n->succ_two, n->self);
        strcpy(n->succ_two_IP, n->self_IP);
        strcpy(n->succ_two_TCP, n->self_TCP);
        n->ring_bool = 1;
        fprintf(stdout, "\nO nó %s criou um novo anel.\n", n->self);
    }
    else
    {
        fd = ENTRY(succIP, succTCP, n);
        // sucessor
        strcpy(n->succ, succid);
        strcpy(n->succ_IP, succIP);
        strcpy(n->succ_TCP, succTCP);
        n->succ_fd = fd;
        n->ring_bool = 1;
        fprintf(stdout, "\nO nó %s conectou-se ao nó %s\n", n->self, n->succ);
    }

    initialize_tables(n);
}

void chord(node *n)
{
    nodeslist list[16];
    char answer[500];
    char aux_id[3], aux_IP[50], aux_TCP[30];
    int i = 0;
    int fd_index = 0;

    if (strcmp(n->succ_two, n->self) == 0 || strcmp(n->succ_two, n->pred) == 0)
    { // caso de só existir 1, 2 ou 3 aneis no anel
        fprintf(stdout, "O anel tem 3 ou menos nós. Não existem nós com os quais é possível criar uma corda!");
    }
    else
    {
        NODES(n->ring, answer);
        char *after = DeleteMethod(answer);
        while (*after != '\0') // guardar nodes list para comparar
        {
            sscanf(after, "%s %s %s", aux_id, aux_IP, aux_TCP);
            strcpy(list[i].id, aux_id);
            strcpy(list[i].IP, aux_IP);
            strcpy(list[i].TCP, aux_TCP);
            after = DeleteMethod(after);
            i++;
            if (*after == '\0') // acho que não está aqui a fazer nada
            {
                break;
            }
        }
        i = 0;

        //Duarte, not sure
        while (strcmp(list[i].id, n->self) == 0 || strcmp(list[i].id, n->pred) == 0 || strcmp(list[i].id, n->succ) == 0)
        {
            i++;
        }
        strcpy(n->chord, list[i].id);
        strcpy(n->chord_IP, list[i].IP);
        strcpy(n->chord_TCP, list[i].TCP);
        n->chord_fd = CHORD(n->chord_IP, n->chord_TCP, n);
        n->chord_bool = 1;

        // atualizar tabelas
        update_neighbour(n, NULL, n->chord);
        // enviar route
        ROUTE(n, n->pred, n->pred_fd);
        ROUTE(n, n->succ, n->succ_fd);
        if (n->chord_bool == 1)
        {
            ROUTE(n, n->chord, n->chord_fd);
        }
        for (size_t j = 0; j < MAX_N_NODES; j++)
        {
            if (n->chord_rec[j] != -1)
            {
                search_fd(n, n->chord_rec[j], &fd_index);
                ROUTE(n, n->table_indexes[fd_index], n->chord_rec[j]);
            }
        }
    }
}

void remove_chord(node *n)
{

    int fd_index = 0;
    n->chord_bool = 0;
    close(n->chord_fd);
    n->chord[0] = '\0';
    n->chord_IP[0] = '\0';
    n->chord_TCP[0] = '\0';
    // Atualizar caminhos
    update_neighbour(n, n->chord, NULL);
    ROUTE(n, n->pred, n->pred_fd);
    ROUTE(n, n->succ, n->succ_fd);
    if (n->chord_bool == 1)
    {
        ROUTE(n, n->chord, n->chord_fd);
    }
    for (size_t j = 0; j < MAX_N_NODES; j++)
    {
        if (n->chord_rec[j] != -1)
        {
            search_fd(n, n->chord_rec[j], &fd_index);
            ROUTE(n, n->table_indexes[fd_index], n->chord_rec[j]);
        }
    }
    return;
}

void message(node *n, char dest[], char message[])
{
    CHAT(n, n->self, dest, message);
}

void show_topology(node *node)
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

    // Não esquecer de tirar
    printf("\tSeu predecessor:\n");
    printf("\t\u2022 id: %s \n\n", node->pred);

    if (node->chord_bool == 1)
    {
        printf("\tO vizinho na corda que criou:\n");
        printf("\t\u2022 id: %s \n", node->chord);
        printf("\t\u2022 IP: %s \n", node->chord_IP);
        printf("\t\u2022 Porto: %s \n\n", node->chord_TCP);
    }
    else
    {
        printf("\tEste nó não possui cordas\n");
    }

    printf("\n================================================\n\n");
}

void show_routing(node *node, char dest[])
{
    if (strcmp(dest, node->self) != 0)
    {
        int dest_index = 0, pred_index = 0, succ_index = 0, chord_index = 0;

        search_index(node, dest, &dest_index);
        search_index(node, node->pred, &pred_index);
        search_index(node, node->succ, &succ_index);

        printf("\n==========| Tabela de encaminhamento |==========\n\n");

        printf("Destino: %s\n\n", dest);
        printf("\u2022Pelo predecessor %s: %s\n", node->pred, node->routing_table[dest_index][pred_index]);
        printf("\u2022Pelo sucessor %s: %s\n", node->succ, node->routing_table[dest_index][succ_index]);

        if (node->chord_bool == 1)
        {
            search_index(node, node->chord, &chord_index);
            printf("\u2022Pelo corda que eu criei com %s: %s\n", node->chord, node->routing_table[dest_index][chord_index]);
        }        

        for (size_t i = 0; i < MAX_N_NODES; i++)
        {
            if (node->chord_rec[i] != -1)
            {
                printf("\u2022Pelo corda com %s: %s\n", node->table_indexes[i], node->routing_table[dest_index][i]);
            } 
        }
        
        printf("\n================================================\n\n");
    }
    else
    {
        printf("\n==========| Tabela de encaminhamento |==========\n\n");

        printf("Destino: %s\n", dest);
        printf("\u2022Pelo vizinho %s: %s\n", node->pred, "-");
        printf("\u2022Pelo vizinho %s: %s\n", node->succ, "-");

        printf("\n================================================\n\n");
    }
}

void show_path(node *node, char dest[])
{
    if (strcmp(dest, node->self) != 0)
    {
        int dest_index;

        search_index(node, dest, &dest_index);

        printf("\n=============| Caminho mais curto |=============\n\n");

        printf("Destino: %s\n", dest);
        printf("\u2022Caminho mais curto: %s\n", node->short_paths_table[dest_index]);

        printf("\n================================================\n\n");
    }
    else
    {
        printf("\n=============| Caminho mais curto |=============\n\n");

        printf("Destino: %s\n", dest);
        printf("\u2022Caminho mais curto: %s\n", node->self);

        printf("\n================================================\n\n");
    }
}

void show_forwarding(node *node)
{
    int dest_index = 0;

    printf("\n============| Tabela de expedição |============\n\n");

    for (size_t i = 0; i < MAX_N_NODES; i++)
    {
        search_index(node, node->table_indexes[i], &dest_index);
        if (dest_index != -1)
        {
            printf("Destino: %s\n", node->table_indexes[i]);
            printf("\u2022 Nó a seguir: %s\n", node->expedition_table[dest_index]);
        }
    }

    printf("\n===============================================\n\n");
}